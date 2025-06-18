#ifndef MEDIAPIPE_UTIL_T_DMS_DURATION_PROCESS_H_
#define MEDIAPIPE_UTIL_T_DMS_DURATION_PROCESS_H_

#include <chrono>
#include <string>

namespace mediapipe {

class DurationProcess {
protected:
    float duration_s_threshold_; // Duration in seconds
    std::chrono::steady_clock::time_point start_time_;
    bool timer_started_;

    bool CheckWithDuration(bool status, std::string verbose_label = "");
};
}
#endif