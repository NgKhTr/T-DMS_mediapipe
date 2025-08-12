#ifndef MEDIAPIPE_CALCULATORS_T_DMS_START_LATENCY_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_START_LATENCY_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include <chrono>

namespace mediapipe {

class StartLatencyCalculator : public CalculatorBase {
public:
	static absl::Status GetContract(CalculatorContract* cc) {
		for (const auto& tag : cc->Inputs().GetTags()) {
			cc->Inputs().Tag(tag).SetAny();
			cc->Outputs().Tag(tag).SetAny();
		}
		cc->Outputs().Tag("START_TIME").Set<double>();
		return OkStatus();
	}
	absl::Status Open(CalculatorContext* cc) override {
		cc->SetOffset(TimestampDiff(0));
		return absl::OkStatus();
	}
	absl::Status Process(CalculatorContext* cc) override {
		auto start_time = std::chrono::steady_clock::now();
		double start_time_us = std::chrono::duration_cast<std::chrono::microseconds>(
			start_time.time_since_epoch()).count();

		// Forward all input streams to corresponding output streams
		for (const auto& tag : cc->Inputs().GetTags()) {
			if (!cc->Inputs().Tag(tag).IsEmpty()) {
			cc->Outputs().Tag(tag).AddPacket(cc->Inputs().Tag(tag).Value());
			}
		}

		// Send the start time
		cc->Outputs().Tag("START_TIME").Add(new double(start_time_us), cc->InputTimestamp());
		return absl::OkStatus();
	}
};
REGISTER_CALCULATOR(StartLatencyCalculator);
}  // namespace mediapipe
#endif