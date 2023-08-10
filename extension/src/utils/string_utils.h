#include <cstdint>
#include <limits>

class StringUtils {
public:
    // The constexpr function 'string_to_long' will convert a string into a int64 integer value.
    // The function takes three parameters: the input string, an end pointer, and the base for numerical conversion.
    // The base parameter defaults to 10 (decimal), but it can be any value between 2 and 36.
    // The end pointer parameter is used to indicate where conversion stopped in the input string. It can be nullptr if this information is not needed.
    static constexpr int64_t string_to_int64(const char* str, char** endptr, int base = 10) {
        // Base value should be between 2 and 36. If it's not, return 0 as an invalid case.
        if (base < 0 || base == 1 || base > 36)
            return 0;

        int64_t result = 0;  // The result of the conversion will be stored in this variable.
        bool is_negative = false;  // This flag will be set if the number is negative.

        // Skip white spaces at the beginning.
        while (*str && is_space(static_cast<unsigned char>(*str))) {
            ++str;
        }

        // Check if there is a sign character.
        if (*str == '+' || *str == '-') {
            if (*str == '-')
                is_negative = true;
            ++str;
        }

        // If base is zero, base is determined by the string prefix.
        if (base == 0) {
            if (*str == '0') {
                if (str[1] == 'x' || str[1] == 'X') {
                    base = 16;  // Hexadecimal.
                    str += 2;  // Skip '0x' or '0X'
                }
                else {
                    base = 8;  // Octal.
                }
            }
            else {
                base = 10;  // Decimal.
            }
        }
        else if (base == 16) {
            // If base is 16 and string starts with '0x' or '0X', skip these characters.
            if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
                str += 2;
            }
        }

        // Convert the number in the string.
        for (; *str; ++str) {
            int digit;
            if (*str >= '0' && *str <= '9') {
                digit = *str - '0';  // Calculate digit value for '0'-'9'.
            }
            else if (*str >= 'a' && *str <= 'z') {
                digit = *str - 'a' + 10;  // Calculate digit value for 'a'-'z'.
            }
            else if (*str >= 'A' && *str <= 'Z') {
                digit = *str - 'A' + 10;  // Calculate digit value for 'A'-'Z'.
            }
            else {
                break;  // Stop conversion if current character is not a digit.
            }

            if (digit >= base) {
                break;  // Stop conversion if current digit is greater than or equal to the base.
            }

            // Check for overflow on multiplication
            if (result > std::numeric_limits<int64_t>::max() / base) {
                return is_negative ? std::numeric_limits<int64_t>::min() : std::numeric_limits<int64_t>::max();
            }

            result *= base;

            // Check for overflow on addition
            if (result > std::numeric_limits<int64_t>::max() - digit) {
                return is_negative ? std::numeric_limits<int64_t>::min() : std::numeric_limits<int64_t>::max();
            }

            result += digit;
        }

        // If endptr is not null, assign the current string pointer to it.
        // This tells the caller where in the string the conversion stopped.
        if (endptr != nullptr) {
            *endptr = const_cast<char*>(str);
        }

        // Return the result. If the number was negative, the result is negated.
        return is_negative ? -result : result;
    }

    static constexpr bool is_space(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r';
    }

    static constexpr std::size_t get_string_length(const char* str) {
        std::size_t len = 0;
        while (str[len] != '\0') {
            ++len;
        }
        return len;
    }
};
