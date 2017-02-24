#include <iostream>
#include <cstring>
#include <limits>

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

bool isAllZeroes(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] != '0') {
            return false;
        }
    }
    return true;
}

bool isAllZeroesOrDecimal(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++) {
        if ((str[i] != '0') && (str[i] != '.')) {
            return false;
        }
    }
    return true;
}

template<typename T>
struct is_c_str : std::integral_constant<bool, false> {};

template<>
struct is_c_str<char*> : std::integral_constant<bool, true> {};

template<>
struct is_c_str<char[]> : std::integral_constant<bool, true> {};

template<>
struct is_c_str<const char*> : std::integral_constant<bool, true> {};

template<>
struct is_c_str<const char[]> : std::integral_constant<bool, true> {};

void genericSplitCast(const char *str, const char *delimiter)
{
    return;
}

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
    if (std::is_integral<First>::value) {
        long long tempValue{strtoll(temp, nullptr, 0)};
        if (isAllZeroes(temp)) {
            *first = 0;
        }
        if ((tempValue == 0) || (tempValue > std::numeric_limits<First>::max())) {
            first = nullptr;
        } else {
            *first = static_cast<First>(tempValue);
        }
    } else if (std::is_floating_point<First>::value) {
        long double tempValue{strtold(temp, nullptr)};
        if (isAllZeroesOrDecimal(temp)) {
            *first = 0;
        }
        if ((tempValue == 0) || (tempValue > std::numeric_limits<First>::max())) {
            first = nullptr;
        } else {
            *first = static_cast<First>(tempValue);
        }
    } else if (is_c_str<First>::value) {
        std::cout << "First is c string" << std::endl;
        for (size_t i = 0; i < strlen(temp); i++) {
            first[i] = temp[i];
        }
        first[strlen(temp)] = '\0';
    } else {
        first = nullptr;
    }
    genericSplitCast(passOn, delimiter, arguments...);
    delete passOn;
    delete temp;
}

int main()
{
    const char *temp{"1,15,4.0,2,5"};
    int *first = new int{0};
    int *second = new int{0};
    double *third = new double{0.00};
    int *fourth = new int{0};
    int *fifth = new int{0};
    char *sixth = new char[10];

    genericSplitCast(temp, ",", first, second, third, fourth, fifth);
    std::cout << "temp = " << temp << std::endl;
    std::cout << "first (int) parsed to ";
    if (!first) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *first << std::endl;
    }

    std::cout << "second (int) parsed to ";
    if (!second) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *second << std::endl;
    }

    std::cout << "third (double) parsed to ";
    if (!third) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *third << std::endl;
    }

    std::cout << "fourth (int) parsed to ";
    if (!fourth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *fourth<< std::endl;
    }

    std::cout << "fifth (int) parsed to ";
    if (!fifth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *fifth << std::endl;
    }

    std::cout << "sixth (c string) parsed to ";
    if (!sixth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *sixth << std::endl;
    }
}