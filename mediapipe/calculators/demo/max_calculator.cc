#ifndef MEDIAPIPE_CALCULATORS_CUSTOM_MAX_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_CUSTOM_MAX_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

class MaxCalculator: public CalculatorBase {
    public:
        static absl::Status GetContract(CalculatorContract* cc) {
            cc->Inputs().Index(0).Set<double>();
            cc->Inputs().Index(1).Set<double>();
            cc->Outputs().Index(0).Set<double>();
            return absl::OkStatus();
        }
        absl::Status Process(CalculatorContext* cc) override {
            Packet pIn0 = cc->Inputs().Index(0).Value(),
                pIn1 = cc->Inputs().Index(1).Value();
            const double num1 = pIn0.Get<double>(),
                num2 = pIn1.Get<double>();
            
            const double maxNum = (num1 > num2 ? num1: num2);
            Packet pOut = MakePacket<double>(maxNum).At(cc->InputTimestamp());
            cc->Outputs().Index(0).AddPacket(pOut);
            return absl::OkStatus();
        }
};
REGISTER_CALCULATOR(MaxCalculator); 
}
#endif  // MEDIAPIPE_CALCULATORS_CUSTOM_MAX_CALCULATOR_H_