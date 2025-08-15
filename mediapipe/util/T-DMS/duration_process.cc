#include "mediapipe/framework/port/logging.h"
#include "mediapipe/util/T-DMS/duration_process.h"

namespace mediapipe {

bool DurationProcess::CheckWithDuration(bool status, Timestamp current_time, std::string verbose_label) {
    bool result = false;
    if (status) {
        if (!timer_started_) {
            start_time_ = current_time;
            timer_started_ = true;
        }
        float closed_duration = (current_time - start_time_).Milliseconds() / 1000.0f;
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