#ifndef ARDUINOPC_UTILITIES_H
#define ARDUINOPC_UTILITIES_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <Arduino.h>

#ifndef 
#    define SMALL_BUFFER_SIZE 255
#endif
#ifndef 
#    define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

namespace Utilities
{
    const int constexpr ASCII_WHITESPACE_MAXIMUM_VALUE{32};
    bool isEvenlyDivisibleBy(int numberToCheck, int divisor);
    bool isEven(int numberToCheck);
    bool isOdd(int numberToCheck);
    bool startsWith(const char *str, char compare);
    bool startsWith(const char *str, const char *compare);
    bool endsWith(const char *str, const char *compare);
    bool endsWith(const char *str, char compare);
    bool isWhitespace(const char *str);
    bool isWhitespace(char charToCheck);
    int whitespace(char *out, size_t howMuch);
    int charToInt(char charToConvert);
    char intToChar (int intToConvert);
    bool isDigit(char charToCheck);
    int toFixedWidth(const char *input, char *out, size_t fixedWidth);
    
    int leftPad(const char *input, char *out, size_t fixedWidth, char padChar = '0');

    uint32_t hexStringToUInt(const char *str);
    uint8_t hexStringToUChar(const char *str);
    uint32_t decStringToUInt(const char *str);
    uint8_t decStringToUChar(const char *str);
    uint32_t stringToUInt(const char *str);
    uint8_t stringToUChar(const char *str);
    int intExp(int base, int super);
    int tAbs(int lhs, int rhs);
    int tMax(int lhs, int rhs);
    int tMin(int lhs, int rhs);
    bool substringExists(const char *first, const char *second);
    bool substringExists(const char *first, char second);
    size_t positionOfSubstring(const char *first, const char *second);
    size_t positionOfSubstring(const char *first, char second);
    size_t substring(const char *str, size_t startPosition, char *out, size_t maximumLength);
    size_t substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength);
    bool isValidByte(char byteToCheck);
    size_t split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength);
    size_t split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength);
    unsigned long tMillis();

    template <typename Ptr>
    void free2D(Ptr **out, size_t elements)
    {
        for (size_t i = 0; i < elements; i++) {
            free(out[i]);
        }
        free(out);
    }

    template <typename Ptr>
    Ptr **calloc2D(size_t elements, size_t maximumLength)
    {
        Ptr **out = (Ptr **)calloc(elements, sizeof(Ptr *) * maximumLength);
        for (size_t i = 0; i < elements; i++) {
            out[i] = (Ptr *)calloc(maximumLength, sizeof(Ptr));
        }
        return out;
    }

    template <typename T>
    int toDecString(T number, char *out, size_t maximumLength)
    {
        return (snprintf(out, maximumLength, "%i", static_cast<unsigned int>(number)));
    }

    template <typename T>
    int toHexString(T number, char *out, size_t maximumLength)
    {
        return (snprintf(out, maximumLength, "%02X", static_cast<unsigned int>(number))); 
    }

    bool isAllZeroes(const char *str);
    bool isAllZeroesOrDecimal(const char *str);
    
    template<typename T>
    struct is_c_str { static bool const value{false}; };
    template<>
    struct is_c_str<char *> { static bool const value{true}; };
    template<>
    struct is_c_str<const char *> { static bool const value{true}; };

    template <typename T>
    struct is_unsigned_integral { static bool const value{false}; };
    template<>
    struct is_unsigned_integral<uint8_t> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<uint16_t> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<uint32_t> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<uint64_t> { static bool const value{true}; };
    /*
    template<>
    struct is_unsigned_integral<unsigned char> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<unsigned short> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<unsigned int> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<unsigned long> { static bool const value{true}; };
    template<>
    struct is_unsigned_integral<unsigned long long> { static bool const value{true}; };
    */
    
    
    template<typename T>
    struct is_integral  { static bool const value{false}; };
    template<>
    struct  is_integral<uint8_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<int8_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<uint16_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<int16_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<uint32_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<int32_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<uint64_t>  { static bool const value{true}; };
    template<>
    struct  is_integral<int64_t>  { static bool const value{true}; };
    template<>
    struct is_integral<bool> { static bool const value{true}; };

    template<typename T>
    struct is_floating_point { static bool const value{false}; };
    template<>
    struct is_floating_point<float> { static bool const value{true}; };
    template<>
    struct is_floating_point<double> { static bool const value{true}; };
    template<>
    struct is_floating_point<long double> { static bool const value{true}; };

    void genericSplitCast(const char *str, const char delimiter);
    void genericSplitCast(const char *str, const char *delimiter);
    size_t stripNonDigits(char *str); 
    size_t stripNonFloatingPointDigits(char *out);

    template <typename T>
    struct numeric_limits
    {
        static const T maximumValue{0};
        static const T minimumValue{0};
    };

    template<>
    struct numeric_limits<uint8_t>
    {
        static const uint8_t maximumValue{UINT8_MAX};
        static const uint8_t minimumValue{0};
    };

    template<>
    struct numeric_limits<uint16_t>
    {
        static const uint16_t maximumValue{UINT16_MAX};
        static const uint16_t minimumValue{0};
    };

    template<>
    struct numeric_limits<uint32_t>
    {
        static const uint32_t maximumValue{UINT32_MAX};
        static const uint32_t minimumValue{0};
    };

    template<>
    struct numeric_limits<uint64_t>
    {
        static const uint64_t maximumValue{UINT64_MAX};
        static const uint64_t minimumValue{0};
    };

    template<>
    struct numeric_limits<int8_t>
    {
        static const int8_t maximumValue{INT8_MAX};
        static const int8_t minimumValue{INT8_MIN};
    };

    template<>
    struct numeric_limits<int16_t>
    {
        static const int16_t maximumValue{INT16_MAX};
        static const int16_t minimumValue{INT16_MIN};
    };

    template<>
    struct numeric_limits<int32_t>
    {
        static const int32_t maximumValue{INT32_MAX};
        static const int32_t minimumValue{INT32_MIN};
    };

    template<>
    struct numeric_limits<int64_t>
    {
        static const int64_t maximumValue{INT64_MAX};
        static const int64_t minimumValue{INT64_MIN};
    };

    template<>
    struct numeric_limits<float>
    {
        static const float constexpr maximumValue{FLT_MAX};
        static const float constexpr minimumValue{FLT_MIN};
    };

    template<>
    struct numeric_limits<double>
    {
        static const double constexpr maximumValue{DBL_MAX};
        static const double constexpr minimumValue{DBL_MIN};
    };

    template<>
    struct numeric_limits<long double>
    {
        static const long double constexpr maximumValue{LDBL_MAX};
        static const long double constexpr minimumValue{LDBL_MIN};
    };

    template <typename First, typename ... Args>
    void genericSplitCast(const char *str, const char *delimiter, First *first, Args* ... arguments)
    {
        if ((!str) || (!delimiter)) {
            return;
        }
        char *temp{new char[strlen(str) + 1]};
        char *passOn{new char [strlen(str) + 1]};
        strcpy(passOn, str);
        strcpy(temp, str);
        int foundPosition{positionOfSubstring(temp, delimiter)};
        if (foundPosition != -1) {
            substring(str, foundPosition + strlen(delimiter), passOn, strlen(str) + 1);
            substring(str, 0, foundPosition, temp, strlen(str) + 1);
        }
        if (strlen(temp) == 0) {
            first = nullptr;
            return;
        }
        if (is_integral<First>::value) {
            (void)stripNonDigits(temp);
            if (strlen(temp) == 0) {
                first = nullptr;
                return;
            }
            Serial.print("First was interpreted as an integer (first = ");
            Serial.print(temp);
            Serial.println(")");
            long long tempValue{atol(temp)};
            if (isAllZeroes(temp)) {
                *first = 0;
            } else if (tempValue == 0) {
                first = nullptr;
            } else if (tempValue < numeric_limits<First>::minimumValue) {
                first = nullptr;
            } else if (tempValue > numeric_limits<First>::maximumValue) {
                first = nullptr;
            } else {
                *first = static_cast<First>(tempValue);
            }
        } else if (is_floating_point<First>::value) {
            Serial.println("First was interpreted as a floating point");
            (void)stripNonFloatingPointDigits(temp);
            if (strlen(temp) == 0) {
                first = nullptr;
                return;
            }
            long double tempValue{atof(temp)};
            if (isAllZeroesOrDecimal(temp)) {
                *first = 0;
            } else if (tempValue == 0) {
                first = nullptr;
            } else if (tempValue < numeric_limits<First>::minimumValue) {
                first = nullptr;
            } else if (tempValue > numeric_limits<First>::maximumValue) {
                first = nullptr;
            } else {
                *first = static_cast<First>(tempValue);
            }
        } else if (is_c_str<First>::value) {
            Serial.println("First was interpreted as a c string");
            for (size_t i = 0; i < strlen(temp); i++) {
                first[i] = temp[i];
            }
            first[strlen(temp)] = '\0';
        } else {
            Serial.println("First was not interpreted");
            first = nullptr;
        }
        genericSplitCast(passOn, delimiter, arguments...);
        delete passOn;
        delete temp;
    }

    template <typename First, typename ... Args>
    void genericSplitCast(const char *str, const char delimiter, First *first, Args* ... arguments)
    {
        if (!str) {
            return;
        }
        char temp[2]{delimiter, '\0'};
        return genericSplitCast(str, temp, first, arguments...);
    }

    template <typename InputType>
    size_t toFixedWidthHex(char *out, size_t bufferLength, size_t fixedWidth, InputType input, bool includeZeroX = true)
    {
        char duplicateChar[4];
        char formatMessage[10];
        if (includeZeroX) {
            strcpy(formatMessage, "0x%0");
        } else {
            strcpy(formatMessage, "%0");
        }
        snprintf(duplicateChar, 4, "%li", static_cast<long>(fixedWidth));
        strcat(formatMessage, duplicateChar);
        strcat(formatMessage, "x");
        snprintf(out, bufferLength, formatMessage, input);
        return strlen(out);
    }

}

#endif //ARDUINOPC_UTILITIES_H