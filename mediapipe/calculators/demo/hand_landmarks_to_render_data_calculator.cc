#ifndef MEDIAPIPE_CALCULATORS_T_DMS_HAND_LANDMARKS_TO_RENDER_DATA_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_HAND_LANDMARKS_TO_RENDER_DATA_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/util/render_data.pb.h"
#include "mediapipe/framework/port/logging.h"

namespace mediapipe {

class HandLandmarksToRenderDataCalculator : public CalculatorBase {
private:
    void AddHandLandmarks(const NormalizedLandmarkList& list, int hand_index, RenderData* rd) {
        for (int i = 0; i < list.landmark_size(); ++i) {
            const auto& lm = list.landmark(i);

            // Draw point
            auto* annotation = rd->add_render_annotations();
            annotation->set_scene_tag("HAND_POINT");
            annotation->mutable_point()->set_x(lm.x());
            annotation->mutable_point()->set_y(lm.y());
            annotation->mutable_point()->set_normalized(true);
            // Color: alternate for each hand
            annotation->mutable_color()->set_r(hand_index == 0 ? 0 : 255);
            annotation->mutable_color()->set_g(hand_index == 0 ? 255 : 0);
            annotation->mutable_color()->set_b(0);
            annotation->set_thickness(4);

            // Draw index as text
            auto* text_anno = rd->add_render_annotations();
            text_anno->set_scene_tag("HAND_INDEX");
            text_anno->mutable_text()->set_display_text(std::to_string(i));
            text_anno->mutable_text()->set_font_height(0.01f);
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
            const auto& hand_landmarks_vec = cc->Inputs().Tag("LANDMARKS").Get<std::vector<NormalizedLandmarkList>>();
            for (int h = 0; h < hand_landmarks_vec.size(); ++h) {
                AddHandLandmarks(hand_landmarks_vec[h], h, render_data.get());
            }
        }
        cc->Outputs().Tag("RENDER_DATA").Add(render_data.release(), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(HandLandmarksToRenderDataCalculator);

}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_HAND_LANDMARKS_TO_RENDER_DATA_CALCULATOR_H_