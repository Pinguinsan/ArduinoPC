#include "../include/canmessage.h"

const int CanMessage::CAN_BYTE_WIDTH{2};
const int CanMessage::CAN_ID_WIDTH{3};

CanMessage::CanMessage(uint32_t id, uint8_t frame, uint8_t length, const CanDataPacket &dataPacket) :
    m_id{id},
    m_frame{frame},
    m_length{length},
    m_dataPacket{dataPacket}
{

}

CanMessage::CanMessage(uint32_t id, uint8_t frame, uint8_t length, unsigned char *dataPacket) :
    m_id{id},
    m_frame{frame},
    m_length{length},
    m_dataPacket{dataPacket}
{

}

CanMessage::CanMessage(uint32_t id, 
                       uint8_t frame, 
                       uint8_t length, 
                       unsigned char first, 
                       unsigned char second, 
                       unsigned char third, 
                       unsigned char fourth, 
                       unsigned char fifth, 
                       unsigned char sixth, 
                       unsigned char seventh, 
                       unsigned char eighth) :
    m_id{id},
    m_frame{frame},
    m_length{length},
    m_dataPacket{first, second, third, fourth, fifth, sixth, seventh, eighth}
{

}

CanMessage::CanMessage() :
    m_id{0},
    m_frame{0},
    m_length{0},
    m_dataPacket{CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0}}
{

}

void CanMessage::setID(uint32_t id)
{
    this->m_id = id;
}

void CanMessage::setFrame(uint8_t frame)
{
    this->m_frame = frame;
}

void CanMessage::setLength(uint8_t length)
{
    this->m_length = length;
}

void CanMessage::setDataPacket(const CanDataPacket &dataPacket)
{
    this->m_dataPacket = dataPacket;
}

bool CanMessage::setDataPacketNthByte(int index, unsigned char nth)
{   
    return this->m_dataPacket.setNthByte(index, nth);
}

uint32_t CanMessage::id() const
{
    return this->m_id;
}

uint8_t CanMessage::frame() const
{
    return this->m_frame;
}

uint8_t CanMessage::length() const
{
    return this->m_length;
}

CanDataPacket CanMessage::dataPacket() const
{
    return this->m_dataPacket;
}

int CanMessage::toString(char *out, size_t maximumLength) const
{
    using namespace FirmwareUtilities;
    if ((this->m_id == 0) &&
        (this->m_frame == 0) &&
        (this->m_length == 0) &&
        (this->m_dataPacket == CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
        return -1;
    }
    char tempHexString[SMALL_BUFFER_SIZE];
    char tempFixedWidthString[SMALL_BUFFER_SIZE];
    FirmwareUtilities::toHexString(this->m_id, tempHexString, SMALL_BUFFER_SIZE);
    FirmwareUtilities::toFixedWidth(tempHexString, tempFixedWidthString, SMALL_BUFFER_SIZE); 
    strcpy(out, "0x");
    strcat(out, tempFixedWidthString);
    strcat(out, ":");
    if (strlen(out) >= maximumLength) {
        return strlen(out);
    }
    for (int i = 0; i < (NUMBER_OF_BYTES_IN_DATA_PACKET - 1); i++) {
        memset(tempHexString, 0, strlen(tempHexString) + 1);
        memset(tempFixedWidthString, 0, strlen(tempFixedWidthString) + 1);
        FirmwareUtilities::toHexString(this->m_dataPacket.nthByte(i), tempHexString, SMALL_BUFFER_SIZE);
        FirmwareUtilities::toFixedWidth(tempHexString, tempFixedWidthString, SMALL_BUFFER_SIZE);

        strcat(out, "0x");
        strcat(out, tempFixedWidthString);
        strcat(out, ":");

        if (i++ != (NUMBER_OF_BYTES_IN_DATA_PACKET - 1)) {
            strcat(out, ":");
        }
        if (strlen(out) > maximumLength) {
            return strlen(out);
        }
    }
    return strlen(out);
}

uint32_t CanMessage::parseCanID(const char *str)
{
    return FirmwareUtilities::hexStringToUInt(str);
}

uint8_t CanMessage::parseCanByte(const char *str)
{
    return FirmwareUtilities::hexStringToUChar(str);
}
