#include "bytestream.h"
    
ByteStream::ByteStream(Stream *stream,
                       uint8_t rxPin,
                       uint8_t txPin,
                       long long baudRate,
                       long long timeout,
                       bool enabled,
                       const char *lineEnding) :
    m_serialPort{stream},
    m_rxPin{rxPin},
    m_txPin{txPin},
    m_baudRate{baudRate},
    m_timeout{timeout},
    m_isEnabled{enabled},
    m_stringQueueIndex{0}
{
    this->m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
    strncpy(this->m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);
    this->m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
    this->m_stringBuilderQueue[0] = '\0';
    this->m_stringQueue = new char*[MAXIMUM_STRING_COUNT];
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        this->m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
        this->m_stringQueue[i][0] = '\0';
    }
}

ByteStream::~ByteStream()
{
    delete this->m_lineEnding;
    delete this->m_stringBuilderQueue;
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        delete[] this->m_stringQueue[i];
    }
    delete[] this->m_stringQueue;
}


int ByteStream::available()
{
    return (this->m_serialPort->available());
}

int ByteStream::readUntil(char readUntilByte, char *out, size_t maximumReadSize)
{
    return this->readUntil(&readUntilByte, out, maximumReadSize);
}

int ByteStream::readUntil(const char *readUntilString, char *out, size_t maximumReadSize)
{
    if (!readUntilString) {
        return 0;
    }
    char *tempLineEnding = new char[SMALL_BUFFER_SIZE];
    strncpy(tempLineEnding, this->m_lineEnding, SMALL_BUFFER_SIZE); 
    strncpy(this->m_lineEnding, readUntilString, SMALL_BUFFER_SIZE);
    int readStuff{this->readLine(out, maximumReadSize)};
    strncpy(this->m_lineEnding, tempLineEnding, SMALL_BUFFER_SIZE);
    delete tempLineEnding;
    return readStuff;
}

int ByteStream::readLine(char *out, size_t maximumReadSize)
{
    this->syncStringListener();
    if (this->m_stringQueueIndex == 0) {
        return 0;
    } else {
        this->m_stringQueueIndex--;
        strncpy(out, this->m_stringQueue[0], maximumReadSize);
        for (unsigned int i = 0; i < MAXIMUM_STRING_COUNT-1; i++){   
            strcpy(this->m_stringQueue[i], this->m_stringQueue[i+1]);
        }
        this->m_stringQueue[MAXIMUM_STRING_COUNT - 1] = new char[SMALL_BUFFER_SIZE];
        this->m_stringQueue[MAXIMUM_STRING_COUNT - 1][0] = '\0';
        return strlen(out);
    }
    return 0;
}

void ByteStream::setEnabled(bool enabled) 
{ 
    this->m_isEnabled = enabled; 
}

uint8_t ByteStream::rxPin() const 
{ 
    return this->m_rxPin; 
}

uint8_t ByteStream::txPin() const 
{ 
    return this->m_txPin; 
}

long long ByteStream::baudRate() const
{
    return this->m_baudRate;
}

long long ByteStream::timeout() const
{
    return this->m_timeout;
}

bool ByteStream::serialPortIsNull() const
{
    return this->m_serialPort == nullptr;
}

bool ByteStream::isEnabled() const 
{ 
    return this->m_isEnabled; 
}

const char *ByteStream::lineEnding() const
{
    return this->m_lineEnding;
}

void ByteStream::print(const char *stringToPrint)
{
    this->m_serialPort->print(stringToPrint);
}

void ByteStream::print(char *stringToPrint)
{
    this->m_serialPort->print(stringToPrint);
}

void ByteStream::print(char charToPrint)
{
    this->m_serialPort->print(charToPrint);
}

void ByteStream::print(short shortToPrint)
{
    this->m_serialPort->print(shortToPrint);
}

void ByteStream::print(unsigned short ushortToPrint)
{
    this->m_serialPort->print(ushortToPrint);
}

void ByteStream::print(int intToPrint)
{
    this->m_serialPort->print(intToPrint);
}

void ByteStream::print(unsigned int uintToPrint)
{
    this->m_serialPort->print(uintToPrint);
}

void ByteStream::print(long longToPrint)
{
    this->m_serialPort->print(longToPrint);
}

void ByteStream::print(unsigned long ulongToPrint)
{
    this->m_serialPort->print(ulongToPrint);
}

void ByteStream::print(long long longLongToPrint)
{
    this->m_serialPort->print((long)longLongToPrint);
}

void ByteStream::print(unsigned long long ulongLongToPrint)
{
    this->m_serialPort->print((unsigned long)ulongLongToPrint);
}

void ByteStream::print(bool boolToPrint)
{
    this->m_serialPort->print(boolToPrint);
}

void ByteStream::println(const char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint, SERIAL_PORT_BUFFER_MAX);
    strncpy(temp, this->m_lineEnding, SERIAL_PORT_BUFFER_MAX);
    this->print(temp);
    delete temp;
}

void ByteStream::println(char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint, SERIAL_PORT_BUFFER_MAX);
    strncpy(temp, this->m_lineEnding, SERIAL_PORT_BUFFER_MAX);
    this->print(temp);
    delete temp;
}

void ByteStream::println(char charToPrint)
{
    char *temp = new char[2];
    temp[0] = charToPrint;
    temp[1] = '\0';
    strncpy(temp, this->m_lineEnding, SERIAL_PORT_BUFFER_MAX);
    this->print(temp);
    delete temp;
}

void ByteStream::println(unsigned short ushortToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%i%s", (int)ushortToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}


void ByteStream::println(short shortToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%i%s", (int)shortToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(int intToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%i%s", intToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(unsigned int uintToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%i%s", uintToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(long longToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%li%s", longToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(long long longLongToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%lli%s", longLongToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(unsigned long ulongToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%li%s", ulongToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(unsigned long long ulongLongToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%lli%s", ulongLongToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void ByteStream::println(bool boolToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    char *boolConvert = new char[6];
    strcpy(boolConvert, (boolToPrint ? "true" : "false"));
    snprintf(temp, SERIAL_PORT_BUFFER_MAX, "%s%s", boolConvert, this->m_lineEnding);
    this->print(temp);
    delete temp;
    delete boolConvert;
}

ByteStream &ByteStream::operator<<(const char *rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned char *rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned char rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(char *rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(char rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned short rhs)
{
    this->print(rhs);
    return *this;
}


ByteStream &ByteStream::operator<<(short rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(int rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned int rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(long rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned long rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(long long rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(unsigned long long rhs)
{
    this->print(rhs);
    return *this;
}

ByteStream &ByteStream::operator<<(bool rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

void ByteStream::syncStringListener()
{
    using namespace Utilities;
    long long int startTime = millis();
    long long int endTime = millis();
    do {
        char byteRead{static_cast<char>(this->m_serialPort->read())};
        if (isValidByte(byteRead)) {
            addToStringBuilderQueue(byteRead);
            startTime = millis();
        } else {
            break;
        }
        endTime = millis();
    } while ((endTime - startTime) <= this->m_timeout);
}

void ByteStream::addToStringBuilderQueue(char byte)
{
    using namespace Utilities;
    if (strlen(this->m_stringBuilderQueue) >= SERIAL_PORT_BUFFER_MAX) {
        (void)substring(this->m_stringBuilderQueue, 1, this->m_stringBuilderQueue, SERIAL_PORT_BUFFER_MAX);
    }
    Serial.print("Adding byte '");
    Serial.print(byte);
    Serial.print("' (");
    Serial.print((int)byte);
    Serial.println(") to this->m_stringBuilderQueue");
    size_t stringLength{strlen(this->m_stringBuilderQueue)};
    this->m_stringBuilderQueue[stringLength] = byte;
    this->m_stringBuilderQueue[stringLength+1] = '\0';
    while (substringExists(this->m_stringBuilderQueue, this->m_lineEnding)) {
        Serial.println("this->m_lineEnding found in this->m_stringBuilderQueue");
        (void)substring(this->m_stringBuilderQueue,
                        0, 
                        positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding), 
                        this->m_stringQueue[this->m_stringQueueIndex++],
                        SMALL_BUFFER_SIZE);
        (void)substring(this->m_stringBuilderQueue,
                        positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding) + 1,
                        this->m_stringBuilderQueue,
                        strlen(this->m_stringBuilderQueue) + 1);
    }
}  