#ifndef MEDIAPIPE_CALCULATORS_T_DMS_IMAGE_FRAME_TO_BASE64_STRING_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_IMAGE_FRAME_TO_BASE64_STRING_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_imgcodecs_inc.h"  // For use cv::imencode
#include "mediapipe/framework/port/opencv_imgproc_inc.h" // For use cv::cvtColor
#include <string>
#include <vector>

namespace mediapipe {

class ImageFrameToBase64StringCalculator : public CalculatorBase {
private:
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string base64_encode(const unsigned char* bytes_to_encode, size_t in_len) {
        std::string ret;
        int i = 0;
        unsigned char char_array_3[3], char_array_4[4];
        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                for(i = 0; i < 4; i++) {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
        if (i) {
            for(int j = i; j < 3; j++) {
                char_array_3[j] = '\0';
            }
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (int j = 0; j < i + 1; j++) {
                ret += base64_chars[char_array_4[j]];            
            }
            while((i++ < 3)) {
                ret += '=';
            }
        }
        return ret;
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Index(0).Set<ImageFrame>();
        cc->Outputs().Index(0).Set<std::string>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        if (cc->Inputs().Index(0).IsEmpty()) {
            return absl::OkStatus();
        }
        const auto& image_frame = cc->Inputs().Index(0).Get<ImageFrame>();
        cv::Mat mat = formats::MatView(&image_frame);
        const ImageFormat::Format image_format = image_frame.Format();
        if (image_format == ImageFormat::SRGB) {
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        } else if (image_format == ImageFormat::SRGBA) {
            cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGR);
        }

        std::vector<uchar> buf;
        if (!cv::imencode(".jpg", mat, buf)) {
            return absl::InternalError("Failed to encode image to JPEG");
        }
        std::string encoded = base64_encode(buf.data(), buf.size());
        cc->Outputs().Index(0).Add(new std::string(encoded), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(ImageFrameToBase64StringCalculator);
}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_IMAGE_FRAME_TO_BASE64_STRING_CALCULATOR_H_