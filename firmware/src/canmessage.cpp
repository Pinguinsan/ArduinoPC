#include "../include/canmessage.h"

const uint8_t CanMessage::CAN_BYTE_WIDTH{2};
const uint8_t CanMessage::CAN_ID_WIDTH{3};
const uint8_t CanMessage::DEFAULT_MESSAGE_LENGTH{8};
const uint8_t CanMessage::DEFAULT_FRAME_TYPE{0};

CanMessage::CanMessage(uint32_t id, uint8_t frameType, uint8_t length, uint8_t *message) :
    m_id{id},
    m_frameType{frameType},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setMessage(message, length);
}

CanMessage::CanMessage(uint32_t id, uint8_t frameType) :
    m_id{id},
    m_frameType{frameType},
    m_length{DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

CanMessage::CanMessage(uint32_t id, uint8_t frameType, uint8_t length) :
    m_id{id},
    m_frameType{frameType},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

CanMessage::CanMessage(const CanMessage &other) :
    m_id{other.id()},
    m_frameType{other.frameType()},
    m_length{other.length()},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setMessage(other.message(), this->m_length);
}

CanMessage::CanMessage(uint8_t length) :
    m_id{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}


CanMessage::CanMessage() :
    m_id{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_length{CanMessage::DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

CanMessage::~CanMessage()
{
    free(this->m_message);
}

CanMessage& CanMessage::operator=(const CanMessage &rhs)
{
    this->m_length = rhs.length();
    free(this->m_message);
    this->m_message = static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)));
    this->setMessage(rhs.message(), this->m_length);
    return *this;
}

void CanMessage::setZeroedMessage()
{
    for (int i = 0; i < this->m_length; i++) {
        this->m_message[i] = 0x00;
    }
}

void CanMessage::setID(uint32_t id)
{
    this->m_id = id;
}

void CanMessage::setFrameType(uint8_t frameType)
{
    this->m_frameType = frameType;
}

void CanMessage::setMessage(uint8_t *message, uint8_t length)
{
    if (this->m_length != length) {
        free(this->m_message);
        this->m_message = static_cast<uint8_t *>(calloc(length, sizeof(uint8_t)));
    }
    for (int i = 0; i < length; i++) {
        if (!(message + i)) {
            this->m_message[i] = 0;
        } else {
            this->m_message[i] = *(message + i);
        }
    }
}

void CanMessage::setMessage(uint8_t *message)
{
    for (int i = 0; i < this->m_length; i++) {
        if (!(message + i)) {
            this->m_message[i] = 0;
        } else {
            this->m_message[i] = *(message + i);
        }
    }
}

bool CanMessage::setMessageNthByte(uint8_t index, uint8_t nth)
{
    if (index < this->m_length) {
        this->m_message[index] = nth;
        return true;
    } else {
        return false;
    }
}

uint8_t CanMessage::operator[](int index)
{
    if (index < 0) {
        return 0;
    } else {
        return this->nthByte(index);
    }
}

uint8_t CanMessage::nthByte(uint8_t index) const
{
    if (index < this->m_length) {
        return this->m_message[index];
    } else {
        return 0;
    }
}

uint8_t *CanMessage::message() const
{
    return this->m_message;
}

void CanMessage::setLength(uint8_t length)
{
    if (this->m_length == length) {
        return;
    }
    uint8_t oldLength{this->m_length};
    uint8_t *tempStorage{static_cast<uint8_t *>(calloc(oldLength, sizeof(uint8_t)))};
    for (int i = 0; i < oldLength; i++) {
        tempStorage[i] = this->m_message[i];
    }
    free(this->m_message);
    this->m_message = static_cast<uint8_t *>(calloc(length, sizeof(uint8_t)));
    this->m_length = length;
    uint8_t numberToCopy{(this->m_length < oldLength) ? oldLength : this->m_length};
    for (int i = 0; i < numberToCopy; i++) {
        this->m_message[i] = tempStorage[i]; 
    }
    free(tempStorage);
}

uint32_t CanMessage::id() const
{
    return this->m_id;
}

uint8_t CanMessage::frameType() const
{
    return this->m_frameType;
}

uint8_t CanMessage::length() const
{
    return this->m_length;
}

int CanMessage::toString(char *out, size_t maximumLength) const
{
    using namespace Utilities;
    if ((this->m_id == 0) &&
        (this->m_frameType == DEFAULT_FRAME_TYPE) &&
        (this->m_length == 0)) {
        strncpy(out, "", maximumLength);
        return -1;
    }
    char tempFrameType[2];
    toDecString(this->m_frameType, tempFrameType, 2);
    strcpy(out, tempFrameType);
    strncat(out, ":", maximumLength);

    char tempHexString[SMALL_BUFFER_SIZE];
    char tempFixedWidthString[SMALL_BUFFER_SIZE];
    toHexString(this->m_id, tempHexString, SMALL_BUFFER_SIZE);
    leftPad(tempHexString, tempFixedWidthString, 2, '0'); 
    strncat(out, "0x", maximumLength);
    strncat(out, tempFixedWidthString, maximumLength);
    strncat(out, ":", maximumLength);
    if (strlen(out) >= maximumLength) {
        return strlen(out);
    }
    for (int i = 0; i < this->m_length; i++) {
        tempHexString[0] = '\0';
        tempFixedWidthString[0] = '\0';
        toHexString(this->m_message[i], tempHexString, SMALL_BUFFER_SIZE);
        leftPad(tempHexString, tempFixedWidthString, 2, '0');

        strncat(out, "0x", maximumLength);
        strncat(out, tempFixedWidthString, maximumLength);

        if (i != (this->m_length - 1)) {
            strncat(out, ":", maximumLength);
        }
        if (strlen(out) > maximumLength) {
            return strlen(out);
        }
    }
    return strlen(out);
}

CanMessage CanMessage::parse(const char *str, char delimiter, uint8_t messageLength)
{
    const char temp[2]{delimiter, '\0'};
    return CanMessage::parse(str, temp, messageLength);
}

CanMessage CanMessage::parse(const char *str, const char *delimiter, uint8_t messageLength)
{
    CanMessage returnMessage{};
    using namespace Utilities;
    int bufferSpace{messageLength + 2};
    char **result{calloc2D<char>(bufferSpace, 10)};
    int resultSize{split(str, result, delimiter, bufferSpace, 10)};
    if (resultSize < (bufferSpace)) {
        free2D(result, bufferSpace);
        return CanMessage{};
    }
    uint8_t tempFrameType{stringToUChar(result[0])};
    if (tempFrameType > 1) {
        free2D(result, bufferSpace);
        return returnMessage;
    } 
    returnMessage.setFrameType(tempFrameType);
    returnMessage.setID(stringToUInt(result[1]));
    for (uint8_t i = 0; i < messageLength; i++) {
        returnMessage.setMessageNthByte(i, stringToUChar(result[i + 2]));
    }
    free2D(result, bufferSpace);
    return returnMessage;
}
    
