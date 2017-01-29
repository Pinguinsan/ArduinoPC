#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>

bool substringExists(const char *first, const char *second)
{
    if ((!first) || (!second)) {
        return false;
    }
    return (strstr(first, second) != NULL);
}

bool substringExists(const char *first, char second)
{
    char temp[2];
    temp[0] = second;
    temp[1] = '\0';
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
    char temp[2];
    temp[0] = second;
    temp[1] = '\0';
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

#define SMALL_BUFFER_SIZE 255
#define SERIAL_PORT_BUFFER_MAX 1024
#define MAXIMUM_LINE_ENDING_STRING 5
#define MAXIMUM_STRING_COUNT 10

int main()
{
    const char *lineEnding{"\r"};
    size_t m_stringQueueIndex{0};
    char *m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
    strncpy(m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);

    char *m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
    char *m_stringQueue[MAXIMUM_STRING_COUNT];
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
    }

    strncpy(m_stringBuilderQueue, "{ardtype}\r", SERIAL_PORT_BUFFER_MAX);
    while (substringExists(m_stringBuilderQueue, m_lineEnding)) {
        char tempString[SMALL_BUFFER_SIZE];
        (void)substring(m_stringBuilderQueue,
                        0,
                        positionOfSubstring(m_stringBuilderQueue, m_lineEnding),
                        tempString,
                        SMALL_BUFFER_SIZE);
        std::cout << "tempString = " << tempString << std::endl;
        strncpy(m_stringQueue[m_stringQueueIndex++], tempString, SMALL_BUFFER_SIZE);
        (void)substring(m_stringBuilderQueue,
                        positionOfSubstring(m_stringBuilderQueue, m_lineEnding) + 1,
                        m_stringBuilderQueue,
                        strlen(m_stringBuilderQueue) + 1);
    }
    std::cout << "Exiting addToStringBuilderQueue()" << std::endl;
    std::cout << "m_stringQueue[0] = " << m_stringQueue[0] << std::endl;
    return 0;
}
