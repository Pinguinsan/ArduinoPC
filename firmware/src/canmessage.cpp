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

std::string CanMessage::toString() const
{
    using namespace FirmwareUtilities;
    if ((this->m_id == 0) &&
        (this->m_frame == 0) &&
        (this->m_length == 0) &&
        (this->m_dataPacket == CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
        return "";
    }
    std::string returnString{"0x" + toFixedWidth(toHexString(this->m_id), CAN_ID_WIDTH) + ":"};
    unsigned int i{0};
    for (auto &it : this->m_dataPacket.dataPacket()) {
        returnString += "0x" + toFixedWidth(toHexString(it), CAN_BYTE_WIDTH);
        if (i++ != (this->m_dataPacket.dataPacket().size()-1)) {
            returnString += ":";
        }
    }
    return returnString;
}

uint32_t CanMessage::parseCanID(const std::string &str)
{
    using namespace FirmwareUtilities;
    return hexStringToUInt(str);
}

uint8_t CanMessage::parseCanByte(const std::string &str)
{
    using namespace FirmwareUtilities;
    return hexStringToUChar(str);
}
