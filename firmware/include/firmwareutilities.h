#ifndef ARDUINOPC_FIRMWAREUTILITIES_H
#define ARDUINOPC_FIRMWAREUTILITIES_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <Arduino.h>

namespace FirmwareUtilities
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
    uint32_t parseCanID(const char *str);
    uint8_t parseCanByte(const char *str);
    uint32_t hexStringToUInt(const char *str);
    uint8_t hexStringToUChar(const char *str);
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
   
    unsigned long tMillis();

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

    /*
    int toBinaryString(int number);
    int toDecString(int number);
    int toHexString(int number);

    int toBinaryString(uint32_t number);
    int toDecString(uint32_t number);
    int toHexString(uint32_t number);

    int toBinaryString(uint8_t number);
    int toDecString(uint8_t number);
    int toHexString(uint8_t number);
    
    int toBinaryString(bool number);
    int toDecString(bool number);
    int toHexString(bool number);
    std::string stripFromString(const std::string &stringToStrip, const std::string &whatToStrip);
    std::string stripFromString(const std::string &stringToStrip, char whatToStrip);
    std::string stripAllFromString(const std::string &stringToStrip, const std::string &whatToStrip);
    std::string stripAllFromString(const std::string &stringToStrip, char whatToStrip);
    template <typename Container, typename InputIter, typename Delimiter>
    Container parseToContainer(InputIter first, InputIter last, Delimiter delimiter)
    {
        Container returnContainer;
        InputIter it;
        do {
            it = std::find(first, last, delimiter);
            typename Container::value_type tempContainer;
            std::copy(first, it, std::inserter(tempContainer, tempContainer.end()));
            if (!tempContainer.empty()) {
                returnContainer.insert(returnContainer.end(), tempContainer);
            }
            first = it+1;
        } while (it != last);
        return returnContainer;
    }
    template <typename T>
    std::string toString(const T &convert)
    {
        std::string returnString{""};
        std::stringstream transfer;
        transfer << convert;
        transfer >> returnString;
        return returnString;
    }

    template <typename T>
    std::string tQuoted(const T &convert)
    {
        return "\"" + toString(convert) + "\"";
    }
    */

}

#endif //ARDUINOPC_FIRMWAREUTILITIES_H