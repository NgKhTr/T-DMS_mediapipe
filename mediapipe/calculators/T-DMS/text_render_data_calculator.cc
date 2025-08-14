#ifndef MEDIAPIPE_CALCULATORS_T_DMS_TEXT_RENDER_DATA_CALCULATOR_H
#define MEDIAPIPE_CALCULATORS_T_DMS_TEXT_RENDER_DATA_CALCULATOR_H

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/util/render_data.pb.h"
#include "mediapipe/util/color.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/calculators/T-DMS/text_render_data_calculator_options.pb.h"

namespace mediapipe {

class TextRenderDataCalculator : public CalculatorBase {
private:
    Color warning_color_;
    Color normal_color_;
    Color content_color_;
    double font_height_;
    bool normalized_;
    int font_face_;
    double line_gap_;
    double left_;
    double init_baseline_;
    double outline_thickness_;
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("FPS").Set<double>();
        cc->Inputs().Tag("THREATEN").Set<bool>();
        cc->Inputs().Tag("SOS").Set<bool>();
        cc->Inputs().Tag("ALL_EYES_CLOSED").Set<bool>();
        cc->Inputs().Tag("YAWN").Set<bool>();
        cc->Inputs().Tag("HAND_HOLD_CELL_PHONE").Set<bool>();
        cc->Inputs().Tag("HAND_HOLD_DRINK").Set<bool>();
        cc->Outputs().Tag("TEXT_RENDER_DATA").Set<RenderData>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& options = cc->Options<TextRenderDataCalculatorOptions>();
        warning_color_ = options.warning_color();
        normal_color_ = options.normal_color();
        content_color_ = options.content_color();
        font_height_ = options.font_height();
        normalized_ = options.normalized();
        font_face_ = options.font_face();
        line_gap_ = options.line_gap();
        left_ = options.left();
        init_baseline_ = options.init_baseline();
        outline_thickness_ = options.outline_thickness();
        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        auto render_data = absl::make_unique<RenderData>();
        if (!cc->Inputs().Tag("FPS").IsEmpty()) {
            double fps = cc->Inputs().Tag("FPS").Get<double>();
            auto* annotation = render_data->add_render_annotations();
            annotation->mutable_text()->set_left(left_);
            annotation->mutable_text()->set_baseline(init_baseline_ + font_height_);
            annotation->mutable_text()->set_display_text(absl::StrFormat("FPS: %.2f", fps));
            annotation->mutable_text()->set_font_height(font_height_);
            annotation->mutable_text()->set_normalized(normalized_);
            annotation->mutable_text()->set_font_face(font_face_);
            annotation->mutable_text()->set_outline_thickness(outline_thickness_);
            annotation->mutable_color()->set_r(content_color_.r());
            annotation->mutable_color()->set_g(content_color_.g());
            annotation->mutable_color()->set_b(content_color_.b());
        }
        const std::vector<std::pair<std::string, std::string>> input_tags_for_render = {
            {"THREATEN", "Threatening"},
            {"SOS", "SOS"},
            {"ALL_EYES_CLOSED", "All Eyes Closed"},
            {"YAWN", "Yawning"},
            {"HAND_HOLD_CELL_PHONE", "Hand Holding Cell Phone"},
            {"HAND_HOLD_DRINK", "Hand Holding Drink"}
        };
        for (int i = 0, n = input_tags_for_render.size(); i < n; ++i) {
            
            auto* annotation = render_data->add_render_annotations();
            annotation->mutable_text()->set_left(left_);
            annotation->mutable_text()->set_baseline(init_baseline_ + (1 + i) * line_gap_ + font_height_);
            annotation->mutable_text()->set_display_text(input_tags_for_render[i].second);
            annotation->mutable_text()->set_font_height(font_height_);
            annotation->mutable_text()->set_normalized(normalized_);
            annotation->mutable_text()->set_font_face(font_face_);
            annotation->mutable_text()->set_outline_thickness(outline_thickness_);
            if (cc->Inputs().Tag(input_tags_for_render[i].first).IsEmpty() || !(cc->Inputs().Tag(input_tags_for_render[i].first).Get<bool>())) {
                annotation->mutable_color()->set_r(normal_color_.r());
                annotation->mutable_color()->set_g(normal_color_.g());
                annotation->mutable_color()->set_b(normal_color_.b());
            } else {
                annotation->mutable_color()->set_r(warning_color_.r());
                annotation->mutable_color()->set_g(warning_color_.g());
                annotation->mutable_color()->set_b(warning_color_.b());
            }
        }

        cc->Outputs().Tag("TEXT_RENDER_DATA").Add(render_data.release(), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(TextRenderDataCalculator);
}  // namespace mediapipe
#endif