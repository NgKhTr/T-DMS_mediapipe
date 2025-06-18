#ifndef MEDIAPIPE_CALCULATORS_T_DMS_SOS_HAND_SIGN_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_SOS_HAND_SIGN_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/calculators/T-DMS/sos_hand_sign_calculator_options.pb.h"
#include "mediapipe/framework/port/logging.h"

namespace mediapipe {

// Just check if any hand satisfy next hand sign;
class SOSHandSignCalculator : public CalculatorBase {
private:
    bool verbose_;
    float max_transform_duration_s_threshold_; // Duration in seconds
    float min_keep_duration_s_threshold_; // Duration in seconds
    std::chrono::steady_clock::time_point start_time_;

    enum class SignStatus {
        INITIAL,
        SIGN1,
        SIGN2,
        SIGN3
    };
	SignStatus recent_sign_;

    enum class State {
        INITIAL,
        TRANSFORMING,
        KEEPING
    };
    State recent_state_;
    
    bool IsPointOnTheSameSide(const NormalizedLandmark& target_point,
                            const NormalizedLandmark& border_point,
                            const NormalizedLandmark& side_point) {
        // Vector: border_point -> target_point
        float bp_tp_x = target_point.x() - border_point.x(),
            bp_tp_y = target_point.y() - border_point.y();
        // Vector: border_point -> side_point
        float bp_sp_x = side_point.x() - border_point.x(),
            bp_sp_y = side_point.y() - border_point.y();

        float dot_product = bp_tp_x * bp_sp_x + bp_tp_y * bp_sp_y;
        return (dot_product > 0);
    }

    bool IsPointOnTheSameSide(const NormalizedLandmark& target_point,
                            const NormalizedLandmark& border_point_0,
                            const NormalizedLandmark& border_point_1,
                            const NormalizedLandmark& side_point) {
        // Vector: border_point_0 -> target_point
        float bp0_tp_x = target_point.x() - border_point_0.x(),
            bp0_tp_y = target_point.y() - border_point_0.y();
        // Vector: border_point_0 -> border_point_1
        float bp0_bp1_x = border_point_1.x() - border_point_0.x(),
            bp0_bp1_y = border_point_1.y() - border_point_0.y();
        // Vector: border_point_0 -> side_point
        float bp0_sp_x = side_point.x() - border_point_0.x(),
            bp0_sp_y = side_point.y() - border_point_0.y();

        // (border_point_0 -> border_point_1) x (border_point_0 -> target_point)
        float cross_product_target = bp0_bp1_x * bp0_tp_y - bp0_bp1_y * bp0_tp_x;
        // (border_point_0 -> border_point_1) x (border_point_0 -> side_point)
        float cross_product_side = bp0_bp1_x * bp0_sp_y - bp0_bp1_y * bp0_sp_x;

        return cross_product_target * cross_product_side > 0;
    }
    bool IsIndexFingerStraight(const NormalizedLandmarkList& hand_landmarks) {
        return ! IsPointOnTheSameSide(hand_landmarks.landmark(8), hand_landmarks.landmark(6), hand_landmarks.landmark(0));
    }
    bool IsMiddleFingerStraight(const NormalizedLandmarkList& hand_landmarks) {
        return ! IsPointOnTheSameSide(hand_landmarks.landmark(12), hand_landmarks.landmark(10), hand_landmarks.landmark(0));
    }
    bool IsRingFingerStraight(const NormalizedLandmarkList& hand_landmarks) {
        return ! IsPointOnTheSameSide(hand_landmarks.landmark(15), hand_landmarks.landmark(14), hand_landmarks.landmark(0));
    }
    bool IsPinkyFingerStraight(const NormalizedLandmarkList& hand_landmarks) {
        return ! IsPointOnTheSameSide(hand_landmarks.landmark(20), hand_landmarks.landmark(18), hand_landmarks.landmark(0));
    }
    bool IsThumbStraight(const NormalizedLandmarkList& hand_landmarks) {
        return !IsPointOnTheSameSide(hand_landmarks.landmark(4), hand_landmarks.landmark(6), hand_landmarks.landmark(2), hand_landmarks.landmark(0));
    }
    bool IsSign1(std::vector<NormalizedLandmarkList> hand_landmarks_vec) {
        for (const auto& hand_landmarks : hand_landmarks_vec) {
            if (hand_landmarks.landmark_size() < 21) {
                LOG(INFO) << "Not enough landmarks: " << hand_landmarks.landmark_size();
                continue;
            } else if (IsIndexFingerStraight(hand_landmarks) && IsMiddleFingerStraight(hand_landmarks)
                    && IsRingFingerStraight(hand_landmarks) && IsPinkyFingerStraight(hand_landmarks)
                    && IsThumbStraight(hand_landmarks)) {
                return true;
            }
        }
        return false;
    }
    bool IsSign2(std::vector<NormalizedLandmarkList> hand_landmarks_vec) {
        for (const auto& hand_landmarks : hand_landmarks_vec) {
            if (hand_landmarks.landmark_size() < 21) {
                LOG(INFO) << "Not enough landmarks: " << hand_landmarks.landmark_size();
                continue;
            } else if (IsIndexFingerStraight(hand_landmarks) && IsMiddleFingerStraight(hand_landmarks)
                    && IsRingFingerStraight(hand_landmarks) && IsPinkyFingerStraight(hand_landmarks)
                    && ! IsThumbStraight(hand_landmarks)) {
                return true;
            }
        }
        return false;
    }
    bool IsSign3(std::vector<NormalizedLandmarkList> hand_landmarks_vec) {
        for (const auto& hand_landmarks : hand_landmarks_vec) {
            if (hand_landmarks.landmark_size() < 21) {
                LOG(INFO) << "Not enough landmarks: " << hand_landmarks.landmark_size();
                continue;
            }
            else if (!IsIndexFingerStraight(hand_landmarks) && !IsMiddleFingerStraight(hand_landmarks)
                    && !IsRingFingerStraight(hand_landmarks) && !IsPinkyFingerStraight(hand_landmarks)
                    && !IsThumbStraight(hand_landmarks)) {
                return true;
            }
        }
        return false;
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
        cc->Outputs().Tag("SOS").Set<bool>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        const auto& opts = cc->Options<mediapipe::SOSHandSignCalculatorOptions>();
        max_transform_duration_s_threshold_ = opts.max_transform_duration_s_threshold();
        min_keep_duration_s_threshold_ = opts.min_keep_duration_s_threshold();
        verbose_ = opts.verbose();
        recent_sign_ = SignStatus::INITIAL;
        recent_state_ = State::INITIAL;
        cc->SetOffset(TimestampDiff(0));
        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        bool isSOS = false;

        if (recent_sign_ == SignStatus::INITIAL) {
            if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
                const auto& hand_landmarks_vec = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
                if (IsSign1(hand_landmarks_vec)) {
                    recent_sign_ = SignStatus::SIGN1;
                    start_time_ = std::chrono::steady_clock::now();
                    recent_state_ = State::KEEPING;
                }
            }
        } else if (recent_sign_ == SignStatus::SIGN1) {
            auto now = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(now - start_time_).count();
            if (recent_state_ == State::KEEPING) {
                if (duration < min_keep_duration_s_threshold_) {
                    if (!cc->Inputs().Tag("LANDMARKS").IsEmpty() 
                        && ! IsSign1(cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>())
                    ) {
                        recent_sign_ = SignStatus::INITIAL;
                        recent_state_ = State::INITIAL;
                    }
                } else {
                    start_time_ = std::chrono::steady_clock::now();
                    recent_state_ = State::TRANSFORMING;
                }
            } else if (recent_state_ == State::TRANSFORMING) {
                if (duration < max_transform_duration_s_threshold_) {
                    if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
                        const auto& hand_landmarks_vec = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
                        if (IsSign1(hand_landmarks_vec)) {
                            start_time_ = std::chrono::steady_clock::now();
                        } else if (IsSign2(hand_landmarks_vec)) {
                            recent_sign_ = SignStatus::SIGN2;
                            start_time_ = std::chrono::steady_clock::now();
                            recent_state_ = State::KEEPING;
                        }
                    }
                } else {
                    recent_sign_ = SignStatus::INITIAL;
                    recent_state_ = State::INITIAL;
                }
            } else {
                recent_sign_ = SignStatus::INITIAL;
                recent_state_ = State::INITIAL;
            }
        } else if (recent_sign_ == SignStatus::SIGN2) {
            auto now = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(now - start_time_).count();
            if (recent_state_ == State::KEEPING) {
                if (duration < min_keep_duration_s_threshold_) {
                    if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()
                        && ! IsSign2(cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>())
                    ) {
                        recent_sign_ = SignStatus::INITIAL;
                        recent_state_ = State::INITIAL;
                    }
                } else {
                    start_time_ = std::chrono::steady_clock::now();
                    recent_state_ = State::TRANSFORMING;
                }
            } else if (recent_state_ == State::TRANSFORMING) {
                if (duration < max_transform_duration_s_threshold_) {
                    if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
                        const auto& hand_landmarks_vec = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
                        if (IsSign2(hand_landmarks_vec)) {
                            start_time_ = std::chrono::steady_clock::now();
                        } else if (IsSign3(hand_landmarks_vec)) {
                            recent_sign_ = SignStatus::SIGN3;
                            start_time_ = std::chrono::steady_clock::now();
                            recent_state_ = State::KEEPING;
                        }
                    }
                } else {
                    recent_sign_ = SignStatus::INITIAL;
                    recent_state_ = State::INITIAL;
                }
            } else {
                recent_sign_ = SignStatus::INITIAL;
                recent_state_ = State::INITIAL;
            }
        } else if (recent_sign_ == SignStatus::SIGN3) {
            auto now = std::chrono::steady_clock::now();
            float duration = std::chrono::duration<float>(now - start_time_).count();
            if (recent_state_ == State::KEEPING) {
                if (duration < min_keep_duration_s_threshold_) {
                    if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()
                        && ! IsSign3(cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>())
                    ) {
                        recent_sign_ = SignStatus::INITIAL;
                        recent_state_ = State::INITIAL;
                    }
                } else if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()
                        && IsSign3(cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>())
                ) {
                    isSOS = true;
                } else {
                    recent_sign_ = SignStatus::INITIAL;
                    recent_state_ = State::INITIAL;
                }
            } else {
                recent_sign_ = SignStatus::INITIAL;
                recent_state_ = State::INITIAL;
            }
        }
        
        cc->Outputs().Tag("SOS").Add(new bool(isSOS), cc->InputTimestamp());
        if (verbose_) {
            if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
                auto& landmarks = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
                for (int8_t i = 0; i < landmarks.size(); ++i) {
                    const auto& hand_landmarks = landmarks[i];
                    LOG(INFO) << "Hand " << i << " is straight: " <<
                        IsThumbStraight(hand_landmarks) << " + " <<
                        IsIndexFingerStraight(hand_landmarks) << " | " << 
                        IsMiddleFingerStraight(hand_landmarks) << " | " <<
                        IsRingFingerStraight(hand_landmarks) << " | " <<
                        IsPinkyFingerStraight(hand_landmarks);
                }

            }
            LOG(INFO) << "recent_sign_ " << static_cast<int>(recent_sign_) << ", isSOS: " << isSOS;
        }
        return absl::OkStatus();
    }
};

REGISTER_CALCULATOR(SOSHandSignCalculator);
}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_SOS_HAND_SIGN_CALCULATOR_H_