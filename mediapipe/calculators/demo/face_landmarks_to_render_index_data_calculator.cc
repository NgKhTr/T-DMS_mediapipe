#ifndef MEDIAPIPE_CALCULATORS_T_DMS_FACE_LANDMARKS_TO_RENDER_INDEX_DATA_CALCULATOR
#define MEDIAPIPE_CALCULATORS_T_DMS_FACE_LANDMARKS_TO_RENDER_INDEX_DATA_CALCULATOR

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/util/render_data.pb.h"
#include "mediapipe/framework/port/logging.h"

namespace mediapipe {

class FaceLandmarksToRenderIndexDataCalculator : public CalculatorBase {
private:
    void AddLandmarks(const NormalizedLandmarkList& list, RenderData* rd) {
        for (int i = 0; i < list.landmark_size(); ++i) {
            const auto& lm = list.landmark(i);

            // Draw point
            auto* annotation = rd->add_render_annotations();
            annotation->mutable_point()->set_x(lm.x());
            annotation->mutable_point()->set_y(lm.y());
            annotation->mutable_point()->set_normalized(true);
            annotation->mutable_color()->set_r(0);
            annotation->mutable_color()->set_g(255);
            annotation->mutable_color()->set_b(255);
            annotation->set_thickness(4);

            // Draw index as text
            auto* text_anno = rd->add_render_annotations();
            text_anno->mutable_text()->set_display_text(std::to_string(i));
            text_anno->mutable_text()->set_font_height(0.003f);
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
        cc->Inputs().Tag("LANDMARKS").Set<std::vector<NormalizedLandmarkList>>();
        cc->Outputs().Tag("RENDER_DATA").Set<RenderData>();  
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        auto render_data = absl::make_unique<RenderData>();
        if (!cc->Inputs().Tag("LANDMARKS").IsEmpty()) {
            const auto& landmarks_lst = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
            for (const auto& landmarks: landmarks_lst) {
                AddLandmarks(landmarks, render_data.get());
            }
        }
        cc->Outputs().Tag("RENDER_DATA").Add(render_data.release(), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(FaceLandmarksToRenderIndexDataCalculator);
}  // namespace mediapipe
#endif