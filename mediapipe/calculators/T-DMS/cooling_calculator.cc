 #ifndef MEDIAPIPE_CALCULATORS_T_DMS_COOLING_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_COOLING_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/T-DMS/cooling_calculator_options.pb.h"
#include <chrono>

namespace mediapipe {
class CoolingCalculator : public CalculatorBase {
private:
    bool verbose_;
    int cool_duration_s_; // seconds
    std::chrono::steady_clock::time_point end_cooling_time_;
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("TRIGGER").Set<bool>();
        cc->Outputs().Tag("TRIGGER").Set<bool>();
        for (const auto& tag : cc->Inputs().GetTags()) {
            if (tag != "TRIGGER") {
                cc->Inputs().Tag(tag).SetAny();
                cc->Outputs().Tag(tag).SetAny();
            }
        }
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& opts = cc->Options<CoolingCalculatorOptions>();
        verbose_ = opts.verbose();
        cool_duration_s_ = opts.cool_duration_s();
        end_cooling_time_ = std::chrono::steady_clock::now();
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        if (std::chrono::steady_clock::now() < end_cooling_time_ 
            || cc->Inputs().Tag("TRIGGER").IsEmpty() 
            || !cc->Inputs().Tag("TRIGGER").Get<bool>()
        ) {
            if (verbose_) {
                LOG(INFO) << "Skipping output.";
            }
            return absl::OkStatus();
        } else {            
            for (const auto& tag: cc->Inputs().GetTags()) { // Including "TRIGGER"
                const auto& packet = cc->Inputs().Tag(tag).Value();
                cc->Outputs().Tag(tag).AddPacket(packet);
            }
            if (verbose_) {
                LOG(INFO) << "Cooling period ended, emitting output.";
            }
            end_cooling_time_ = std::chrono::steady_clock::now() + std::chrono::seconds(cool_duration_s_);
        }
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(CoolingCalculator);
}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_COOLING_CALCULATOR_H_