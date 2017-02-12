#ifndef ARDUINOPC_LinMessage_H
#define ARDUINOPC_LinMessage_H

#include "utilities.h"


#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif


class LinDataPacket
{
public:
    LinDataPacket();
    LinDataPacket(const LinDataPacket &dataPacket);
    LinDataPacket(uint8_t *packet, uint8_t length);
    LinDataPacket& operator=(const LinDataPacket &rhs);
    ~LinDataPacket();

    void setDataPacket(uint8_t *packet, uint8_t length);
    bool setNthByte(uint8_t index, uint8_t nth);
    uint8_t nthByte(uint8_t index) const;
    void toBasicArray(uint8_t *out, uint8_t length) const;

    uint8_t length() const;
    friend bool operator==(const LinDataPacket &lhs, const LinDataPacket &rhs) 
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

private:
    uint8_t m_length;
    uint8_t *m_dataPacket;

    static const uint8_t DEFAULT_PACKET_LENGTH;
};

enum LinVersion {
    RevisionOne = 1,
    RevisionTwo = 2
};

class LinMessage
{
public:
    LinMessage(uint8_t address, LinVersion version, uint8_t length, const LinDataPacket &dataPacket);
    LinMessage(uint8_t address, LinVersion version, uint8_t length, uint8_t *dataPacket);
    LinMessage(const LinMessage &other);
    LinMessage();
    uint8_t address() const;
    LinVersion version() const;
    uint8_t length() const;
    LinDataPacket dataPacket() const;
    bool setDataPacketNthByte(uint8_t index, uint8_t nth);

    void setAddress(uint8_t address);
    void setVersion(LinVersion version);
    void setDataPacket(const LinDataPacket &dataPacket);
    int toString(char *out, size_t maximumLength) const;
    static uint8_t parseLinAddress(const char *str);
    static uint8_t parseLinByte(const char *str);
    static LinMessage parseLinMessage(const char *str, char delimiter, uint8_t messageLength);

private:
    uint8_t m_address;
    LinVersion m_version;
    uint8_t m_length;
    LinDataPacket m_dataPacket;
};

#endif //ARDUINOPC_LinMessage_H
