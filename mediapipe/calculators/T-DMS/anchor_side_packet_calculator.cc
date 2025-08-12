#ifndef MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_SIDE_PACKET_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_SIDE_PACKET_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/object_detection/anchor.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/calculators/T-DMS/anchor_side_packet_calculator_options.pb.h"
#include "mediapipe/util/resource_util.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp> // For JSON serialization (header-only, add to your project)

namespace mediapipe {

class AnchorSidePacketCalculator : public CalculatorBase {
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->OutputSidePackets().Index(0).Set<std::vector<Anchor>>();
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        const auto& opts = cc->Options<AnchorSidePacketCalculatorOptions>();
        std::string csv_path, metadata_path;
        std::vector<Anchor> anchors;

        RET_CHECK(opts.has_csv_path() != opts.has_metadata_path())
            << "Either csv_path or metadata_path must be provided in AnchorCsvSidePacketCalculatorOptions.";
        if (opts.has_metadata_path()) {
            metadata_path = opts.metadata_path();
            RET_CHECK(!metadata_path.empty()) << "metadata_path must not be empty";
            RET_CHECK(metadata_path.size() >= 5 && metadata_path.substr(metadata_path.size() - 5) == ".json")
                << "metadata_path must have .json suffix";
            absl::StatusOr<std::string> metadata_path_or = PathToResourceAsFile(metadata_path, false); // This func has check relative and absolute path check
            MP_RETURN_IF_ERROR(metadata_path_or.status());
            metadata_path = *metadata_path_or;
            std::ifstream file(metadata_path);
            nlohmann::json metadata;
            file >> metadata;
            auto& anchors_json = metadata["subgraph_metadata"][0]
                        ["custom_metadata"][0]
                        ["data"]["ssd_anchors_options"]
                        ["fixed_anchors_schema"]["anchors"];
            for (const auto& anchor_json: anchors_json) {
                Anchor anchor;
                anchor.set_x_center(anchor_json.value("x_center", 0.0f));
                anchor.set_y_center(anchor_json.value("y_center", 0.0f));
                anchor.set_w(anchor_json.value("width", 0.0f));
                anchor.set_h(anchor_json.value("height", 0.0f));
                anchors.push_back(anchor);
            }
            file.close();

        } else {
            csv_path = opts.csv_path();
            RET_CHECK(!csv_path.empty()) << "csv_path must not be empty";
            RET_CHECK(csv_path.size() >= 4 && csv_path.substr(csv_path.size() - 4) == ".csv")
                << "csv_path must have .csv suffix";
            absl::StatusOr<std::string> csv_path_or = PathToResourceAsFile(csv_path, false); // This func has check relative and absolute path check
            MP_RETURN_IF_ERROR(csv_path_or.status());
            csv_path = *csv_path_or;
            std::ifstream file(csv_path);
            RET_CHECK(file.is_open()) << "Failed to open anchor CSV file: " << csv_path;

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
        }
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
REGISTER_CALCULATOR(AnchorSidePacketCalculator);

}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_ANCHOR_CSV_SIDE_PACKET_CALCULATOR_H_