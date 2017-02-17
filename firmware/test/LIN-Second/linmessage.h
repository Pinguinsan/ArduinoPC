#ifndef ARDUINOPC_LINMESSAGE_H
#define ARDUINOPC_LINMESSAGE_H

#include "utilities.h"
#include <stdio.h>

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

enum LinVersion {
    RevisionOne = 1,
    RevisionTwo = 2
};

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

    void setAddress(uint8_t address);
    void setLength(uint8_t length);
    void setVersion(LinVersion version);
    void setMessage(uint8_t *message, uint8_t length);
    void setMessage(uint8_t *message);
    bool setMessageNthByte(uint8_t index, uint8_t nth);
    
    int toString(char *out, size_t maximumLength) const;
    static uint8_t parseLinAddress(const char *str);
    static uint8_t parseLinByte(const char *str);
    static LinMessage parse(const char *str, char delimiter, uint8_t messageLength);
    static LinMessage parse(const char *str, const char *delimiter, uint8_t messageLength);
    uint8_t operator[](int index);
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
        return true;
    }

    static LinVersion toLinVersion(uint8_t version);


private:
    uint8_t m_address;
    LinVersion m_version;
    uint8_t m_length;
    uint8_t *m_message;
    
    static const uint8_t DEFAULT_MESSAGE_LENGTH;
    static const LinVersion DEFAULT_LIN_VERSION;

    void setZeroedMessage();

};

#endif //ARDUINOPC_LINMESSAGE_H
