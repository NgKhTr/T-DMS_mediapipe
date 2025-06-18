#include "mediapipe/framework/port/logging.h"
#include "mediapipe/util/T-DMS/duration_process.h"

namespace mediapipe {

bool DurationProcess::CheckWithDuration(bool status, std::string verbose_label) {
    auto now = std::chrono::steady_clock::now();
    bool result = false;
    if (status) {
        if (!timer_started_) {
            start_time_ = now;
            timer_started_ = true;
        }
        float closed_duration = std::chrono::duration<float>(now - start_time_).count();
        if (closed_duration >= duration_s_threshold_) {
            result = true;
        }
        if (!verbose_label.empty()) {
            LOG(INFO) << verbose_label << ": " << closed_duration << "s (threshold: " << duration_s_threshold_ << ")";
        }
    } else {
        timer_started_ = false;
    }
    return result;
}
}