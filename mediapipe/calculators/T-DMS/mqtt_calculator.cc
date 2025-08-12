#ifndef MEDIAPIPE_CALCULATORS_T_DMS_MQTT_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_MQTT_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/T-DMS/mqtt_calculator_options.pb.h"
#include <mqtt/async_client.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <nlohmann/json.hpp> // For JSON serialization (header-only, add to your project)

namespace mediapipe {
class MqttCallback : public virtual mqtt::callback {
public:
    explicit MqttCallback(bool verbose) : verbose_(verbose) {}
    void message_arrived(mqtt::const_message_ptr msg) override {
        if (verbose_) {
            LOG(INFO) << "Received message on topic: " << msg->get_topic()
                      << " payload: " << msg->to_string();
        }
        // Add custom logic here
    }
private:
    bool verbose_;
};

class MqttCalculator : public CalculatorBase {
private:
    std::unique_ptr<MqttCallback> callback_;
    std::unique_ptr<mqtt::async_client> client_;
    bool verbose_;
    std::string broker_address_;
    uint16_t port_;
    std::string client_id_;
    std::string username_;
    std::string password_;
    std::string telemetry_topic_;
    std::string attribute_topic_;

    std::vector<std::string> payload_keys_;

    // void GetMessageCallback(mqtt::const_message_ptr msg) {
    //     if (verbose_) {
    //         LOG(INFO) << "Received message on topic: " << msg->get_topic()
    //                   << " payload: " << msg->to_string();
    //     }

    // }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        for (const auto& tag : cc->Inputs().GetTags()) {
            if (tag.rfind("PAYLOAD_", 0) == 0) { // starts with "PAYLOAD_"
                cc->Inputs().Tag(tag).Set<std::string>();
            }
        }
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        payload_keys_.clear();
        for (const auto& tag : cc->Inputs().GetTags()) {
            if (tag.rfind("PAYLOAD_", 0) == 0) {
                payload_keys_.push_back(tag.substr(8));
            }
        }

        const auto &opts = cc->Options<MqttCalculatorOptions>();
        verbose_ = opts.verbose();
        broker_address_ = opts.broker_address();
        port_ = opts.port();
        client_id_ = opts.client_id();
        username_ = opts.username();
        password_ = opts.password();
        telemetry_topic_ = opts.telemetry_topic();
        attribute_topic_ = opts.attribute_topic();

        std::string broker_uri = "tcp://" + broker_address_ + ":" + std::to_string(port_);
        client_ = std::make_unique<mqtt::async_client>(broker_uri, client_id_);
        callback_ = std::make_unique<MqttCallback>(verbose_);
        client_->set_callback(*callback_);

        mqtt::connect_options connOpts;
        connOpts.set_user_name(username_);
        connOpts.set_password(password_);


        try {
            client_->connect(connOpts)->wait();
        } catch (const mqtt::exception& exc) {
            LOG(ERROR) << "MQTT connect failed: " << exc.what();
            return absl::InternalError("MQTT connect failed");
        }
        const uint8_t qos = 2; // Quality of Service level
        client_->subscribe(attribute_topic_, qos)->wait();
        if (verbose_) {
            LOG(INFO) << "Connected to MQTT broker at " << broker_address_ << ":" << port_;
        }
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        LOG(INFO) << "VERBOSE_ " << verbose_;
        for (const auto& key : payload_keys_) {
            if (cc->Inputs().Tag("PAYLOAD_" + key).IsEmpty()) {
                if (verbose_) {
                    LOG(INFO) << "No input for PAYLOAD_" << key;
                }
                return absl::OkStatus();
            }
        }

        nlohmann::json data;
        for (std::string key : payload_keys_) {
            const std::string value = cc->Inputs().Tag("PAYLOAD_" + key).Get<std::string>();
            std::transform(key.begin(), key.end(), key.begin(),
               [](unsigned char c){ return std::tolower(c); });
            data[key] = value;
        }
        std::string payload = data.dump();

        try {
            client_->publish(telemetry_topic_, payload.c_str(), payload.size(), 1, false)->wait();
        } catch (const mqtt::exception& exc) {
            LOG(ERROR) << "MQTT publish failed: " << exc.what();
        }
        return absl::OkStatus();
    }

    absl::Status Close(CalculatorContext* cc) override {
        if (client_) {
            try {
                client_->disconnect()->wait();
            } catch (const mqtt::exception& exc) {
                LOG(ERROR) << "MQTT disconnect failed: " << exc.what();
            } catch (const std::exception& exc) {
                LOG(ERROR) << "Exception during MQTT disconnect: " << typeid(exc).name() << ": " << exc.what();
            } catch (...) {
                LOG(ERROR) << "Unknown exception during MQTT disconnect.";
            }
        }
        return absl::OkStatus();
    }
};

REGISTER_CALCULATOR(MqttCalculator);

}  // namespace mediapipe
#endif