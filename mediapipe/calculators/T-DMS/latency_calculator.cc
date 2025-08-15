#ifndef MEDIAPIPE_CALCULATORS_T_DMS_LATENCY_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_LATENCY_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/timestamp.h"
#include "mediapipe/calculators/T-DMS/latency_calculator_options.pb.h"
#include <chrono>
#include <cmath>

namespace mediapipe {

class LatencyCalculator : public CalculatorBase {
private:
	std::deque<double> latencies_;
	static constexpr size_t window_size_ = 100;  // Number of frames for variance
	std::string label_;
	bool verbose_;
public:
	static absl::Status GetContract(CalculatorContract* cc) {
		for (const auto& tag : cc->Inputs().GetTags()) {
			if (tag != "START_TIME") {
				cc->Inputs().Tag(tag).SetAny();
				cc->Outputs().Tag(tag).SetAny();
			}
		}
		cc->Inputs().Tag("START_TIME").Set<double>();
		cc->Outputs().Tag("FPS").Set<double>().Optional();
		cc->Outputs().Tag("LATENCY").Set<double>().Optional();
		return OkStatus();
	}
	absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
  		const auto& opts = cc->Options<mediapipe::LatencyCalculatorOptions>();
		label_ = opts.label();
		verbose_ = opts.verbose();
		latencies_.clear();
		return OkStatus();
	}
	absl::Status Process(CalculatorContext* cc) override {
		if (!cc->Inputs().Tag("START_TIME").IsEmpty()) {
			// Fix: Use .Get<double>() to access the packet's value
			double start_time_us = cc->Inputs().Tag("START_TIME").Get<double>();
			auto end_time = std::chrono::steady_clock::now();
			double end_time_us = std::chrono::duration_cast<std::chrono::microseconds>(
				end_time.time_since_epoch()).count();
			double latency_us = end_time_us - start_time_us;
			double fps = 1000000 / latency_us;
			// Store latency in deque
			latencies_.push_back(latency_us);
			if (latencies_.size() > window_size_) {
				latencies_.pop_front();  // Remove oldest latency
			}

			// Compute mean and variance every window_size_ frames
			int latencies_size = latencies_.size();
			double mean = 0.0;
			for (double lat: latencies_) {
				mean += lat;
			}
			mean /= latencies_size;
			double mean_fps = 1000000 / mean;
			if (verbose_) {
				LOG(INFO) << label_ << " | latency: " << latency_us << " us | "
							<< "mean latency: " << mean << " us | "
							<< "FPS: " << fps << " FPS | "
							<< "mean FPS: " << mean_fps << " FPS";
			}

			// Forward all input streams (except START_TIME)
			for (const auto& tag : cc->Inputs().GetTags()) {
				if (tag != "START_TIME" && !cc->Inputs().Tag(tag).IsEmpty()) {
					cc->Outputs().Tag(tag).AddPacket(cc->Inputs().Tag(tag).Value());
				}
			}

			cc->Outputs().Tag("FPS").Add(new double(fps), cc->InputTimestamp());
			cc->Outputs().Tag("LATENCY").Add(new double(latency_us), cc->InputTimestamp());
		} else {
			LOG(WARNING) << "Missing START_TIME input";
		}
		return OkStatus();
	}
};
REGISTER_CALCULATOR(LatencyCalculator);
}  // namespace mediapipe
#endif