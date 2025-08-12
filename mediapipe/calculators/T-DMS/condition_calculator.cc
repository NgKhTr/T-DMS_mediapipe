#ifndef MEDIAPIPE_CALCULATORS_T_DMS_CONDITION_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_CONDITION_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/T-DMS/condition_calculator_options.pb.h"

namespace mediapipe {

class ConditionCalculator : public CalculatorBase {
private:
    ConditionCalculatorOptions::LogicType logic_type_;
    uint8_t num_conditions_;
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Outputs().Index(0).Set<bool>();
        RET_CHECK_EQ(cc->Outputs().NumEntries(), 1)
            << "ConditionCalculator expects exactly one output stream for conditions.";
        const uint8_t num_data_streams = cc->Inputs().NumEntries("");
        RET_CHECK_GT(num_data_streams, 0) << "At least one input is required.";
        for (uint8_t i = 0; i < num_data_streams; ++i) {
            cc->Inputs().Get("", i).Set<bool>();
        }
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
		cc->SetOffset(TimestampDiff(0));
        logic_type_ = cc->Options<ConditionCalculatorOptions>().logic_type();
        num_conditions_ = cc->Inputs().NumEntries();
        return absl::OkStatus();
    } 

    absl::Status Process(CalculatorContext* cc) override {
        bool result = (cc->Inputs().Get("", 0).IsEmpty() 
                ? false : cc->Inputs().Get("", 0).Get<bool>());
        for (uint8_t i = 1; i < num_conditions_; ++i) {
            bool recent_cond = (cc->Inputs().Get("", i).IsEmpty() 
                ? false : cc->Inputs().Get("", i).Get<bool>());
            if (logic_type_ == ConditionCalculatorOptions::AND) {
                result = (result && recent_cond);
                if (!result) {
                    break;
                }
            } else if (logic_type_ == ConditionCalculatorOptions::OR) {
                result = (result || recent_cond);
                if (result) {
                    break;
                }
            } else {
                CHECK(false) << "Unknown logic type";
            }
        }
        cc->Outputs().Index(0).Add(new bool(result), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(ConditionCalculator);
}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_CONDITION_CALCULATOR_H_