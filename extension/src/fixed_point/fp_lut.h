#pragma once

#include <cstdint>
#include <numbers>
#include <cmath>
#include <array>
#include <utility>
#include <cstddef>

#include "fixed_point/fp_lut_sin_512.h"

class FPLUT {
public:
	static constexpr int32_t FRACTIONS_COUNT = 5;
	static constexpr int32_t PRECISION = 16;
	static constexpr int64_t ONE = 1 << PRECISION;

    // The LUT index is between 0 and 2^16, the index table goes until 512, if we shift by 7 our index will be between 0 and 511
    static constexpr int32_t SHIFT = 16 - 9;

    static constexpr int64_t sin(int64_t value) {
        int sign = 1;
        if (value < 0) {
            value = -value;
            sign = -1;
        }

        int index = (int)(value >> SHIFT);
        int64_t a = SIN_LUT[index];
        int64_t b = SIN_LUT[index + 1];
        int64_t fraction = (value - (index << SHIFT)) << 9;
        int64_t result = a + (((b - a) * fraction) >> PRECISION);
        return result * sign;
    }
};