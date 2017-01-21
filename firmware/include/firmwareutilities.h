#ifndef ARDUINOPC_FIRMWAREUTILITIES_H
#define ARDUINOPC_FIRMWAREUTILITIES_H

/* 
 * Standard C++ headers
 * Special thanks to maniacbug for the
 * nice port of the c++ stdlib 
 * https://github.com/maniacbug/StandardCplusplus
 */
#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <Arduino.h>

namespace FirmwareUtilities
{
    const int constexpr ASCII_WHITESPACE_MAXIMUM_VALUE{32};
    std::string toLowercase(const std::string &str);
    std::string toUppercase(const std::string &str);
    
    bool isEvenlyDivisibleBy(int numberToCheck, int divisor);
    bool isEven(int numberToCheck);
    bool isOdd(int numberToCheck);
    
    std::string toBinaryString(int number);
    std::string toDecString(int number);
    std::string toHexString(int number);

    std::string toBinaryString(uint32_t number);
    std::string toDecString(uint32_t number);
    std::string toHexString(uint32_t number);

    std::string toBinaryString(uint8_t number);
    std::string toDecString(uint8_t number);
    std::string toHexString(uint8_t number);
    
    std::string toBinaryString(bool number);
    std::string toDecString(bool number);
    std::string toHexString(bool number);

    std::string toString(const char *convert);
    std::string toString(const String &convert);
    std::string toString(char convert);
    std::string tQuoted(const std::string &convert);
    std::string tQuoted(const char *convert);
    std::string tQuoted(char convert);
    bool startsWith(const std::string &str, const std::string &compare);
    bool startsWith(const std::string &str, char compare);
    bool startsWith(const std::string &str, const char *compare);
    bool endsWith(const std::string &str, const std::string &compare);
    bool endsWith(const std::string &str, char compare);
    std::vector<std::string> parseToVector(const std::string &thingToParse, char delimiter);
    bool isWhitespace(const std::string &stringToCheck);
    bool isWhitespace(char charToCheck);
    std::string stripFromString(const std::string &stringToStrip, const std::string &whatToStrip);
    std::string stripFromString(const std::string &stringToStrip, char whatToStrip);
    std::string stripAllFromString(const std::string &stringToStrip, const std::string &whatToStrip);
    std::string stripAllFromString(const std::string &stringToStrip, char whatToStrip);
    std::string whitespace(unsigned int howMuch);
    int charToInt(char charToConvert);
    char intToChar (int intToConvert);
    bool isDigit(char charToCheck);
    String toFixedWidth(const String &inputString, unsigned int fixedWidth);
    std::string toFixedWidth(const std::string &inputString, unsigned int fixedWidth);
    uint32_t parseCanID(const std::string &str);
    uint8_t parseCanByte(const std::string &str);
    uint32_t hexStringToUInt(const std::string &str);
    uint8_t hexStringToUChar(const std::string &str);

    int intExp(int base, int super);
    int tAbs(int lhs, int rhs);
    int tMax(int lhs, int rhs);
    int tMin(int lhs, int rhs);

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


}

#endif //ARDUINOPC_FIRMWAREUTILITIES_H