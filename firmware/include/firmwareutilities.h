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
#include <iterator>
#include <vector>
#include <sstream>
#include <algorithm>

#include <Arduino.h>

namespace FirmwareUtilities
{
    const int constexpr ASCII_WHITESPACE_MAXIMUM_VALUE{32};
    
    bool isEvenlyDivisibleBy(int numberToCheck, int divisor);
    bool isEven(int numberToCheck);
    bool isOdd(int numberToCheck);
    bool startsWith(char *str, char compare);
    bool startsWith(char *str, char *compare);
    bool endsWith(char *str, char *compare);
    bool endsWith(char *str, char compare);
    bool isWhitespace(char *str);
    bool isWhitespace(char charToCheck);
    int whitespace(unsigned int howMuch, char *out);
    int charToInt(char charToConvert);
    char intToChar (int intToConvert);
    bool isDigit(char charToCheck);
    int toFixedWidth(const String &inputString, unsigned int fixedWidth);
    int toFixedWidth(const std::string &inputString, unsigned int fixedWidth);
    uint32_t parseCanID(char *str);
    uint8_t parseCanByte(char *str);
    uint32_t hexStringToUInt(char *str);
    uint8_t hexStringToUChar(char *str);
    int intExp(int base, int super);
    int tAbs(int lhs, int rhs);
    int tMax(int lhs, int rhs);
    int tMin(int lhs, int rhs);

    template <typename T>
    int toDecString(T number, char *out)
    {
        return (sprintf(out, "%i", number));
    }

    template <typename T>
    int toHexString(T number, char *out)
    {
        return (sprintf(out, "%02X", number)); 
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
    */
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