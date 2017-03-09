#ifndef ARDUINOPC_LINMESSAGE_H
#define ARDUINOPC_LINMESSAGE_H

#include "utilities.h"
#include "heapskew.h"
#include <stdio.h>

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
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
    
    
    int toString(char *out, size_t maximumLength) const;
    static uint8_t parseLinAddress(const char *str);
    static uint8_t parseLinByte(const char *str);
    static LinMessage parse(const char *str, char delimiter, uint8_t messageLength);
    static LinMessage parse(const char *str, const char *delimiter, uint8_t messageLength);
    uint8_t operator[](int index) const;
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
};

#endif //ARDUINOPC_LINMESSAGE_H
