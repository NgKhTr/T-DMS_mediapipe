#ifndef MEDIAPIPE_CALCUALTORS_DEMO_CALCUALTORS_CHECK_FLOW_CALCULATOR_H_
#define MEDIAPIPE_CALCUALTORS_DEMO_CALCUALTORS_CHECK_FLOW_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/formats/tensor.h"
#include "tensorflow/lite/interpreter.h"
#include <chrono>

namespace mediapipe {

class CheckFlowCalculator : public CalculatorBase {
  public:
      static absl::Status GetContract(CalculatorContract* cc) {
        for (const auto& tag : cc->Inputs().GetTags()) {
          cc->Inputs().Tag(tag).SetAny();
          cc->Outputs().Tag(tag).SetAny();
        }
        return OkStatus();
      }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        return OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
      LOG(INFO) << "Processing CheckFlowCalculator";
      for (const auto& tag : cc->Inputs().GetTags()) {
          if (!cc->Inputs().Tag(tag).IsEmpty()) {
            const auto& packet = cc->Inputs().Tag(tag).Value();
            cc->Outputs().Tag(tag).AddPacket(packet);
            LOG(WARNING) << "Input stream " << tag << " not empty. "
                        << "Packet type: " << packet.DebugTypeName();

            if (packet.ValidateAsType<std::vector<TfLiteTensor>>().ok()) {
              const auto& tensors = packet.Get<std::vector<TfLiteTensor>>();
              for (int i = 0; i < tensors.size(); ++i) {
                const TfLiteTensor& tensor = tensors[i];
                std::ostringstream oss;
                oss << "Tensor[" << i << "] shape: [";
                for (int d = 0; d < tensor.dims->size; ++d) {
                  oss << tensor.dims->data[d];
                  if (d < tensor.dims->size - 1) oss << " ";
                }
                oss << "]";
                LOG(INFO) << oss.str();
              }
            }
          } else {
              LOG(WARNING) << "Input stream " << tag << " is empty.";
          }
      }
    return OkStatus();
}
};
REGISTER_CALCULATOR(CheckFlowCalculator);
}  // namespace mediapipe
#endif