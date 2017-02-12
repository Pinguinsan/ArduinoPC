#ifndef ARDUINOPC_CANMESSAGE_H
#define ARDUINOPC_CANMESSAGE_H

#include "utilities.h"


#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif


class CanDataPacket
{
public:
    CanDataPacket();
    CanDataPacket(const CanDataPacket &dataPacket);
    CanDataPacket(uint8_t *packet, uint8_t length);
    CanDataPacket& operator=(const CanDataPacket &rhs);
    ~CanDataPacket();

    void setDataPacket(uint8_t *packet, uint8_t length);
    bool setNthByte(uint8_t index, uint8_t nth);
    uint8_t nthByte(uint8_t index) const;
    void toBasicArray(uint8_t *out, uint8_t length) const;

    uint8_t length() const;
    friend bool operator==(const CanDataPacket &lhs, const CanDataPacket &rhs) 
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


class CanMessage
{
public:
    CanMessage(uint32_t id, uint8_t frame, uint8_t length, const CanDataPacket &dataPacket);
    CanMessage(uint32_t id, uint8_t frame, uint8_t length, uint8_t *dataPacket);
    CanMessage();
    uint32_t id() const;
    uint8_t frame() const;
    uint8_t length() const;
    CanDataPacket dataPacket() const;
    bool setDataPacketNthByte(uint8_t index, uint8_t nth);

    void setID(uint32_t id);
    void setFrame(uint8_t frame);
    void setDataPacket(const CanDataPacket &dataPacket);
    int toString(char *out, size_t maximumLength) const;
    static uint32_t parseCanID(const char *str);
    static uint8_t parseCanByte(const char *str);
    static CanMessage parseCanMessage(const char *str);
    static const int CAN_BYTE_WIDTH;
    static const int CAN_ID_WIDTH;  

private:
    uint32_t m_id;
    uint8_t m_frame;
    uint8_t m_length;
    CanDataPacket m_dataPacket;
};

#endif //ARDUINOPC_CANMESSAGE_H
