#ifndef MEDIAPIPE_CALCULATORS_T_DMS_IRIS_LANDMARKS_TO_RENDER_DATA_CALCULATOR
#define MEDIAPIPE_CALCULATORS_T_DMS_IRIS_LANDMARKS_TO_RENDER_DATA_CALCULATOR

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/util/render_data.pb.h"
#include "mediapipe/framework/port/logging.h"

namespace mediapipe {

class IrisLandmarksToRenderDataCalculator : public CalculatorBase {
private:
    void AddLandmarks(const NormalizedLandmarkList& list, const std::string& prefix, RenderData* rd) {
        for (int i = 0; i < list.landmark_size(); ++i) {
            const auto& lm = list.landmark(i);

            // Draw point
            auto* annotation = rd->add_render_annotations();
            annotation->set_scene_tag(prefix + "_POINT");
            annotation->mutable_point()->set_x(lm.x());
            annotation->mutable_point()->set_y(lm.y());
            annotation->mutable_point()->set_normalized(true);
            annotation->mutable_color()->set_r(0);
            annotation->mutable_color()->set_g(prefix == "LEFT" ? 255 : 0);
            annotation->mutable_color()->set_b(prefix == "RIGHT" ? 255 : 0);
            annotation->set_thickness(4);

            // Draw index as text
            auto* text_anno = rd->add_render_annotations();
            text_anno->set_scene_tag(prefix + "_INDEX");
            text_anno->mutable_text()->set_display_text(std::to_string(i));
            text_anno->mutable_text()->set_font_height(0.005f);
            text_anno->mutable_text()->set_left(lm.x());
            text_anno->mutable_text()->set_baseline(lm.y());
            text_anno->mutable_text()->set_normalized(true);
            text_anno->mutable_color()->set_r(255);
            text_anno->mutable_color()->set_g(255);
            text_anno->mutable_color()->set_b(0);
        }
    }

public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").Set<NormalizedLandmarkList>();
        cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").Set<NormalizedLandmarkList>();
        cc->Outputs().Tag("RENDER_DATA").Set<RenderData>();  
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        LOG(INFO) << "BF auto render_data";
        auto render_data = absl::make_unique<RenderData>();
        LOG(INFO) << "BF if";
        if (!cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").IsEmpty()) {
            LOG(INFO) << "BF left_list";
            const auto& left_list = cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").Get<NormalizedLandmarkList>();
            LOG(INFO) << "BF AddLandmarks";
            AddLandmarks(left_list, "LEFT", render_data.get());
        }
        if (!cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").IsEmpty()) {
            LOG(INFO) << "BF right_list";
            const auto& right_list = cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").Get<NormalizedLandmarkList>();
            LOG(INFO) << "BF AddLandmarks";
            AddLandmarks(right_list, "RIGHT", render_data.get());
        }
        LOG(INFO) << "BF add render_data";
        cc->Outputs().Tag("RENDER_DATA").Add(render_data.release(), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(IrisLandmarksToRenderDataCalculator);
}  // namespace mediapipe
#endif