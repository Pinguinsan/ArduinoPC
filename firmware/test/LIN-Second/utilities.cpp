#include "utilities.h"

namespace Utilities
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

    bool startsWith(const char *str, char compare)
    {
        if (strlen(str) == 0) {
            return false;
        } else {
            return (str[0] == compare);
        }
        return false;
    }

    bool endsWith(const char *str, const char *compare)
    {
        if ((!str) || (!compare)) {
            return false;
        }
        size_t stringLength = strlen(str);
        size_t endingStringLength = strlen(compare);
        if (endingStringLength > stringLength) {
            return false;
        } else {
            return strncmp(str + stringLength - endingStringLength, compare, endingStringLength) == 0;
        }
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
        for (unsigned int i = 0; i < strlen(stringToCheck); i++) {
            char tempChar{*(stringToCheck + i)};
            if ((tempChar > ASCII_WHITESPACE_MAXIMUM_VALUE) || (tempChar == '\r') || (tempChar == '\n')) {
                return false;
            }
        }
        return true;
    }

    bool isWhitespace(char charToCheck)
    {
        return ((charToCheck < ASCII_WHITESPACE_MAXIMUM_VALUE) || (charToCheck == '\r') || (charToCheck == '\n')); 
    }

    int whitespace(char *out, size_t howMuch) 
    {
        if (!out) {
            return -1;
        }
        char temp[howMuch + 1]{' '};
        strncpy(out, temp, howMuch);
        return strlen(out);
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

    int toFixedWidth(const char *inputString, char *out, size_t fixedWidth)
    {

        size_t ssize{strlen(inputString)};
        size_t bits{fixedWidth * 8};
        out = (char *) malloc(bits + 1);
        memset(out, '0', bits - ssize);
        strcpy(out + bits - ssize, inputString);
        return strlen(out);
    }

    int leftPad(const char *inputString, char *out, size_t width, char padChar)
    {
        if (strlen(inputString) >= width) {
            strncpy(out, inputString, strlen(inputString));
            out[strlen(inputString)] = '\0';
            return strlen(out);
        }
        for (int i = 0; i < width - strlen(inputString); i++) {
            out[i] = padChar;
        }
        out[width - strlen(inputString)] = '\0';
        strcat(out, inputString);
        return strlen(out);
    }

    uint32_t hexStringToUInt(const char *str)
    {
        if (!str) {
            return 0;
        }
        return (uint32_t)strtol(str, NULL, 16);
    }

    uint8_t hexStringToUChar(const char *str)
    {
        return (uint8_t)hexStringToUInt(str);
    }


    uint32_t decStringToUInt(const char *str)
    {
        if (!str) {
            return 0;
        }
        return (uint32_t)strtol(str, NULL, 10);
    }

    uint8_t decStringToUChar(const char *str)
    {
        return (uint8_t)decStringToUInt(str);
    }
    
    uint32_t stringToUInt(const char *str)
    {
        if (!str) {
            return 0;
        }
        return (uint32_t)strtol(str, NULL, 0);
    }

    uint8_t stringToUChar(const char *str)
    {
        return (uint8_t)strtol(str, NULL, 0);
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

    bool substringExists(const char *first, const char *second)
    {
        if ((!first) || (!second)) {
            return false;
        }
        return (strstr(first, second) != NULL);
    }

    bool substringExists(const char *first, char second)
    {
        char temp[2]{second, '\0'};
        return (substringExists(first, temp));
    }

    int positionOfSubstring(const char *first, const char *second)
    {
        if ((!first) || (!second)) {
            return -1;
        }
        const char *pos{strstr(first, second)};
        if (!pos) {
            return -1;
        }
        return (pos - first);
    }

    int positionOfSubstring(const char *first, char second)
    {
        char temp[2]{second, '\0'};
        return positionOfSubstring(first, temp);
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
        (void)stringLength;
        size_t numberToCopy{length};
        if (numberToCopy > maximumLength) {
            return -1;
        }
        memcpy(out, &(*(str + startPosition)), numberToCopy);
        *(out + numberToCopy) = '\0';
        return numberToCopy;
    }

    int split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength)
    {
        char *copyString = (char *)calloc(strlen(str) + 1, sizeof(char));
        strncpy(copyString, str, strlen(str) + 1);
        int outLength{0};
        size_t copyStringMaxLength{strlen(str) + 1};
        while (substringExists (copyString, delimiter)) {
            if ((unsigned)outLength >= maximumElements) {
                break;
            }
            if (positionOfSubstring(copyString, delimiter) == 0) {
                substring(copyString, strlen(delimiter), copyString, maximumLength);
            } else {
                substring(copyString, 0, positionOfSubstring(copyString, delimiter), out[outLength++], maximumLength);
                substring(copyString, positionOfSubstring(copyString, delimiter) + strlen(delimiter), copyString, copyStringMaxLength);
            }
        }
        if ((strlen(copyString) > 0) && ((unsigned)outLength < maximumElements)) {
            strncpy(out[outLength++], copyString, maximumLength);
        }
        free(copyString);
        return outLength;
    }

    int split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength)
    {
        char temp[2]{delimiter, '\0'};
        return split(str, out, temp, maximumElements, maximumLength);
    }

    bool isValidByte(char byteToCheck)
    {
        return (isprint(byteToCheck) || (byteToCheck == '\r') || (byteToCheck == '\n'));
    }
}
