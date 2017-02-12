#include "canmessage.h"

const int CanMessage::CAN_BYTE_WIDTH{2};
const int CanMessage::CAN_ID_WIDTH{3};
const uint8_t CanDataPacket::DEFAULT_PACKET_LENGTH{8};

CanDataPacket::CanDataPacket() :
    m_length{DEFAULT_PACKET_LENGTH},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{

}

CanDataPacket::CanDataPacket(const CanDataPacket &dataPacket) :
    m_length{dataPacket.length()},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    dataPacket.toBasicArray(this->m_dataPacket, this->m_length);
}

CanDataPacket::CanDataPacket(uint8_t *packet, uint8_t length) :
    m_length{length},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setDataPacket(packet, this->m_length);
}

CanDataPacket& CanDataPacket::operator=(const CanDataPacket &rhs)
{
    free(this->m_dataPacket);
    this->m_length = rhs.length();
    this->m_dataPacket = static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)));
    return *this;
}

uint8_t CanDataPacket::length() const
{
    return this->m_length;
}

CanDataPacket::~CanDataPacket()
{
    free(this->m_dataPacket);
}    


void CanDataPacket::setDataPacket(uint8_t *packet, uint8_t length)
{
    for (int i = 0; i < (length - 1); i++) {
        if (i >= this->m_length) {
            break;
        }
        if (!(packet + i)) {
            this->m_dataPacket[i] = 0;
        } else {
            this->m_dataPacket[i] = *(packet + i);
        }
    }
}

bool CanDataPacket::setNthByte(uint8_t index, uint8_t nth)
{
    if (index < this->m_length) {
        this->m_dataPacket[index] = nth;
        return true;
    } else {
        return false;
    }
}

uint8_t CanDataPacket::nthByte(uint8_t index) const
{
    if (index < this->m_length) {
        return this->m_dataPacket[index];
    } else {
        return 0;
    }
}

void CanDataPacket::toBasicArray(uint8_t *out, uint8_t length) const
{
    for (int i = 0; i < length; i++) {
        if (i >= this->m_length) {
            out[i] = 0;
        } else {
            out[i] = this->m_dataPacket[i];
        }
    }
}


CanMessage::CanMessage(uint32_t id, uint8_t frame, uint8_t length, const CanDataPacket &dataPacket) :
    m_id{id},
    m_frame{frame},
    m_length{length},
    m_dataPacket{dataPacket}
{

}

CanMessage::CanMessage(uint32_t id, uint8_t frame, uint8_t length, uint8_t *dataPacket) :
    m_id{id},
    m_frame{frame},
    m_length{length},
    m_dataPacket{dataPacket, this->m_length}
{

}


CanMessage::CanMessage() :
    m_id{0},
    m_frame{0},
    m_length{0},
    m_dataPacket{}
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

void CanMessage::setDataPacket(const CanDataPacket &dataPacket)
{
    this->m_dataPacket = dataPacket;
}

bool CanMessage::setDataPacketNthByte(uint8_t index, uint8_t nth)
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
    using namespace Utilities;
    if ((this->m_id == 0) &&
        (this->m_frame == 0) &&
        (this->m_length == 0) &&
        (this->m_dataPacket == CanDataPacket{})) {
        return -1;
    }
    char tempHexString[SMALL_BUFFER_SIZE];
    char tempFixedWidthString[SMALL_BUFFER_SIZE];
    toHexString(this->m_id, tempHexString, SMALL_BUFFER_SIZE);
    toFixedWidth(tempHexString, tempFixedWidthString, SMALL_BUFFER_SIZE); 
    strcpy(out, "0x");
    strcat(out, tempFixedWidthString);
    strcat(out, ":");
    if (strlen(out) >= maximumLength) {
        return strlen(out);
    }
    for (int i = 0; i < (this->m_dataPacket.length() - 1); i++) {
        memset(tempHexString, 0, strlen(tempHexString) + 1);
        memset(tempFixedWidthString, 0, strlen(tempFixedWidthString) + 1);
        toHexString(this->m_dataPacket.nthByte(i), tempHexString, SMALL_BUFFER_SIZE);
        toFixedWidth(tempHexString, tempFixedWidthString, SMALL_BUFFER_SIZE);

        strcat(out, "0x");
        strcat(out, tempFixedWidthString);
        strcat(out, ":");

        if (i++ != (this->m_dataPacket.length() - 1)) {
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
    using namespace Utilities;
    return hexStringToUInt(str);
}

uint8_t CanMessage::parseCanByte(const char *str)
{
    using namespace Utilities;
    return hexStringToUChar(str);
}
