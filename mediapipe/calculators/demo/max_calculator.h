#ifndef MEDIAPIPE_DEMO_CALCULATORS_MAX_CALCULATOR_H_
#define MEDIAPIPE_DEMO_CALCULATORS_MAX_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"

namespace mediapipe {

class MaxCalculator : public CalculatorBase {
 public:
  static absl::Status GetContract(CalculatorContract* cc);
  absl::Status Open(CalculatorContext* cc) override;
  absl::Status Process(CalculatorContext* cc) override;
};

}  // namespace mediapipe

#endif  // MEDIAPIPE_CALCULATORS_CUSTOM_MY_CUSTOM_CALCULATOR_H_
