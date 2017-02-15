#include "../include/linmessage.h"

const uint8_t LinMessage::DEFAULT_MESSAGE_LENGTH{8};
const LinVersion LinMessage::DEFAULT_LIN_VERSION{LinVersion::RevisionOne};

LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length, uint8_t *message) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setMessage(message, length);
}

LinMessage::LinMessage(uint8_t address, uint8_t version, uint8_t length, uint8_t *message) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setMessage(message, length);
}

LinMessage::LinMessage(uint8_t address, LinVersion version) :
    m_address{address},
    m_version{version},
    m_length{DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

LinMessage::LinMessage(uint8_t address, uint8_t version) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

LinMessage::LinMessage(uint8_t address, uint8_t version, uint8_t length) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

LinMessage::LinMessage(const LinMessage &other) :
    m_address{other.address()},
    m_version{other.version()},
    m_length{other.length()},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setMessage(other.message(), this->m_length);
}

LinMessage::LinMessage(uint8_t length) :
    m_address{0},
    m_version{DEFAULT_LIN_VERSION},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}


LinMessage::LinMessage() :
    m_address{0},
    m_version{LinVersion::RevisionOne},
    m_length{LinMessage::DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))}
{
    this->setZeroedMessage();
}

LinMessage::~LinMessage()
{
    free(this->m_message);
}

LinMessage& LinMessage::operator=(const LinMessage &rhs)
{
    this->m_length = rhs.length();
    free(this->m_message);
    this->m_message = static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)));
    this->setMessage(rhs.message(), this->m_length);
    return *this;
}

void LinMessage::setZeroedMessage()
{
    for (int i = 0; i < this->m_length; i++) {
        this->m_message[i] = 0x00;
    }
}

void LinMessage::setAddress(uint8_t address)
{
    this->m_address = address;
}

void LinMessage::setVersion(LinVersion version)
{
    this->m_version = version;
}

void LinMessage::setMessage(uint8_t *message, uint8_t length)
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

void LinMessage::setMessage(uint8_t *message)
{
    for (int i = 0; i < this->m_length; i++) {
        if (!(message + i)) {
            this->m_message[i] = 0;
        } else {
            this->m_message[i] = *(message + i);
        }
    }
}

bool LinMessage::setMessageNthByte(uint8_t index, uint8_t nth)
{
    if (index < this->m_length) {
        this->m_message[index] = nth;
        return true;
    } else {
        return false;
    }
}

uint8_t LinMessage::operator[](int index)
{
    if (index < 0) {
        return 0;
    } else {
        return this->nthByte(index);
    }
}

uint8_t LinMessage::nthByte(uint8_t index) const
{
    if (index < this->m_length) {
        return this->m_message[index];
    } else {
        return 0;
    }
}

uint8_t *LinMessage::message() const
{
    return this->m_message;
}

void LinMessage::setLength(uint8_t length)
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

int LinMessage::toString(char *out, size_t maximumLength) const
{
    using namespace Utilities;
    if ((this->m_address == 0) &&
        (this->m_version == LinVersion::RevisionOne) &&
        (this->m_length == 0)) {
        strncpy(out, "", maximumLength);
        return -1;
    }
    char tempVersion[2];
    toDecString(this->m_version, tempVersion, 2);
    strncpy(out, tempVersion, maximumLength);
    strncat(out, ":", maximumLength);

    char tempHexString[SMALL_BUFFER_SIZE];
    char tempFixedWidthString[SMALL_BUFFER_SIZE];
    toHexString(this->m_address, tempHexString, SMALL_BUFFER_SIZE);
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
    char **result{calloc2D<char>(bufferSpace, 10)};
    int resultSize{split(str, result, delimiter, bufferSpace, 10)};
    if (resultSize < (bufferSpace)) {
        free2D(result, bufferSpace);
        return LinMessage{};
    }
    uint8_t tempVersion{stringToUChar(result[0])};
    if ((tempVersion != LinVersion::RevisionOne) && (tempVersion != LinVersion::RevisionTwo)) {
        free2D(result, bufferSpace);
        return returnMessage;
    } 
    returnMessage.setVersion(tempVersion == LinVersion::RevisionOne ? LinVersion::RevisionOne : LinVersion::RevisionTwo);
    returnMessage.setAddress(stringToUChar(result[1]));
    for (uint8_t i = 0; i < messageLength; i++) {
        returnMessage.setMessageNthByte(i, stringToUChar(result[i + 2]));
    }
    free2D(result, bufferSpace);
    return returnMessage;
}

LinVersion LinMessage::toLinVersion(uint8_t version)
{
    if (version == static_cast<uint8_t>(LinVersion::RevisionOne)) {
        return LinVersion::RevisionOne;
    } else if (version == static_cast<uint8_t>(LinVersion::RevisionTwo)) {
        return LinVersion::RevisionTwo;
    } else {
        return LinMessage::DEFAULT_LIN_VERSION;
    }

}
    
