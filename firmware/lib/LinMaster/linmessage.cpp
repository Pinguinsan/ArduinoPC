#include "linmessage.h"

const uint8_t LinMessage::DEFAULT_MESSAGE_LENGTH{8};
const LinVersion LinMessage::DEFAULT_LIN_VERSION{LinVersion::RevisionOne};
const LinFrameType LinMessage::DEFAULT_FRAME_TYPE{LinFrameType::ReadFrame};

LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length, uint8_t *message) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setMessage(message, length);
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(uint8_t address, uint8_t version, uint8_t length, uint8_t *message) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setMessage(message, length);
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(uint8_t address, LinVersion version) :
    m_address{address},
    m_version{version},
    m_length{DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(uint8_t address, uint8_t version) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(uint8_t address, LinVersion version, uint8_t length) :
    m_address{address},
    m_version{version},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(uint8_t address, uint8_t version, uint8_t length) :
    m_address{address},
    m_version{LinMessage::toLinVersion(version)},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage(const LinMessage &other) :
    m_address{other.address()},
    m_version{other.version()},
    m_length{other.length()},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{other.triggerTime()},
    m_frameType{other.frameType()},
    m_callback{other.callback()}
{
    this->setMessage(other.message(), this->m_length);
    this->m_skewChildren.left = other.skewChildren().left;
    this->m_skewChildren.right = other.skewChildren().right;
}

LinMessage::LinMessage(uint8_t length) :
    m_address{0},
    m_version{DEFAULT_LIN_VERSION},
    m_length{length},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
}

LinMessage::LinMessage() :
    m_address{0},
    m_version{LinVersion::RevisionOne},
    m_length{LinMessage::DEFAULT_MESSAGE_LENGTH},
    m_message{static_cast<uint8_t *>(calloc(this->m_length, sizeof(uint8_t)))},
    m_triggerTime{0},
    m_frameType{DEFAULT_FRAME_TYPE},
    m_callback{nullptr}
{
    this->setZeroedMessage();
    this->zeroOutSkewChildren();
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

void LinMessage::setVersion(uint8_t version)
{
    this->m_version = LinMessage::toLinVersion(version);
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

uint8_t LinMessage::operator[](uint8_t index) const
{
    return this->nthByte(index);
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

size_t LinMessage::toString(char *out, size_t maximumLength) const
{
    if (maximumLength == 0) {
        return -1;
    }
    char tempVersion[2];
    snprintf(tempVersion, 2, "%i", static_cast<int>(this->m_version));
    strncpy(out, tempVersion, maximumLength);
    strncat(out, " : ", maximumLength);

    char tempHexString[SMALL_BUFFER_SIZE];
    toFixedWidthHex(tempHexString, SMALL_BUFFER_SIZE, this->m_address, 2, true);
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

LinMessage LinMessage::parse(const char *str, char delimiter)
{
    const char temp[2]{delimiter, '\0'};
    return LinMessage::parse(str, temp);
}

LinMessage LinMessage::parse(const char *str, const char *delimiter)
{
    char **result{calloc2D<char>(LIN_MESSAGE_PARSE_BUFFER_SPACE, 4)};
    size_t resultSize{split(str, result, delimiter, LIN_MESSAGE_PARSE_BUFFER_SPACE, 4)};
    if (resultSize < LIN_MESSAGE_PARSE_BUFFER_SPACE) {
        free2D(result, LIN_MESSAGE_PARSE_BUFFER_SPACE);
        return LinMessage{};
    }
    uint8_t tempVersion{static_cast<uint8_t>(strtol(result[0], nullptr, 0))};
    if ((tempVersion != LinVersion::RevisionOne) && (tempVersion != LinVersion::RevisionTwo)) {
        free2D(result, LIN_MESSAGE_PARSE_BUFFER_SPACE);
        return LinMessage{};
    } 
    LinMessage returnMessage{static_cast<uint8_t>(strtol(result[1], nullptr, 0)),
                             tempVersion == LinVersion::RevisionOne ? LinVersion::RevisionOne : LinVersion::RevisionTwo,
                             static_cast<uint8_t>(resultSize - 2)};
    for (uint8_t i = 0; i < returnMessage.length(); i++) {
        returnMessage.setMessageNthByte(i, static_cast<uint8_t>(strtol(result[i + 2], nullptr, 0)));
    }
    free2D(result, LIN_MESSAGE_PARSE_BUFFER_SPACE);
    return returnMessage;
}

LinFrameType LinMessage::frameType() const
{
    return this->m_frameType;
}

unsigned long LinMessage::triggerTime() const
{
    return this->m_triggerTime;
}

callback_ptr_t LinMessage::callback() const
{
    return this->m_callback;
}

heap_skew_element_t LinMessage::skewChildren() const
{
    return this->m_skewChildren;
}

void LinMessage::setCallback(callback_ptr_t callback)
{
    this->m_callback = callback;
}

void LinMessage::setFrameType(uint8_t frameType)
{
    this->m_frameType = LinMessage::toFrameType(frameType);
}

void LinMessage::setFrameType(LinFrameType frameType)
{
    this->m_frameType = frameType;
}

void LinMessage::setTriggerTime(unsigned long triggerTime)
{
    this->m_triggerTime = triggerTime;
}

void LinMessage::zeroOutSkewChildren()
{
    m_skewChildren.left = nullptr;
    m_skewChildren.right = nullptr;
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
    
LinFrameType LinMessage::toFrameType(uint8_t frameType)
{
    if (frameType == static_cast<uint8_t>(LinFrameType::ReadFrame)) {
        return LinFrameType::ReadFrame;
    } else if (frameType == static_cast<uint8_t>(LinFrameType::WriteFrame)) {
        return LinFrameType::WriteFrame;
    } else {
        return LinMessage::DEFAULT_FRAME_TYPE;
    }
}

bool LinMessage::substringExists(const char *first, const char *second)
{
    if ((!first) || (!second)) {
        return false;
    }
    return (strstr(first, second) != nullptr);
}

bool LinMessage::substringExists(const char *first, char second)
{
    char temp[2]{second, '\0'};
    return (substringExists(first, temp));
}

size_t LinMessage::positionOfSubstring(const char *first, const char *second)
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

size_t LinMessage::positionOfSubstring(const char *first, char second)
{
    char temp[2]{second, '\0'};
    return positionOfSubstring(first, temp);
}

size_t LinMessage::substring(const char *str, size_t startPosition, char *out, size_t maximumLength)
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

size_t LinMessage::substring(const char *str, size_t startPosition, size_t length, char *out, size_t maximumLength)
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

size_t LinMessage::split(const char *str, char **out, const char *delimiter, size_t maximumElements, size_t maximumLength)
{
    char *copyString = (char *)calloc(strlen(str) + 1, sizeof(char));
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

size_t LinMessage::split(const char *str, char **out, const char delimiter, size_t maximumElements, size_t maximumLength)
{
    char temp[2]{delimiter, '\0'};
    return split(str, out, temp, maximumElements, maximumLength);
}
