#ifndef MEDIAPIPE_UTIL_T_DMS_DURATION_PROCESS_H_
#define MEDIAPIPE_UTIL_T_DMS_DURATION_PROCESS_H_

#include <chrono>
#include <string>
#include "mediapipe/framework/timestamp.h"

namespace mediapipe {

class DurationProcess {
protected:
    float duration_s_threshold_; // Duration in seconds
    Timestamp start_time_;
    bool timer_started_;

    bool CheckWithDuration(bool status, Timestamp current_time, std::string verbose_label = "");
};
}
#endif