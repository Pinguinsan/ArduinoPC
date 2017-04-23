#ifndef ARDUINOPC_LINMESSAGE_H
#define ARDUINOPC_LINMESSAGE_H

#include "heapskew.h"
#include <cstdio>

#ifndef SMALL_BUFFER_SIZE
#    define SMALL_BUFFER_SIZE 255
#endif

#ifndef LIN_MESSAGE_PARSE_BUFFER_SPACE
#    define LIN_MESSAGE_PARSE_BUFFER_SPACE 15
#endif

enum LinVersion {
    RevisionOne = 1,
    RevisionTwo = 2
};

enum FrameType {
    ReadFrame = 0,
    WriteFrame = 1,
};

class LinMessage;

using heap_skew_element_t = HeapSkew<LinMessage>::HeapSkewElement;
using callback_ptr_t = uint16_t (*)(LinMessage* me);

class LinMessage
{

public:
    LinMessage(uint8_t address, LinVersion version, uint8_t length, uint8_t *message);
    LinMessage(uint8_t address, LinVersion version);
    LinMessage(uint8_t address, uint8_t version, uint8_t length, uint8_t *message);
    LinMessage(uint8_t address, uint8_t version);
    LinMessage(uint8_t address, LinVersion version, uint8_t length);
    LinMessage(uint8_t address, uint8_t version, uint8_t length);
    LinMessage(const LinMessage &other);
    LinMessage(uint8_t length);
    LinMessage();
    ~LinMessage();

    uint8_t nthByte(uint8_t index) const;
    uint8_t address() const;
    LinVersion version() const;
    uint8_t length() const;
    uint8_t *message() const;
    FrameType frameType() const;
    unsigned long triggerTime() const;
    callback_ptr_t callback() const;
    heap_skew_element_t skewChildren() const;

    void setCallback(callback_ptr_t callback);
    void setFrameType(uint8_t frameType);
    void setFrameType(FrameType frameType); 
    void setTriggerTime(unsigned long triggerTime);
    void setAddress(uint8_t address);
    void setLength(uint8_t length);
    void setVersion(LinVersion version);
    void setVersion(uint8_t version);
    void setMessage(uint8_t *message, uint8_t length);
    void setMessage(uint8_t *message);
    bool setMessageNthByte(uint8_t index, uint8_t nth);
    
    size_t toString(char *out, size_t maximumLength) const;
    static LinMessage parse(const char *str, char delimiter);
    static LinMessage parse(const char *str, const char *delimiter);
    uint8_t operator[](uint8_t index) const;
    LinMessage &operator=(const LinMessage &rhs);
    friend bool operator==(const LinMessage &lhs, const LinMessage &rhs) 
    {
        if (lhs.length() != rhs.length()) {
            return false;
        }
        for (int i = 0; i < (lhs.length() - 1); i++) {
            if (lhs.nthByte(i) != rhs.nthByte(i)) {
                return false;
            }
        }
        return lhs.triggerTime() == rhs.triggerTime();
    }

    static LinVersion toLinVersion(uint8_t version);
    static FrameType toFrameType(uint8_t frameType);

    // So that the skew heap orders the elements by when they should be executed
    friend bool operator>(LinMessage &lhs, LinMessage &rhs)
    {
        return lhs.triggerTime() > rhs.triggerTime();
    }

    friend bool operator<(LinMessage &lhs, LinMessage &rhs)
    {
        return lhs.triggerTime() < rhs.triggerTime();
    }


private:
    uint8_t m_address;
    LinVersion m_version;
    uint8_t m_length;
    uint8_t *m_message;
    unsigned long m_triggerTime;
    FrameType m_frameType;
    heap_skew_element_t m_skewChildren;
    callback_ptr_t m_callback;
    
    static const uint8_t DEFAULT_MESSAGE_LENGTH;
    static const LinVersion DEFAULT_LIN_VERSION;
    static const FrameType DEFAULT_FRAME_TYPE;

    void setZeroedMessage();
    void zeroOutSkewChildren();

    template <typename InputType>
    size_t toFixedWidthHex(char *out, size_t bufferLength, size_t fixedWidth, InputType input, bool includeZeroX = true)
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

    size_t positionOfSubstring(const char *first, const char *second);
    size_t positionOfSubstring(const char *first, char second);
    size_t substring(const char *str, size_t startPosition, char *out, size_t maximumLength);
    size_t substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength);
    size_t split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength);
    size_t split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength);

};

#endif //ARDUINOPC_LINMESSAGE_H
