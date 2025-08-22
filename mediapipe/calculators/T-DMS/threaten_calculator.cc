#ifndef MEDIAPIPE_CALCULATORS_T_DMS_THREATEN_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_THREATEN_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/calculators/T-DMS/threaten_calculator_options.pb.h"
#include "mediapipe/framework/port/logging.h"

namespace mediapipe {

class ThreatenCalculator: public CalculatorBase {
private:
    std::unordered_set<std::string> labels_;
    bool verbose_;
    float face_margin_top_ratio_;
    float face_margin_left_ratio_;
    float face_margin_right_ratio_;
    float face_margin_bottom_ratio_;

    bool IsObjectBeHolding(NormalizedLandmarkList hand_landmarks, Detection detection) {
        CHECK_EQ(detection.location_data().format(), LocationData::RELATIVE_BOUNDING_BOX)
            << "Detection must be relative bounding box";

        const auto& bbox = detection.location_data().relative_bounding_box();
        float det_xmin = bbox.xmin();
        float det_ymin = bbox.ymin();
        float det_xmax = bbox.xmin() + bbox.width();
        float det_ymax = bbox.ymin() + bbox.height();

        for (const auto& lm : hand_landmarks.landmark()) {
            if (lm.x() >= det_xmin && lm.x() <= det_xmax 
                && lm.y() >= det_ymin && lm.y() <= det_ymax
            ) {
                return true;
            }
        }
        return false;
    }
    Detection AddMarginToFaceDetections(const Detection& face_detection) {
        CHECK_EQ(face_detection.location_data().format(), LocationData::RELATIVE_BOUNDING_BOX)
            << "Face detection must be relative bounding box";

        const auto& face_bbox = face_detection.location_data().relative_bounding_box();

        float face_xmin = face_bbox.xmin() - face_margin_left_ratio_ * face_bbox.width();
        float face_ymin = face_bbox.ymin() - face_margin_top_ratio_ * face_bbox.height();
        float face_width = face_bbox.width() + (face_margin_left_ratio_ + face_margin_right_ratio_) * face_bbox.width();
        float face_height = face_bbox.height() + (face_margin_top_ratio_ + face_margin_bottom_ratio_) * face_bbox.height();

        face_xmin = (face_xmin < 0 ? 0: face_xmin);
        face_ymin = (face_ymin < 0 ? 0: face_ymin);
        face_width = (face_xmin + face_width > 1 ? 1 - face_xmin: face_width);
        face_height = (face_ymin + face_height > 1 ? 1 - face_ymin: face_height);

        Detection face_detection_margin_extend;
        auto* loc = face_detection_margin_extend.mutable_location_data();
        loc->set_format(LocationData::RELATIVE_BOUNDING_BOX);
        auto* margin_bbox = loc->mutable_relative_bounding_box();
        margin_bbox->set_xmin(face_xmin);
        margin_bbox->set_ymin(face_ymin);
        margin_bbox->set_width(face_width);
        margin_bbox->set_height(face_height);

        return face_detection_margin_extend;
    }
    bool Intersect(const Detection& object_detection_0, const Detection& object_detection_1) {
        CHECK(object_detection_0.location_data().format() == LocationData::RELATIVE_BOUNDING_BOX
            && object_detection_0.location_data().format() == LocationData::RELATIVE_BOUNDING_BOX
        ) << "Detection must be relative bounding box";
        const auto& obj_bbox_0 = object_detection_0.location_data().relative_bounding_box();
        const auto& obj_bbox_1 = object_detection_1.location_data().relative_bounding_box();

        // Face bbox
        float obj_xmin_0 = obj_bbox_0.xmin();
        float obj_ymin_0 = obj_bbox_0.ymin();
        float obj_xmax_0 = obj_bbox_0.xmin() + obj_bbox_0.width();
        float obj_ymax_0 = obj_bbox_0.ymin() + obj_bbox_0.height();

        // Object bbox
        float obj_xmin_1 = obj_bbox_1.xmin();
        float obj_ymin_1 = obj_bbox_1.ymin();
        float obj_xmax_1 = obj_bbox_1.xmin() + obj_bbox_1.width();
        float obj_ymax_1 = obj_bbox_1.ymin() + obj_bbox_1.height();

        // Check intersection
        return !(obj_xmax_0 < obj_xmin_1 || obj_xmin_0 > obj_xmax_1
                || obj_ymax_0 < obj_ymin_1 || obj_ymin_0 > obj_ymax_1);
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("MULTI_HAND_LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
        cc->Inputs().Tag("OBJECT_DETECTIONS").Set<std::vector<Detection>>();
        cc->Inputs().Tag("FACE_DETECTION").Set<Detection>();
        cc->Outputs().Tag("THREATEN").Set<bool>();
        cc->Outputs().Tag("FACE_MARGIN_DETECTION").Set<Detection>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& opts = cc->Options<ThreatenCalculatorOptions>();
        
        verbose_ = opts.verbose();
        labels_ = std::unordered_set<std::string>(opts.labels().begin(), opts.labels().end());
        face_margin_top_ratio_ = opts.face_margin_top_ratio();
        face_margin_left_ratio_ = opts.face_margin_left_ratio();
        face_margin_right_ratio_ = opts.face_margin_right_ratio();
        face_margin_bottom_ratio_ = opts.face_margin_bottom_ratio();

        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        bool threaten = false;
        if (!cc->Inputs().Tag("MULTI_HAND_LANDMARKS").IsEmpty()
            && !cc->Inputs().Tag("OBJECT_DETECTIONS").IsEmpty()
            && !cc->Inputs().Tag("FACE_DETECTION").IsEmpty()
        ) {
            const auto& hand_landmarks_vec = cc->Inputs().Tag("MULTI_HAND_LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
            const auto& object_detections = cc->Inputs().Tag("OBJECT_DETECTIONS").Get<std::vector<Detection>>();
            const auto& face_detection_margin_extended = AddMarginToFaceDetections(cc->Inputs().Tag("FACE_DETECTION").Get<Detection>());
            
            for (const Detection& object_detection: object_detections) {
                bool is_threaten_object = false;
                for (const std::string& label : object_detection.label()) {
                    if (labels_.find(label) != labels_.end()) {
                        is_threaten_object = true;
                        break;
                    }
                }
                if (!is_threaten_object) {
                    continue;
                }

                bool is_holding = false;
                for (const NormalizedLandmarkList& hand_landmarks: hand_landmarks_vec) {
                    if (IsObjectBeHolding(hand_landmarks, object_detection)) {
                        is_holding = true;
                        break;
                    }
                }
                if (!is_holding) {
                    continue;
                }
                

                if (Intersect(face_detection_margin_extended, object_detection)) {
                    threaten = true;
                    break;
                }
            }
            
            cc->Outputs().Tag("FACE_MARGIN_DETECTION").Add(new Detection(face_detection_margin_extended), cc->InputTimestamp());
        }

        if (verbose_) {
            LOG(INFO) << "Threaten " << threaten;
        }
        cc->Outputs().Tag("THREATEN").Add(new bool(threaten), cc->InputTimestamp());
        return absl::OkStatus();
    }

};
REGISTER_CALCULATOR(ThreatenCalculator);
}
#endif