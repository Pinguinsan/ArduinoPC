#include "../include/firmwareutilities.h"

volatile unsigned long timer0_millis = 0;

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

    bool startsWith(const char *str, const char *compare)
    {
        return (strncmp(str, compare, strlen(compare)) == 0);
    }

    bool endsWith(const char *str, const char *compare)
    {
        if ((!str) || (!compare)) {
            return false;
        }
        size_t stringLength = strlen(str);
        size_t endingStringLength = strlen(compare);
        if (endingStringLength > stringLength)
            return 0;
        return strncmp(str + stringLength - endingStringLength, compare, endingStringLength) == 0;
    }

    bool endsWith(const char *str, char compare)
    {
        if (!str) {
            return false;
        }
        size_t endOfStringPosition{strlen(str)-1};
        return (*(str + endOfStringPosition) == compare);
    }

    bool isWhitespace(const char *stringToCheck)
    {
        if (!stringToCheck) {
            return false;
        }
        for (int i = 0; i < strlen(stringToCheck); i++) {
            char tempChar{*(stringToCheck + i)};
            if ((tempChar > ASCII_WHITESPACE_MAXIMUM_VALUE) || (tempChar == '\r') || (tempChar == '\n')) {
                return false;
            }
        }
        return true;
    }

    bool isWhitespace(char charToCheck)
    {
        return ((charToCheck < ASCII_WHITESPACE_MAXIMUM_VALUE) || (charToCheck == '\r') || (charToCheck == '\n')) 
    }

    int whitespace(size_t howMuch, char *out) 
    {
        if (!out) {
            return;
        }
        char temp[howMuch + 1]{' '};
        strncopy(out, temp, howMuch);
        return 0;
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

    int toFixedWidth(const char *inputString, unsigned int fixedWidth, char *out)
    {
        size_t ssize = strlen(inputString);
        size_t bits = fixedWidth * 8;
        out = (const char *) malloc(bits + 1);
        assert(ssize < bits);
        memset(out, '0', bits - ssize);
        strcpy(out + bits - ssize, string);
        strlen(out);
    }

    uint32_t hexStringToUInt(const char *str)
    {
        if (!str) {
            return 0;
        };
        int base{16};
        if (startsWith(str, "0x")) {
            base = 0;
        }
        return (int)strtol(str, NULL, base);
    }

    uint8_t hexStringToUChar(const char *str)
    {
        return (uint8_t)hexStringToUInt(str);
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
    
    unsigned long tMillis()
    {   
        unsigned long m;
        uint8_t oldSREG = SREG;
        // disable interrupts while we read timer0_millis or we might get an
        // inconsistent value (e.g. in the middle of a write to timer0_millis)
        cli();
        m = timer0_millis;
        SREG = oldSREG;
        return m;
    }

    bool substringExists(const char *first, const char *second)
    {
        if ((!first) || (!second)) {
            return false;
        }
        return (strstr(first, second) != NULL);
    }

    int positionOfSubtring(const char *first, const char *second)
    {
        if ((!first) || (!second)) {
            return -1;
        }
        char *pos{strstr(first, second)};
        if (!pos) {
            return -1;
        }
        return (pos - first);
    }

    int substring(const char *str, size_t startPosition, char *out, size_t maximumLength)
    {
        if ((!str) || (!out)) {
            return -1;
        }
        size_t stringLength{strlen(str)};
        size_t numberToCopy{stringLength - startPosition};
        if (numberToCopy > maximumLength) {
            return -1;
        }
        memcpy(out, &(*(str + startPosition)), numberToCopy);
        *(out + numberToCopy) = '\0';
        return numberToCopy;
    }

    int substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength)
    {
        if ((!str) || (!out)) {
            return -1;
        }
        size_t stringLength{strlen(str)};
        size_t numberToCopy{length};
        if (numberToCopy > maximumLength) {
            return -1;
        }
        memcpy(out, &(*(str + startPosition)), numberToCopy);
        *(out + numberToCopy) = '\0';
        return numberToCopy;
    }
/*
    char subbuff[5];
    memcpy( subbuff, &buff[10], 4 );
    subbuff[4] = '\0';
*/
    /*
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
*/
}