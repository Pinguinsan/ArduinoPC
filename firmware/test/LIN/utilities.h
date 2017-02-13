#ifndef ARDUINOPC_UTILITIES_H
#define ARDUINOPC_UTILITIES_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
//#include <avr/interrupt.h>
//#include <Arduino.h>

#define SERIAL_PORT_BUFFER_MAX 50
#define SMALL_BUFFER_SIZE 50
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

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
    int positionOfSubstring(const char *first, const char *second);
    int positionOfSubstring(const char *first, char second);
    int substring(const char *str, size_t startPosition, char *out, size_t maximumLength);
    int substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength);
    bool isValidByte(char byteToCheck);
    int split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength);
    int split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength);
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
}

#endif //ARDUINOPC_UTILITIES_H