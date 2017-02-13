#include "linmessage.h"

const uint8_t LinDataPacket::DEFAULT_PACKET_LENGTH{8};

LinDataPacket::LinDataPacket() :
    m_length{DEFAULT_PACKET_LENGTH},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{

}

LinDataPacket::LinDataPacket(const LinDataPacket &dataPacket) :
    m_length{dataPacket.length()},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    dataPacket.toBasicArray(this->m_dataPacket, this->m_length);
}

LinDataPacket::LinDataPacket(uint8_t *packet, uint8_t length) :
    m_length{length},
    m_dataPacket{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setDataPacket(packet, this->m_length);
}

LinDataPacket& LinDataPacket::operator=(const LinDataPacket &rhs)
{
    free(this->m_dataPacket);
    this->m_length = rhs.length();
    this->m_dataPacket = static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)));
    return *this;
}

uint8_t LinDataPacket::length() const
{
    return this->m_length;
}

LinDataPacket::~LinDataPacket()
{
    free(this->m_dataPacket);
}    


void LinDataPacket::setDataPacket(uint8_t *packet, uint8_t length)
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

bool LinDataPacket::setNthByte(uint8_t index, uint8_t nth)
{
    if (index < this->m_length) {
        this->m_dataPacket[index] = nth;
        return true;
    } else {
        return false;
    }
}

uint8_t LinDataPacket::nthByte(uint8_t index) const
{
    if (index < this->m_length) {
        return this->m_dataPacket[index];
    } else {
        return 0;
    }
}

void LinDataPacket::toBasicArray(uint8_t *out, uint8_t length) const
{
    for (int i = 0; i < length; i++) {
        if (i >= this->m_length) {
            out[i] = 0;
        } else {
            out[i] = this->m_dataPacket[i];
        }
    }
}


LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length, const LinDataPacket &dataPacket) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_dataPacket{dataPacket}
{

}

LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length, uint8_t *dataPacket) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_dataPacket{dataPacket, this->m_length}
{

}

LinMessage::LinMessage(const LinMessage &other) :
    m_address{other.address()},
    m_version{other.version()},
    m_length{other.length()},
    m_dataPacket{other.dataPacket()}
{

}


LinMessage::LinMessage() :
    m_address{0},
    m_version{LinVersion::RevisionOne},
    m_length{0},
    m_dataPacket{}
{

}


void LinMessage::setAddress(uint8_t address)
{
    this->m_address = address;
}

void LinMessage::setVersion(LinVersion version)
{
    this->m_version = version;
}

void LinMessage::setDataPacket(const LinDataPacket &dataPacket)
{
    this->m_dataPacket = dataPacket;
}

bool LinMessage::setDataPacketNthByte(uint8_t index, uint8_t nth)
{   
    return this->m_dataPacket.setNthByte(index, nth);
}

uint8_t LinMessage::address() const
{
    return this->m_address;
}

LinVersion LinMessage::version() const
{
    return this->m_version;
}

uint8_t LinMessage::length() const
{
    return this->m_length;
}

LinDataPacket LinMessage::dataPacket() const
{
    return this->m_dataPacket;
}

int LinMessage::toString(char *out, size_t maximumLength) const
{
    using namespace Utilities;
    if ((this->m_address == 0) &&
        (this->m_version == LinVersion::RevisionOne) &&
        (this->m_length == 0) &&
        (this->m_dataPacket == LinDataPacket{})) {
        return -1;
    }
    char tempVersion[2];
    toDecString(this->m_version, tempVersion, 2);
    strcpy(out, tempVersion);
    strcat(out, ":");

    char tempHexString[SMALL_BUFFER_SIZE];
    char tempFixedWidthString[SMALL_BUFFER_SIZE];
    toHexString(this->m_address, tempHexString, SMALL_BUFFER_SIZE);
    leftPad(tempHexString, tempFixedWidthString, 2, '0'); 
    strcat(out, "0x");
    strcat(out, tempFixedWidthString);
    strcat(out, ":");
    if (strlen(out) >= maximumLength) {
        return strlen(out);
    }
    for (int i = 0; i < this->m_dataPacket.length(); i++) {
        tempHexString[0] = '\0';
        tempFixedWidthString[0] = '\0';
        toHexString(this->m_dataPacket.nthByte(i), tempHexString, SMALL_BUFFER_SIZE);
        leftPad(tempHexString, tempFixedWidthString, 2, '0');

        strcat(out, "0x");
        strcat(out, tempFixedWidthString);

        if (i != (this->m_dataPacket.length() - 1)) {
            strcat(out, ":");
        }
        if (strlen(out) > maximumLength) {
            return strlen(out);
        }
    }
    return strlen(out);
}

uint8_t LinMessage::parseLinAddress(const char *str)
{
    using namespace Utilities;
    return hexStringToUChar(str);
}

uint8_t LinMessage::parseLinByte(const char *str)
{
    using namespace Utilities;
    return hexStringToUChar(str);
}

LinMessage LinMessage::parse(const char *str, char delimiter, uint8_t messageLength)
{
    const char temp[2]{delimiter, '\0'};
    return LinMessage::parse(str, temp, messageLength);
}

LinMessage LinMessage::parse(const char *str, const char *delimiter, uint8_t messageLength)
{
    LinMessage returnMessage{};
    using namespace Utilities;
    int bufferSpace{messageLength + 2};
    char **result{calloc2D<char>(bufferSpace, 4)};
    int resultSize{split(str, result, delimiter, bufferSpace, 4)};
    if (resultSize < (bufferSpace)) {
        free2D(result, bufferSpace);
        return LinMessage{};
    }
    uint8_t tempVersion{hexStringToUChar(result[0])};
    if ((tempVersion != LinVersion::RevisionOne) && (tempVersion != LinVersion::RevisionTwo)) {
        free2D(result, bufferSpace);
        return returnMessage;
    } 
    returnMessage.setVersion(tempVersion == LinVersion::RevisionOne ? LinVersion::RevisionOne : LinVersion::RevisionTwo);
    returnMessage.setAddress(hexStringToUChar(result[1]));
    for (uint8_t i = 0; i < messageLength; i++) {
        returnMessage.setDataPacketNthByte(i, hexStringToUChar(result[i + 2]));
    }
    free2D(result, bufferSpace);
    return returnMessage;
}
    
