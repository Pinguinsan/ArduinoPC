#include "../include/firmwareutilities.h"

namespace FirmwareUtilities
{
    bool isEvenlyDivisibleBy(int numberToCheck, int divisor)
    {
        if ((numberToCheck == 0) || (divisor == 0)) {
            return false;
        }
        return ((numberToCheck % divisor) == 0);
    }

    bool isEven(int numberToCheck)
    {
        return isEvenlyDivisibleBy(numberToCheck, 2);
    }

    bool isOdd(int numberToCheck) 
    {
        return !isEven(numberToCheck);
    }
 
    std::string toLowercase(const std::string &str)
    {
        std::string copyString{str};
        std::transform(copyString.begin(), copyString.end(), copyString.begin(), ::tolower);
        return copyString;
    }
    
    std::string toUppercase(const std::string &str)
    {
        std::string copyString{str};
        std::transform(copyString.begin(), copyString.end(), copyString.begin(), ::toupper);
        return copyString;
    }

    std::string toBinaryString(int number)
    {
        if (number == 0) {
            return "0";
        }
        int digitCount = 0;
        int copy = number;
        do {
            digitCount++;
        } while (copy /= 2);
        copy = number;
        std::string returnString{""};
        for (int i = 0; i < digitCount; i++) {
            copy = number % 2;
            if (copy == 0) returnString.insert(returnString.begin(), '0');
            else if ((copy % 1) == 0) returnString.insert(returnString.begin(), '1');
            number /= 2;
        }
        return returnString;
    }

    std::string toDecString(int number)
    {
        if (number == 0) {
            return "0";
        }
        int digitCount = 0;
        int copy = number;
        while (copy > 0) {
            copy /= 10;
            digitCount++;
        }
        copy = number;
        std::string returnString{""};
        for (int i = 0; i < digitCount; i++) {
            copy = number % 10;
            if (copy == 0) returnString.insert(returnString.begin(), '0');
            else if ((copy % 9 ) == 0) returnString.insert(returnString.begin(), '9');
            else if ((copy % 8) == 0) returnString.insert(returnString.begin(), '8');
            else if ((copy % 7) == 0) returnString.insert(returnString.begin(), '7');
            else if ((copy % 6) == 0) returnString.insert(returnString.begin(), '6');
            else if ((copy % 5) == 0) returnString.insert(returnString.begin(), '5');
            else if ((copy % 4) == 0) returnString.insert(returnString.begin(), '4');
            else if ((copy % 3) == 0) returnString.insert(returnString.begin(), '3');
            else if ((copy % 2) == 0) returnString.insert(returnString.begin(), '2');
            else if ((copy % 1) == 0) returnString.insert(returnString.begin(), '1');
            number /= 10;
        }
        return returnString;
    }

    std::string toHexString(int number)
    {
        if (number == 0) {
            return "0";
        }
        int digitCount = 0;
        int copy = number;
        do {
            digitCount++;
        } while (copy /= 16);
        copy = number;
        std::string returnString{""};
        for (int i = 0; i < digitCount; i++) {
            copy = number % 16;
            if (copy == 0) returnString.insert(returnString.begin(), '0');
            else if ((copy % 15) == 0) returnString.insert(returnString.begin(), 'f');
            else if ((copy % 14) == 0) returnString.insert(returnString.begin(), 'e');
            else if ((copy % 13) == 0) returnString.insert(returnString.begin(), 'd');
            else if ((copy % 12) == 0) returnString.insert(returnString.begin(), 'c');
            else if ((copy % 11) == 0) returnString.insert(returnString.begin(), 'b');
            else if ((copy % 10) == 0) returnString.insert(returnString.begin(), 'a');                        
            else if ((copy % 9) == 0) returnString.insert(returnString.begin(), '9');
            else if ((copy % 8) == 0) returnString.insert(returnString.begin(), '8');
            else if ((copy % 7) == 0) returnString.insert(returnString.begin(), '7');
            else if ((copy % 6) == 0) returnString.insert(returnString.begin(), '6');
            else if ((copy % 5) == 0) returnString.insert(returnString.begin(), '5');
            else if ((copy % 4) == 0) returnString.insert(returnString.begin(), '4');
            else if ((copy % 3) == 0) returnString.insert(returnString.begin(), '3');
            else if ((copy % 2) == 0) returnString.insert(returnString.begin(), '2');
            else if ((copy % 1) == 0) returnString.insert(returnString.begin(), '1');
            number /= 16;
        }
        return returnString;
    }

    std::string toBinaryString(bool number) { return (number ? toBinaryString("1") : toBinaryString("0")); }
    std::string toDecString(bool number) { return (number ? toDecString("1") : toDecString("0")); }
    std::string toHexString(bool number) { return (number ? toHexString("1") : toHexString("0")); }
    
    std::string toBinaryString(uint32_t number)
    {
        return toBinaryString(static_cast<int>(number));
    }

    std::string toDecString(uint32_t number)
    {
        return toDecString(static_cast<int>(number));
    }


    std::string toHexString(uint32_t number)
    {
        return toHexString(static_cast<int>(number));
    }

    std::string toBinaryString(uint8_t number)
    {
        return toBinaryString(static_cast<int>(number));
    }

    std::string toDecString(uint8_t number)
    {
        return toDecString(static_cast<int>(number));
    }    
    
    std::string toHexString(uint8_t number)
    {
        return toHexString(static_cast<int>(number));
        
    }

    std::string toString(const char *convert)
    {
        return std::string(convert);
    }

    std::string toString(const String &convert)
    {
        return std::string(convert.c_str());
    }

    std::string toString(char convert)
    {
        return std::string(1, convert);
    }

    std::string tQuoted(const std::string &convert)
    {
        return "\"" + convert + "\"";
    }

    std::string tQuoted(const char *convert)
    {
        return "\"" + static_cast<std::string>(convert) + "\"";
    }

    std::string tQuoted(char convert)
    {
        return "\"" + std::string(1, convert) + "\"";
    }

    bool startsWith(const std::string &str, const std::string &compare)
    {
        return (str.find(compare) == 0);
    }

    bool startsWith(const std::string &str, const char *compare)
    {
        return startsWith(str, static_cast<std::string>(compare));
    }

    bool startsWith(const std::string &str, char compare)
    {
        return startsWith(str, std::string{1, compare});
    }

    bool endsWith(const std::string &str, const std::string &compare)
    {
        if (str.length() < compare.length()) {
            return false;
        }
        size_t foundPosition{str.find(compare)};
        if (foundPosition == std::string::npos) {
            return false;
        }
        return ((str.substr(foundPosition) == compare) && (str.substr(foundPosition).length() == compare.length()));
    }

    bool endsWith(const std::string &str, char compare)
    {
        return endsWith(str, std::string{1,compare});
    }

    std::vector<std::string> parseToVector(const std::string &thingToParse, char delimiter)
    {
        std::vector<std::string> returnVector;
        std::string copyThing{thingToParse};
        if (copyThing.length() > 0) {
            if (copyThing[0] == delimiter) {
                copyThing = copyThing.substr(1);
            }
        }
        while (copyThing.length() > 0) {
            size_t foundPosition{copyThing.find(delimiter)};
            if (foundPosition != std::string::npos) {
                returnVector.push_back(copyThing.substr(0, foundPosition));
                copyThing = copyThing.substr(foundPosition+1);
            } else {
                returnVector.push_back(copyThing);
                copyThing = "";
            }
        }           
        return returnVector;
    }

    bool isWhitespace(const std::string &stringToCheck)
    {
        for (std::string::const_iterator iter = stringToCheck.begin(); iter != stringToCheck.end(); iter++) {
            if (static_cast<unsigned>(*iter) > ASCII_WHITESPACE_MAXIMUM_VALUE) {
                return false;
            }
        }
        return true;
    }

    bool isWhitespace(char charToCheck)
    {
        return isWhitespace(std::string(1, charToCheck));
    }

    std::string stripFromString(const std::string &stringToStrip, const std::string &whatToStrip)
    {
        std::string returnString{stringToStrip};
        if (returnString.find(whatToStrip) == std::string::npos) {
            return returnString;
        }
        size_t foundPosition{stringToStrip.find(whatToStrip)};
        if (foundPosition == 0) {
            returnString = returnString.substr(whatToStrip.length());
        } else if (foundPosition == (returnString.length() - whatToStrip.length())) {
            returnString = returnString.substr(0, foundPosition);
        } else {
            returnString = returnString.substr(0, foundPosition) + returnString.substr(foundPosition+whatToStrip.length());
        }
        return returnString;
    }

    std::string stripFromString(const std::string &stringToStrip, char whatToStrip)
    {
        return stripFromString(stringToStrip, std::string(1, whatToStrip));
    }

    std::string stripAllFromString(const std::string &stringToStrip, const std::string &whatToStrip)
    {
        std::string returnString{stringToStrip};
        if (returnString.find(whatToStrip) == std::string::npos) {
            return returnString;
        }
        while (returnString.find(whatToStrip) != std::string::npos) {
            returnString = stripFromString(returnString, whatToStrip);
        }
        return returnString;
    }

    std::string stripAllFromString(const std::string &stringToStrip, char whatToStrip)
    {
        return stripAllFromString(stringToStrip, std::string(1, whatToStrip));
    }

    std::string whitespace(unsigned int howMuch) 
    {
        std::string returnString{""};
        for (unsigned int i = 0; i < howMuch; i++) {
            returnString += " ";
        }
        return returnString;
    }

    int charToInt(char charToConvert)
    {
        switch(charToConvert) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return 0;
        }
    }

    char intToChar (int intToConvert)
    {
        switch(intToConvert) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        default: return '0';
        }
    }

    bool isDigit(char charToCheck)
    {
        return ((charToCheck == '0') || (charToCheck == '1') || (charToCheck == '2') || (charToCheck == '3') || (charToCheck == '4') || (charToCheck == '5') || (charToCheck == '6') || (charToCheck == '7') || (charToCheck == '8') || (charToCheck == '9'));
    }

    String toFixedWidth(const String &inputString, unsigned int fixedWidth)
    {
        String copyString{inputString};
        unsigned int copyLength{copyString.length()};
        if (copyLength < fixedWidth) {
            copyLength = fixedWidth - copyLength;
            while (copyLength--) {
                copyString = "0" + copyString;
            }
        }
        return copyString;
    }

    std::string toFixedWidth(const std::string &inputString, unsigned int fixedWidth)
    {
        std::string copyString{inputString};
        unsigned int copyLength{copyString.length()};
        if (copyLength < fixedWidth) {
            copyLength = (fixedWidth - copyLength);
            while (copyLength--) {
                copyString = "0" + copyString;
            }
        }
        return copyString;
    }

    std::string toFixedWidth(const char *inputString, unsigned int fixedWidth)
    {
        return toFixedWidth(static_cast<std::string>(inputString), fixedWidth);
    }

    uint32_t hexStringToUInt(const std::string &str)
    {
        std::string copyString{str};
        if (startsWith(copyString, "0x")) {
            copyString = copyString.substr(2);
        }
        unsigned int ret{0};   
        std::stringstream ss;
        ss << std::hex << copyString;
        ss >> ret;
        return ret;
    }

    uint8_t hexStringToUChar(const std::string &str)
    {
        std::string copyString{str};
        if (startsWith(copyString, "0x")) {
            copyString = copyString.substr(2);
        }
        unsigned int ret{0};   
        std::stringstream ss;
        ss << std::hex << copyString;
        ss >> ret;
        return ret;
    }

    int intExp(int base, int super)
    {
        if (super == 0) {
            return 1;
        }
        int total{base};
        for (int i = 1; i < super; i++) {
            total = total * base;
        }
        return total;
    }

    int tAbs(int lhs, int rhs)
    {
        return (lhs-rhs >= 0 ) ? (lhs-rhs) : -1*(lhs-rhs);
    }

    int tMax(int lhs, int rhs)
    {
        return (lhs >= rhs) ? lhs : rhs;
    }

    int tMin(int lhs, int rhs)
    {
        return (lhs <= rhs) ? lhs : rhs;
    }
}