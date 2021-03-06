#include "canmessage.h"

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

uint8_t CanMessage::operator[](uint8_t index)
{
    return this->nthByte(index);
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
    if (maximumLength == 0) {
        return -1;
    }
    char tempHexString[SMALL_BUFFER_SIZE];
    toFixedWidthHex(tempHexString, SMALL_BUFFER_SIZE, this->m_id, 3, true);
    strncat(out, tempHexString, maximumLength);
    strncat(out, " : ", maximumLength);
    for (int i = 0; i < this->m_length; i++) {
        memset(tempHexString, '\0', SMALL_BUFFER_SIZE);
        toFixedWidthHex(tempHexString, SMALL_BUFFER_SIZE, this->m_message[i], 2, true);
        strncat(out, tempHexString, maximumLength);
        if (i != (this->m_length - 1)) {
            strncat(out, " : ", maximumLength);
        }
    }
    return strlen(out);
}

CanMessage CanMessage::parse(const char *str, char delimiter)
{
    const char temp[2]{delimiter, '\0'};
    return CanMessage::parse(str, temp);
}

CanMessage CanMessage::parse(const char *str, const char *delimiter)
{
    char **result{calloc2D<char>(CAN_MESSAGE_PARSE_BUFFER_SPACE, 10)};
    size_t resultSize{split(str, result, delimiter, CAN_MESSAGE_PARSE_BUFFER_SPACE, 10)};
    if (resultSize < 1) {
        free2D(result, CAN_MESSAGE_PARSE_BUFFER_SPACE);
        return CanMessage{};
    }
    CanMessage returnMessage{static_cast<uint32_t>(strtol(result[0], nullptr, 0)),
                             CanMessage::DEFAULT_FRAME_TYPE, 
                             static_cast<uint8_t>(resultSize - 1)};
    for (uint8_t i = 0; i < returnMessage.length(); i++) {
        returnMessage.setMessageNthByte(i, static_cast<uint8_t>(strtol(result[i + 1], nullptr, 0)));
    }
    free2D(result, CAN_MESSAGE_PARSE_BUFFER_SPACE);
    return returnMessage;
}

bool CanMessage::substringExists(const char *first, const char *second)
{
    if ((!first) || (!second)) {
        return false;
    }
    return (strstr(first, second) != nullptr);
}

bool CanMessage::substringExists(const char *first, char second)
{
    char temp[2]{second, '\0'};
    return (substringExists(first, temp));
}

int CanMessage::positionOfSubstring(const char *first, const char *second)
{
    if ((!first) || (!second)) {
        return -1;
    }
    const char *pos{strstr(first, second)};
    if (!pos) {
        return -1;
    }
    return (pos - first);
}

int CanMessage::positionOfSubstring(const char *first, char second)
{
    char temp[2]{second, '\0'};
    return positionOfSubstring(first, temp);
}

int CanMessage::substring(const char *str, size_t startPosition, char *out, size_t maximumLength)
{
    if ((!str) || (!out)) {
        return -1;
    }
    size_t stringLength{strlen(str)};
    size_t numberToCopy{stringLength - startPosition};
    if (numberToCopy > maximumLength) {
        return -1;
    }
    memcpy(out, &(*(str + startPosition)), numberToCopy);
    *(out + numberToCopy) = '\0';
    return numberToCopy;
}

int CanMessage::substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength)
{
    if ((!str) || (!out)) {
        return -1;
    }
    size_t stringLength{strlen(str)};
    (void)stringLength;
    size_t numberToCopy{length};
    if (numberToCopy > maximumLength) {
        return -1;
    }
    memcpy(out, &(*(str + startPosition)), numberToCopy);
    *(out + numberToCopy) = '\0';
    return numberToCopy;
}

size_t CanMessage::split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength)
{
    char *copyString = static_cast<char *>(calloc(strlen(str) + 1, sizeof(char)));
    strncpy(copyString, str, strlen(str) + 1);
    size_t outLength{0};
    size_t copyStringMaxLength{strlen(str) + 1};
    while (substringExists(copyString, delimiter)) {
        if (outLength >= maximumElements) {
            break;
        }
        if (positionOfSubstring(copyString, delimiter) == 0) {
            substring(copyString, strlen(delimiter), copyString, maximumLength);
        } else {
            substring(copyString, 0, positionOfSubstring(copyString, delimiter), out[outLength++], maximumLength);
            substring(copyString, positionOfSubstring(copyString, delimiter) + strlen(delimiter), copyString, copyStringMaxLength);
        }
    }
    if ((strlen(copyString) > 0) && (outLength < maximumElements)) {
        strncpy(out[outLength++], copyString, maximumLength);
    }
    free(copyString);
    return outLength;
}

size_t CanMessage::split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength)
{
    char temp[2]{delimiter, '\0'};
    return split(str, out, temp, maximumElements, maximumLength);
}

    
