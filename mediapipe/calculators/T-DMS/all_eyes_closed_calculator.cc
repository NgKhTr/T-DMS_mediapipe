#ifndef MEDIAPIPE_CALCULATORS_T_DMS_ALL_EYES_CLOSED_CALCULATOR_H
#define MEDIAPIPE_CALCULATORS_T_DMS_ALL_EYES_CLOSED_CALCULATOR_H

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/calculators/T-DMS/all_eyes_closed_calculator_options.pb.h"
#include "mediapipe/util/T-DMS/duration_process.h"

namespace mediapipe {

class AllEyesClosedCalculator : public CalculatorBase, public DurationProcess {
private:
	bool verbose_; // Verbose logging flag
	float ear_threshold_; // Eye Aspect Ratio threshold for determining if the eye is closed

	float ComputeEAR(const NormalizedLandmarkList& landmarks) {
		if (landmarks.landmark_size() < 8) {
			return -1; // Not enough points
		}

		const auto& left = landmarks.landmark(16);
		const auto& right = landmarks.landmark(8);

		const auto& bottom3 = landmarks.landmark(3);
		const auto& bottom4 = landmarks.landmark(4);
		const auto& bottom5 = landmarks.landmark(5);

		const auto& top11 = landmarks.landmark(11);
		const auto& top12 = landmarks.landmark(12);
		const auto& top13 = landmarks.landmark(13);

		float horizontal = std::hypot(left.x() - right.x(), left.y() - right.y());

		float vertical_3_11 = std::hypot(bottom3.x() - top11.x(), bottom3.y() - top11.y());
		float vertical_4_12 = std::hypot(bottom4.x() - top12.x(), bottom4.y() - top12.y());
		float vertical_5_13 = std::hypot(bottom5.x() - top13.x(), bottom5.y() - top13.y());

		return (vertical_3_11 + vertical_4_12 + vertical_5_13) / (3 * horizontal + 1e-6f); // Avoid division by zero
	}
public:
	static absl::Status GetContract(CalculatorContract* cc) {
		cc->Inputs().Tag("LEFT_EYE_CONTOUR_LANDMARKS").Set<NormalizedLandmarkList>();
		cc->Inputs().Tag("RIGHT_EYE_CONTOUR_LANDMARKS").Set<NormalizedLandmarkList>();
		cc->Outputs().Tag("ALL_EYES_CLOSED").Set<bool>();
		return absl::OkStatus();
	}
	absl::Status Open(CalculatorContext* cc) override {
		cc->SetOffset(TimestampDiff(0));
		const auto& opts = cc->Options<AllEyesClosedCalculatorOptions>();
		ear_threshold_ = opts.ear_threshold();
		verbose_ = opts.verbose();
		duration_s_threshold_ = opts.duration_s_threshold();
        timer_started_ = false;
		return absl::OkStatus();
	}
	absl::Status Process(CalculatorContext* cc) override {
        bool left_eye_closed = false, right_eye_closed = false, any_eye_detected = false;

        if (!cc->Inputs().Tag("LEFT_EYE_CONTOUR_LANDMARKS").IsEmpty()) {
            const auto& left_list = cc->Inputs().Tag("LEFT_EYE_CONTOUR_LANDMARKS").Get<NormalizedLandmarkList>();
			const float ear = ComputeEAR(left_list);
			if (verbose_) {
				LOG(INFO) << "Left EAR: " << ear;
			}

			if (ear < 0) {
				LOG(WARNING) << "Not enough landmarks to compute EAR for left eye.";
			} else if (ear < ear_threshold_) {
				left_eye_closed = true;
				any_eye_detected = true;
			} else {
				any_eye_detected = true;
			}
        }
        if (!cc->Inputs().Tag("RIGHT_EYE_CONTOUR_LANDMARKS").IsEmpty()) {
            const auto& right_list = cc->Inputs().Tag("RIGHT_EYE_CONTOUR_LANDMARKS").Get<NormalizedLandmarkList>();
            const float ear = ComputeEAR(right_list);
			if (verbose_) {
				LOG(INFO) << "Right EAR: " << ear;
			}

			if (ear < 0) {
				LOG(WARNING) << "Not enough landmarks to compute EAR for right eye.";
			} else if (ear < ear_threshold_) {
				right_eye_closed = true;
				any_eye_detected = true;
			} else {
				any_eye_detected = true;
			}
        }
		bool all_closed = any_eye_detected && (left_eye_closed && right_eye_closed);
        bool result = CheckWithDuration(all_closed, (verbose_ ? "All eyes closed": ""));
        if (verbose_) {
			LOG(INFO) << "All eyes closed: " << all_closed << ", Result: " << result;
		}
        cc->Outputs().Tag("ALL_EYES_CLOSED").Add(new bool(result), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(AllEyesClosedCalculator);
} // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_ALL_EYES_CLOSED_CALCULATOR_H