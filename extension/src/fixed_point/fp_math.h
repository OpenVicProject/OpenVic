#pragma once

#include "fixed_point/fp.h"
#include "fixed_point/fp_lut.h"

class FPMath {
public:
    static constexpr FP sin(FP number) {
        number.set_raw_value(number.get_raw_value() % FP::pi2().get_raw_value());
        number *= FP::one_div_pi2();
        int64_t raw = FPLUT::sin(number.get_raw_value());
        FP result = FP::parse_raw(raw);
        return result;
    }
};