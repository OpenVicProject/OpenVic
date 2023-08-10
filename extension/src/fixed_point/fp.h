#pragma once

#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <limits>
#include <cstring>

#include "fixed_point/fp_lut.h"
#include "utils/float_utils.h"
#include "utils/string_utils.h"

class FP {
public:
	static constexpr size_t SIZE = 8;

	constexpr FP () : value(0) {}
	constexpr FP (int64_t value) : value(value) {}
	constexpr FP (int32_t value) : value((int64_t) value << FPLUT::PRECISION) {}

	// Trivial destructor
	~FP() = default;

	static constexpr FP max() {
		return std::numeric_limits<int64_t>::max();
	}

	static constexpr FP min() {
		return std::numeric_limits<int64_t>::min();
	}

	static constexpr FP usable_max() {
		return 2147483648LL;
	}

	static constexpr FP usable_min() {
		return -usable_max();
	}

	static constexpr FP _0() {
		return 0;
	}	
	
	static constexpr FP _1() {
		return 1;
	}

	static constexpr FP _2() {
		return 2;
	}

	static constexpr FP _3() {
		return 3;
	}

	static constexpr FP _4() {
		return 4;
	}

	static constexpr FP _5() {
		return 5;
	}

	static constexpr FP _6() {
		return 6;
	}

	static constexpr FP _7() {
		return 7;
	}

	static constexpr FP _8() {
		return 8;
	}

	static constexpr FP _9() {
		return 9;
	}

	static constexpr FP _10() {
		return 10;
	}

	static constexpr FP _50() {
		return 50;
	}

	static constexpr FP _100() {
		return 100;
	}

	static constexpr FP _200() {
		return 200;
	}

	static constexpr FP _0_01() {
		return _1() / _100();
	}

	static constexpr FP _0_02() {
		return _0_01() * 2;
	}

	static constexpr FP _0_03() {
		return _0_01() * 3;
	}

	static constexpr FP _0_04() {
		return _0_01() * 4;
	}

	static constexpr FP _0_05() {
		return _0_01() * 5;
	}

	static constexpr FP _0_10() {
		return _1() / 10;
	}

	static constexpr FP _0_20() {
		return _0_10() * 2;
	}

	static constexpr FP _0_25() {
		return _1() / 4;
	}

	static constexpr FP _0_33() {
		return _1() / 3;
	}

	static constexpr FP _0_50() {
		return _1() / 2;
	}

	static constexpr FP _0_75() {
		return _1() - _0_25();
	}

	static constexpr FP _0_95() {
		return _1() - _0_05();
	}

	static constexpr FP _0_99() {
		return _1() - _0_01();
	}

	static constexpr FP _1_01() {
		return _1() + _0_01();
	}

	static constexpr FP _1_10() {
		return _1() + _0_10();
	}

	static constexpr FP _1_50() {
		return _1() + _0_50();
	}

	static constexpr FP minus_one() {
		return -1;
	}

	static constexpr FP pi() {
		return FP(205887LL);
	}

	static constexpr FP pi2() {
		return pi() * 2;
	}

	static constexpr FP pi_quarter() {
		return pi() / 4;
	}

	static constexpr FP pi_half() {
		return pi() / 2;
	}

	static constexpr FP one_div_pi2() {
		return 1 / pi2();
	}

	static constexpr FP deg2rad() {
		return FP(1143LL);
	}

	static constexpr FP rad2deg() {
		return FP(3754936LL);
	}

	static constexpr FP e() {
		return FP(178145LL);
	}

	constexpr int64_t to_int64_t() const {
		return value >> FPLUT::PRECISION;
	}

	constexpr void set_raw_value(int64_t value) {
		this->value = value;
	}

	constexpr int64_t get_raw_value() const {
		return value;
	}

	constexpr int32_t to_int32_t() const {
		return (int32_t) (value >> FPLUT::PRECISION);
	}

	constexpr float to_float() const {
		return value / 65536.0F;
	}

	constexpr float to_float_rounded() const {
		return (float) FloatUtils::round_to_int64((value / 65536.0F) * 100000.0F) / 100000.0F;
	}

	constexpr float to_double() const {
		return value / 65536.0;
	}

	constexpr float to_double_rounded() const {
		return FloatUtils::round_to_int64((value / 65536.0) * 100000.0) / 100000.0;
	}

	// Deterministic
	static constexpr FP parse_raw(int64_t value) {
		return FP(value);
	}

	// Deterministic
	static constexpr FP parse(int64_t value) {
		return FP(value << FPLUT::PRECISION);
	}

	// Deterministic
	static constexpr FP parse(const char* value) {
		if (value == nullptr || *value == '\0') {
			return _0();
		}

		bool negative = false;

		int start_index = 0;

		if (value[0] == '-') {
			negative = true;
			start_index = 1;
		}

		const char* dot_pointer = strchr(value, '.');
		int dot_index = dot_pointer == nullptr ? -1 : dot_pointer - value;

		if (dot_index < start_index) {
			if (start_index == 0) {
				return parse_integer(value);
			}

			return -parse_integer(value + start_index);
		}

		FP result;

		if (dot_index > start_index) {
			int integer_string_length = dot_index - start_index;
			char* integer_string = new char[integer_string_length + 1];
			memcpy(integer_string, value + start_index, integer_string_length);
			integer_string[integer_string_length] = '\0';

			result += parse_integer(integer_string);

			delete integer_string;
		}

		int value_length = StringUtils::get_string_length(value);

		if (dot_index == value_length - 1) {
			return negative ? -result : result;
		}

		int fraction_string_length = value_length - dot_index - 1;

		if (fraction_string_length > 0) {
			char* fraction_string = new char[fraction_string_length + 1];
			memcpy(fraction_string, value + dot_index + 1, fraction_string_length);
			fraction_string[fraction_string_length] = '\0';

			result += parse_fraction(fraction_string);

			delete fraction_string;
		}

		return negative ? -result : result;
	}

	// Not Deterministic
	static constexpr FP parse_unsafe(float value) {
		return FP((int64_t)(value * FPLUT::ONE + 0.5f * (value < 0 ? -1 : 1)));
	}

	// Not Deterministic
	static FP parse_unsafe(const char* value) {
		char* endpointer;
		double double_value = std::strtod(value, &endpointer);

		if (*endpointer != '\0') {
			// TODO: Log an error that not all of the string was parsed
		}

		int64_t integer_value = (long)(double_value * FPLUT::ONE + 0.5 * (double_value < 0 ? -1 : 1));

		return integer_value;
	}

	constexpr operator int32_t() const {
		return (int32_t)(value >> FPLUT::PRECISION); 
	}

	constexpr operator int64_t() const {
		return value >> FPLUT::PRECISION;
	}

	constexpr operator float() const {
		return value / 65536.0F;
	}

	constexpr operator double() const {
		return value / 65536.0;
	}

	constexpr friend FP operator- (const FP& obj) {
		return FP(-obj.value);
	}

	constexpr friend FP operator+ (const FP& obj) {
		return FP(+obj.value);
	}

	constexpr friend FP operator+ (const FP& lhs, const FP& rhs) {
		return FP(lhs.value + rhs.value);
	}

	constexpr friend FP operator+ (const FP& lhs, const int32_t& rhs) {
		return FP(lhs.value + ((int64_t) rhs << FPLUT::PRECISION));
	}

	constexpr friend FP operator+ (const int32_t& lhs, const FP& rhs) {
		return FP(((int64_t)lhs << FPLUT::PRECISION) + rhs.value);
	}

	constexpr FP operator+= (const FP& obj) {
		value = value + obj.value;
		return *this;
	}

	constexpr FP operator+= (const int32_t& obj) {
		value = value + ((int64_t)obj << FPLUT::PRECISION);
		return *this;
	}

	constexpr friend FP operator- (const FP& lhs, const FP& rhs) {
		return FP(lhs.value - rhs.value);
	}

	constexpr friend FP operator- (const FP& lhs, const int32_t& rhs) {
		return FP(lhs.value - ((int64_t)rhs << FPLUT::PRECISION));
	}

	constexpr friend FP operator- (const int32_t& lhs, const FP& rhs) {
		return FP(((int64_t)lhs << FPLUT::PRECISION) - rhs.value);
	}

	constexpr FP operator-= (const FP& obj) {
		value = value - obj.value;
		return *this;
	}

	constexpr FP operator-= (const int32_t& obj) {
		value = value - ((int64_t)obj << FPLUT::PRECISION);
		return *this;
	}

	constexpr friend FP operator* (const FP& lhs, const FP& rhs) {
		return lhs.value * rhs.value >> FPLUT::PRECISION;
	}

	constexpr friend FP operator* (const FP& lhs, const int32_t& rhs) {
		return FP(lhs.value * rhs);
	}

	constexpr friend FP operator* (const int32_t& lhs, const FP& rhs) {
		return FP(lhs * rhs.value);
	}

	constexpr FP operator*= (const FP& obj) {
		value = value * obj.value >> FPLUT::PRECISION;
		return *this;
	}

	constexpr FP operator*= (const int32_t& obj) {
		value = value * obj;
		return *this;
	}

	constexpr friend FP operator/ (const FP& lhs, const FP& rhs) {
		return FP((lhs.value  << FPLUT::PRECISION) / rhs.value);
	}

	constexpr friend FP operator/ (const FP& lhs, const int32_t& rhs) {
		return FP(lhs.value / rhs);
	}

	constexpr friend FP operator/ (const int32_t& lhs, const FP& rhs) {
		return FP(((int64_t) lhs << 32) / rhs.value);
	}

	constexpr FP operator/= (const FP& obj) {
		value = (value << FPLUT::PRECISION) / obj.value;
		return *this;
	}

	constexpr FP operator/= (const int32_t& obj) {
		value = value / obj;
		return *this;
	}

	constexpr friend FP operator% (const FP& lhs, const FP& rhs) {
		return FP(lhs.value % rhs.value);
	}

	constexpr friend FP operator% (const FP& lhs, const int32_t& rhs) {
		return FP(lhs.value % ((int64_t) rhs << FPLUT::PRECISION));
	}

	constexpr friend FP operator% (const int32_t& lhs, const FP& rhs) {
		return FP(((int64_t) lhs << FPLUT::PRECISION) % rhs.value);
	}

	constexpr FP operator%= (const FP& obj) {
		value = value % obj.value;
		return *this;
	}

	constexpr FP operator%= (const int32_t& obj) {
		value = value % ((int64_t)obj << FPLUT::PRECISION);
		return *this;
	}

	constexpr friend bool operator< (const FP& lhs, const FP& rhs) {
		return lhs.value < rhs.value;
	}

	constexpr friend bool operator< (const FP& lhs, const int32_t& rhs) {
		return lhs.value < (int64_t) rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator< (const int32_t& lhs, const FP& rhs) {
		return (int64_t) lhs << FPLUT::PRECISION < rhs.value;
	}

	constexpr friend bool operator<= (const FP& lhs, const FP& rhs) {
		return lhs.value <= rhs.value;
	}

	constexpr friend bool operator<= (const FP& lhs, const int32_t& rhs) {
		return lhs.value <= (int64_t)rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator<= (const int32_t& lhs, const FP& rhs) {
		return (int64_t)lhs << FPLUT::PRECISION <= rhs.value;
	}

	constexpr friend bool operator> (const FP& lhs, const FP& rhs) {
		return lhs.value > rhs.value;
	}

	constexpr friend bool operator> (const FP& lhs, const int32_t& rhs) {
		return lhs.value > (int64_t)rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator> (const int32_t& lhs, const FP& rhs) {
		return (int64_t)lhs << FPLUT::PRECISION > rhs.value;
	}

	constexpr friend bool operator>= (const FP& lhs, const FP& rhs) {
		return lhs.value >= rhs.value;
	}

	constexpr friend bool operator>= (const FP& lhs, const int32_t& rhs) {
		return lhs.value >= (int64_t)rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator>= (const int32_t& lhs, const FP& rhs) {
		return (int64_t)lhs << FPLUT::PRECISION >= rhs.value;
	}

	constexpr friend bool operator== (const FP& lhs, const FP& rhs) {
		return lhs.value == rhs.value;
	}

	constexpr friend bool operator== (const FP& lhs, const int32_t& rhs) {
		return lhs.value == (int64_t)rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator== (const int32_t& lhs, const FP& rhs) {
		return (int64_t)lhs << FPLUT::PRECISION == rhs.value;
	}

	constexpr friend bool operator!= (const FP& lhs, const FP& rhs) {
		return lhs.value != rhs.value;
	}

	constexpr friend bool operator!= (const FP& lhs, const int32_t& rhs) {
		return lhs.value != (int64_t)rhs << FPLUT::PRECISION;
	}

	constexpr friend bool operator!= (const int32_t& lhs, const FP& rhs) {
		return (int64_t)lhs << FPLUT::PRECISION != rhs.value;
	}


private:
	int64_t value;

	static constexpr FP parse_integer(const char* value) {
		char* endpointer;
		int64_t parsed_value = StringUtils::string_to_int64(value, &endpointer, 10);

		if (*endpointer != '\0') {
			// TODO: Log an error that not all of the string was parsed
		}

		return parse(parsed_value);
	}

	static constexpr FP parse_fraction(const char* value) {
		int value_length = StringUtils::get_string_length(value);

		char buffer[6];
		if (value_length < 5) {
			for (int i = 0; i < value_length; ++i) {
				buffer[i] = value[i];
			}

			for (size_t i = value_length; i < 5; ++i) {
				buffer[i] = '0';
			}

			buffer[5] = '\0';
		}
		else {
			memcpy(buffer, value, 5);

			buffer[5] = '\0';
		}

		char* endpointer;
		int64_t parsed_value = StringUtils::string_to_int64(buffer, &endpointer);

		if (*endpointer != '\0') {
			// TODO: Log an error that not all of the string was parsed
		}

		return parse_raw(parsed_value * 65536 / 100000);
	}
};

static_assert(sizeof(FP) == FP::SIZE, "FP is not 8 bytes");