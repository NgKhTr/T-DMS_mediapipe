#ifndef MEDIAPIPE_CALCULATORS_T_DMS_SORT_DETECTIONS_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_SORT_DETECTIONS_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/calculators/T-DMS/sort_detections_calculator_options.pb.h"
#include <algorithm>
#include <vector>

namespace mediapipe {

class SortDetectionsCalculator : public CalculatorBase {
private:
    SortDetectionsCalculatorOptions::Criteria criteria_;
    bool ascend_;
    std::function<bool(const Detection&, const Detection&)> cmp_;
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        cc->Outputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& options = cc->Options<SortDetectionsCalculatorOptions>();
        criteria_ = options.criteria();
        ascend_ = options.ascend();
        if (criteria_ == SortDetectionsCalculatorOptions::AREA) {
            cmp_ = [this](const Detection& a, const Detection& b) {
                float score_a = a.score_size() > 0 ? a.score(0) : 0.0f;
                float score_b = b.score_size() > 0 ? b.score(0) : 0.0f;
                return ascend_ ? (score_a < score_b) : (score_a > score_b);
            };
        } else if (criteria_ == SortDetectionsCalculatorOptions::SCORE) {
            cmp_ = [this](const Detection& a, const Detection& b) { 
                float area_a = 0.0f, area_b = 0.0f;
                // 2 detection must the same LocationData.Format
                // RET_CHECK_EQ(a.location_data().format(), b.location_data().format())
                CHECK_EQ(a.location_data().format(), b.location_data().format())
                    << "Detections must have the same LocationData.Format for area comparison.";

                switch (a.location_data().format()) {
                    case LocationData::RELATIVE_BOUNDING_BOX:
                        CHECK(a.location_data().has_relative_bounding_box() && 
                                b.location_data().has_relative_bounding_box())
                            << "Relative bounding box must be present for area comparison.";
                        area_a = a.location_data().relative_bounding_box().width() *
                            a.location_data().relative_bounding_box().height();
                        area_b = b.location_data().relative_bounding_box().width() *
                            b.location_data().relative_bounding_box().height();
                        break;
                    case LocationData::BOUNDING_BOX:
                        CHECK(a.location_data().has_bounding_box() && 
                                b.location_data().has_bounding_box())
                            << "Relative bounding box must be present for area comparison.";
                        area_a = a.location_data().bounding_box().width() *
                            a.location_data().bounding_box().height();
                        area_b = b.location_data().bounding_box().width() *
                            b.location_data().bounding_box().height();
                        break;
                    default:
                        LOG(ERROR) << "Unsupported LocationData.Format for area comparison: "
                                << a.location_data().format();
                        break;
                }
                return ascend_ ? (area_a < area_b) : (area_a > area_b);
            };
        }
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        if (cc->Inputs().Tag("DETECTIONS").IsEmpty() || cc->Inputs().Tag("DETECTIONS").Get<std::vector<Detection>>().empty()) {
            return absl::OkStatus();
        }
        std::vector<Detection> detections = cc->Inputs().Tag("DETECTIONS").Get<std::vector<Detection>>();

        std::sort(detections.begin(), detections.end(), cmp_);

        cc->Outputs().Tag("DETECTIONS").Add(
            new std::vector<Detection>(detections),
            cc->InputTimestamp());
        return absl::OkStatus();
    }
};

REGISTER_CALCULATOR(SortDetectionsCalculator);

}  // namespace mediapipe
#endif