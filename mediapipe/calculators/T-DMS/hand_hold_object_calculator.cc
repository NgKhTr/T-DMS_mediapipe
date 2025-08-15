#ifndef MEDIAPIPE_CALCULATORS_T_DMS_HAND_HOLD_OBJECT_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_HAND_HOLD_OBJECT_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/calculators/T-DMS/hand_hold_object_calculator_options.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/util/T-DMS/duration_process.h"
#include "mediapipe/framework/timestamp.h"

namespace mediapipe {

class HandHoldObjectCalculator : public CalculatorBase, public DurationProcess {
private:
    bool verbose_;
    std::string label_;
    bool IsHoldObject(std::vector<NormalizedLandmarkList> hand_landmarks_vec, std::vector<Detection> detection_vec) {        
        for (const auto& det : detection_vec) {
            bool is_hold = false;
            for (const auto& name : det.label()) {
                if (name == label_) {
                    is_hold = true;
                    break;
                }
            }
            if (!is_hold) continue;

            // Get detection bounding box (normalized)
            if (!det.has_location_data() || det.location_data().format() != LocationData::RELATIVE_BOUNDING_BOX) {
                continue;
            }

            const auto& bbox = det.location_data().relative_bounding_box();
            float det_xmin = bbox.xmin();
            float det_ymin = bbox.ymin();
            float det_xmax = bbox.xmin() + bbox.width();
            float det_ymax = bbox.ymin() + bbox.height();

            // Check all hand landmarks for inclusion in object bbox
            for (const auto& hand_landmarks : hand_landmarks_vec) {
                for (const auto& lm : hand_landmarks.landmark()) {
                    if (lm.x() >= det_xmin && lm.x() <= det_xmax &&
                        lm.y() >= det_ymin && lm.y() <= det_ymax) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
        cc->Inputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        cc->Outputs().Tag("HOLD").Set<bool>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& opts = cc->Options<mediapipe::HandHoldObjectCalculatorOptions>();
        duration_s_threshold_ = opts.duration_s_threshold();
        verbose_ = opts.verbose();
        label_ = opts.label();
        timer_started_ = false;
        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        Timestamp current_timestamp = cc->InputTimestamp();
        bool is_hold = false;
        if (!cc->Inputs().Tag("LANDMARKS").IsEmpty() && !cc->Inputs().Tag("DETECTIONS").IsEmpty()) {

            const auto& hand_landmarks_vec = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
            const auto& detections = cc->Inputs().Tag("DETECTIONS").Get<std::vector<Detection>>();
            is_hold = IsHoldObject(hand_landmarks_vec, detections);
        }

        // bool result = processWithDuration(is_hold);
        bool result = CheckWithDuration(is_hold, current_timestamp, (verbose_ ? ("Hold " + label_ ): ""));
        if (verbose_) {
            LOG(INFO) << "Hold " << label_ << ": " << is_hold << ", Result: " << result;
        }
        cc->Outputs().Tag("HOLD").Add(new bool(result), current_timestamp);
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(HandHoldObjectCalculator);

}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_HAND_HOLD_OBJECT_CALCULATOR_H_