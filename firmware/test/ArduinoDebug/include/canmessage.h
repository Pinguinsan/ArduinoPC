#ifndef ARDUINOPC_CANMESSAGE_H
#define ARDUINOPC_CANMESSAGE_H

#include "candatapacket.h"
#include "firmwareutilities.h"

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

#ifndef NUMBER_OF_BYTES_IN_DATA_PACKET
    #define NUMBER_OF_BYTES_IN_DATA_PACKET 8
#endif

class CanMessage
{
public:
    CanMessage(uint32_t id, uint8_t frame, uint8_t length, const CanDataPacket &dataPacket);
    CanMessage(uint32_t id, uint8_t frame, uint8_t length, unsigned char *dataPacket);
    CanMessage(uint32_t id, 
               uint8_t frame, 
               uint8_t length, 
               unsigned char first, 
               unsigned char second, 
               unsigned char third, 
               unsigned char fourth, 
               unsigned char fifth, 
               unsigned char sixth, 
               unsigned char seventh, 
               unsigned char eighth);
    CanMessage();
    uint32_t id() const;
    uint8_t frame() const;
    uint8_t length() const;
    CanDataPacket dataPacket() const;
    bool setDataPacketNthByte(int index, unsigned char nth);

    void setID(uint32_t id);
    void setFrame(uint8_t frame);
    void setLength(uint8_t length);
    void setDataPacket(const CanDataPacket &dataPacket);
    int toString(char *out, size_t maximumLength) const;
    static uint32_t parseCanID(const char *str);
    static uint8_t parseCanByte(const char *str);  
    static const int CAN_BYTE_WIDTH;
    static const int CAN_ID_WIDTH;  

private:
    uint32_t m_id;
    uint8_t m_frame;
    uint8_t m_length;
    CanDataPacket m_dataPacket;
};

#endif //ARDUINOPC_CANMESSAGE_H
