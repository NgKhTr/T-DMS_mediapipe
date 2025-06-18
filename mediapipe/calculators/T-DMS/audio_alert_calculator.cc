#ifndef MEDIAPIPE_CALCULATORS_T_DMS_AUDIO_ALERT_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_AUDIO_ALERT_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/T-DMS/audio_alert_calculator_options.pb.h"
#include <SFML/Audio.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

namespace mediapipe {

class AudioAlertCalculator : public CalculatorBase {
// When set duration: audio play with loop and extend duration when having trigger during the audio playing process
// When duration = -1: just play audio, not loop or extend duration if there trigger during the audio playing process
private:
    enum class PlayStatus {
        READY,
        PLAYING,
        COOLING
    };
    std::string audio_path_;
    int play_duration_s_; // seconds
    int cool_duration_s_; // seconds

    sf::SoundBuffer buffer_;
    sf::Sound sound_;
    std::atomic<bool> need_exit_{false};
    std::chrono::steady_clock::time_point stop_time_;
    std::thread audio_thread_;
    std::mutex mutex_;
    PlayStatus play_status_;

    std::atomic<bool> trigger_;

    void AudioLoop() {
        /*
        if (trigger) {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(play_duration_s_);
        }
        */
        while (!need_exit_) {
            if (trigger_) {
                if (play_status_ == PlayStatus::READY) {
                    if (play_duration_s_ > 0) {
                        stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(play_duration_s_);
                    }
                    sound_.play();
                    play_status_ = PlayStatus::PLAYING;
                } else if (play_status_ == PlayStatus::PLAYING) {
                    if (play_duration_s_ > 0) {
                        // extend
                        stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(play_duration_s_);
                        if (sound_.getStatus() != sf::Sound::Playing) {
                            sound_.play();
                        }
                    } else if (sound_.getStatus() != sf::Sound::Playing) {
                        play_status_ = PlayStatus::COOLING;
                        stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(cool_duration_s_);
                    }
                } else if (stop_time_ < std::chrono::steady_clock::now()) {// Cooling
                    play_status_ = PlayStatus::READY;
                }
            } else {
                if (sound_.getStatus() != sf::Sound::Playing) {
                    if (play_status_ == PlayStatus::PLAYING) {
                        if (play_duration_s_ < 0 || stop_time_ < std::chrono::steady_clock::now()) {
                            play_status_ = PlayStatus::COOLING;
                            stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(cool_duration_s_);
                        } else {
                            sound_.play();
                        }
                    } else if (play_status_ == PlayStatus::COOLING && stop_time_ < std::chrono::steady_clock::now()) {
                        play_status_ = PlayStatus::READY;
                    }
                } else if (play_duration_s_ > 0 && stop_time_ < std::chrono::steady_clock::now()) {
                    sound_.stop();
                    play_status_ = PlayStatus::COOLING;
                    stop_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(cool_duration_s_);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("TRIGGER").Set<bool>();
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        const auto& opts = cc->Options<mediapipe::AudioAlertCalculatorOptions>();
        audio_path_ = opts.audio_path();
        play_duration_s_ = opts.play_duration_s();
        cool_duration_s_ = opts.cool_duration_s();
        stop_time_ = std::chrono::steady_clock::now();
        need_exit_ = false;

        play_status_ = PlayStatus::READY;
        trigger_ = false;
        // Load sound
        if (!buffer_.loadFromFile(audio_path_)) {
            return absl::InternalError("Failed to load audio file.");
        }
        sound_.setBuffer(buffer_);
        // Start background thread
        audio_thread_ = std::thread([this]() { this->AudioLoop(); });
        return absl::OkStatus();
    }

    absl::Status Close(CalculatorContext* cc) override {
        need_exit_ = true;
        if (audio_thread_.joinable()) {
            audio_thread_.join();
        }
        sound_.stop();
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        trigger_ = cc->Inputs().Tag("TRIGGER").Get<bool>();
        return absl::OkStatus();
    }


};

REGISTER_CALCULATOR(AudioAlertCalculator);

}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_AUDIO_ALERT_CALCULATOR_H_