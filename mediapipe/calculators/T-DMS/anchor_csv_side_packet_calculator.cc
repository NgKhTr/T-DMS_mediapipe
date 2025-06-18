#ifndef MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_CSV_SIDE_PACKET_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_CSV_SIDE_PACKET_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/object_detection/anchor.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/calculators/T-DMS/anchor_csv_side_packet_calculator_options.pb.h"
#include "mediapipe/util/resource_util.h"
#include <fstream>
#include <sstream>

namespace mediapipe {

class AnchorCsvSidePacketCalculator : public CalculatorBase {
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->OutputSidePackets().Index(0).Set<std::vector<Anchor>>();
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        const auto& opts = cc->Options<AnchorCsvSidePacketCalculatorOptions>();
        const std::string& path = opts.path();
        absl::StatusOr<std::string> anchor_csv_path_or = PathToResourceAsFile(path, false);
        MP_RETURN_IF_ERROR(anchor_csv_path_or.status());
        std::string anchor_csv_path = *anchor_csv_path_or;
        std::ifstream file(anchor_csv_path);
        RET_CHECK(file.is_open()) << "Failed to open anchor CSV file: " << anchor_csv_path;
        std::vector<Anchor> anchors;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::istringstream ss(line);
            Anchor anchor;
            // Example: x_center,y_center,w,h,angle
            std::string value;
            std::getline(ss, value, ','); anchor.set_y_center(std::stof(value));
            std::getline(ss, value, ','); anchor.set_x_center(std::stof(value));
            std::getline(ss, value, ','); anchor.set_h(std::stof(value));
            std::getline(ss, value, ','); anchor.set_w(std::stof(value));
            anchors.push_back(anchor);

        }
        file.close();
        LOG(INFO) << "Parsed anchor: " << anchors[0].x_center() << ", " << anchors[0].y_center() 
            << ", " << anchors[0].w() << ", " << anchors[0].h();
        LOG(INFO) << "Anchor size: " << anchors.size();
        cc->OutputSidePackets().Index(0).Set(MakePacket<std::vector<Anchor>>(anchors));
        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(AnchorCsvSidePacketCalculator);

}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_CSV_SIDE_PACKET_CALCULATOR_H_