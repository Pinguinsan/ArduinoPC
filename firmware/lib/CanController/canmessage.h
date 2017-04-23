#ifndef ARDUINOPC_CANMESSAGE_H
#define ARDUINOPC_CANMESSAGE_H

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#ifndef SMALL_BUFFER_SIZE
#    define SMALL_BUFFER_SIZE 255
#endif

#ifndef CAN_MESSAGE_PARSE_BUFFER_SPACE
#    define CAN_MESSAGE_PARSE_BUFFER_SPACE 15
#endif

enum FrameType {
    Normal = 0x00,
    Extended = 0x01
};

class CanMessage
{
public:
    CanMessage(uint32_t id, uint8_t frameType, uint8_t length, uint8_t *message);
    CanMessage(uint32_t id, uint8_t frameType);
    CanMessage(uint32_t id, uint8_t frameType, uint8_t length);
    CanMessage(const CanMessage &other);
    CanMessage(uint8_t length);
    CanMessage();
    ~CanMessage();
    
    uint8_t nthByte(uint8_t index) const;
    uint32_t id() const;
    uint8_t frameType() const;
    uint8_t length() const;
    uint8_t *message() const;

    void setID(uint32_t id);
    void setLength(uint8_t length);
    void setFrameType(uint8_t frameType);
    void setMessage(uint8_t *message, uint8_t length);
    void setMessage(uint8_t *message);
    bool setMessageNthByte(uint8_t index, uint8_t nth);
    
    size_t toString(char *out, size_t maximumLength) const;
    static CanMessage parse(const char *str, char delimiter);
    static CanMessage parse(const char *str, const char *delimiter);
    uint8_t operator[](uint8_t index);
    CanMessage &operator=(const CanMessage &rhs);
    friend bool operator==(const CanMessage &lhs, const CanMessage &rhs) 
    {
        if (lhs.length() != rhs.length()) {
            return false;
        }
        for (int i = 0; i < (lhs.length() - 1); i++) {
            if (lhs.nthByte(i) != rhs.nthByte(i)) {
                return false;
            }
        }
        return true;
    }

    static const uint8_t DEFAULT_MESSAGE_LENGTH;
    static const uint8_t CAN_BYTE_WIDTH;
    static const uint8_t CAN_ID_WIDTH;  
    static const uint8_t DEFAULT_FRAME_TYPE;

private:
    uint32_t m_id;
    uint8_t m_frameType;
    uint8_t m_length;
    uint8_t *m_message;
    
    void setZeroedMessage();

    template <typename InputType>
    static size_t toFixedWidthHex(char *out, size_t bufferLength, size_t fixedWidth, InputType input, bool includeZeroX = true)
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

    template <typename Ptr>
    static void free2D(Ptr **out, size_t elements)
    {
        for (size_t i = 0; i < elements; i++) {
            free(out[i]);
        }
        free(out);
    }

    template <typename Ptr>
    static Ptr **calloc2D(size_t elements, size_t maximumLength)
    {
        Ptr **out = (Ptr **)calloc(elements, sizeof(Ptr *) * maximumLength);
        for (size_t i = 0; i < elements; i++) {
            out[i] = (Ptr *)calloc(maximumLength, sizeof(Ptr));
        }
        return out;
    }

    static bool substringExists(const char *first, const char *second);
    static bool substringExists(const char *first, char second);
    static size_t positionOfSubstring(const char *first, const char *second);
    static size_t positionOfSubstring(const char *first, char second);
    static size_t substring(const char *str, size_t startPosition, char *out, size_t maximumLength);
    static size_t substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength);
    static size_t split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength);
    static size_t split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength);
};


#endif //ARDUINOPC_CANMESSAGE_H
