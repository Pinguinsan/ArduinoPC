#ifndef ARDUINOPC_CANMESSAGE_H
#define ARDUINOPC_CANMESSAGE_H

#include "utilities.h"
#include <stdio.h>

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

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
    
    int toString(char *out, size_t maximumLength) const;
    static CanMessage parse(const char *str, char delimiter, uint8_t messageLength);
    static CanMessage parse(const char *str, const char *delimiter, uint8_t messageLength);
    uint8_t operator[](int index);
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

};

#endif //ARDUINOPC_CANMESSAGE_H
