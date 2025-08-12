#ifndef MEDIAPIPE_CALCULATORS_T_DMS_YAWN_CALCULATOR_H
#define MEDIAPIPE_CALCULATORS_T_DMS_YAWN_CALCULATOR_H

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/calculators/T-DMS/yawn_calculator_options.pb.h"
#include "mediapipe/util/T-DMS/duration_process.h"

namespace mediapipe {

class YawnCalculator : public CalculatorBase, public DurationProcess {
private:
	bool verbose_; // Verbose logging flag
	float mar_threshold_; // Mouth Aspect Ratio threshold for determining if the mouth is opening

	float ComputeMAR(const NormalizedLandmarkList& landmarks) {
		if (landmarks.landmark_size() < 468) {
			return -1; // Not enough points
		}

		const auto& left = landmarks.landmark(62);
		const auto& right = landmarks.landmark(292);

		const auto& bottom86 = landmarks.landmark(86);
		const auto& bottom15 = landmarks.landmark(15);
		const auto& bottom316 = landmarks.landmark(316);

		const auto& top38 = landmarks.landmark(38);
		const auto& top12 = landmarks.landmark(12);
		const auto& top268 = landmarks.landmark(268);

		float horizontal = std::hypot(left.x() - right.x(), left.y() - right.y());

		float vertical_38_86 = std::hypot(bottom86.x() - top38.x(), bottom86.y() - top38.y());
		float vertical_12_15 = std::hypot(bottom15.x() - top12.x(), bottom15.y() - top12.y());
		float vertical_268_316 = std::hypot(bottom316.x() - top268.x(), bottom316.y() - top268.y());
		return (vertical_38_86 + vertical_12_15 + vertical_268_316) / (3 * horizontal + 1e-6f); // Avoid division by zero
	}
public:
	static absl::Status GetContract(CalculatorContract* cc) {
		cc->Inputs().Tag("MULTI_LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
		cc->Outputs().Tag("YAWN").Set<bool>();
		return absl::OkStatus();
	}
	absl::Status Open(CalculatorContext* cc) override {
		cc->SetOffset(TimestampDiff(0));
		const auto& opts = cc->Options<mediapipe::YawnCalculatorOptions>();
		mar_threshold_ = opts.mar_threshold();
		verbose_ = opts.verbose();
		duration_s_threshold_ = opts.duration_s_threshold();
        timer_started_ = false;
		return absl::OkStatus();
	}
	absl::Status Process(CalculatorContext* cc) override {
		bool mouth_open = false;
        if (!cc->Inputs().Tag("MULTI_LANDMARKS").IsEmpty()) {
            const auto& multi_landmarks = cc->Inputs().Tag("MULTI_LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
			if (!multi_landmarks.empty()) {
				for (const auto& landmarks: multi_landmarks) {
					const float mar = ComputeMAR(landmarks);
					if (verbose_) {
						LOG(INFO) << "MAR: " << mar;
					}
					if (mar < 0) {
						LOG(WARNING) << "Not enough landmarks to compute MAR";
					} else if (mar > mar_threshold_) {
						mouth_open = true;
						break;
					}
				}
			}
		}

        bool result = CheckWithDuration(mouth_open, (verbose_ ? "Yawn": ""));
        if (verbose_) {
            LOG(INFO) << "Yawn: " << mouth_open << ", Result: " << result;
        }
        cc->Outputs().Tag("YAWN").Add(new bool(result), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(YawnCalculator);
} // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_YAWN_CALCULATOR_H