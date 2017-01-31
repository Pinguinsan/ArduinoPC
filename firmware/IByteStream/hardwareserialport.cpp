#include "hardwareserialport.h"
    
HardwareSerialPort::HardwareSerialPort(HardwareSerial *serialPort, 
                int rxPin, 
                int txPin, 
                long long baudRate, 
                long long timeout,
                bool enabled,
                const char *lineEnding) :
    m_serialPort{serialPort},
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

HardwareSerialPort::~HardwareSerialPort()
{
    delete this->m_lineEnding;
    delete this->m_stringBuilderQueue;
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        delete[] this->m_stringQueue[i];
    }
    delete[] this->m_stringQueue;
}


int HardwareSerialPort::available()
{
    return (this->m_serialPort->available());
}

int HardwareSerialPort::readUntil(char readUntilByte, char *out, size_t maximumReadSize)
{
    return this->readUntil(&readUntilByte, out, maximumReadSize);
}

int HardwareSerialPort::readUntil(const char *readUntilString, char *out, size_t maximumReadSize)
{
    if (!readUntilString) {
        return 0;
    }
    char *tempLineEnding[SMALL_BUFFER_SIZE];
    strncpy(tempLineEnding, this->m_lineEnding, SMALL_BUFFER_SIZE); 
    strncpy(this->m_lineEnding, readUntilString, SMALL_BUFFER_SIZE);
    int readStuff{this->readLine(out, maximumReadSize)};
    strncpy(this->m_lineEnding, tempLineEnding, SMALL_BUFFER_SIZE);
    return readStuff;
}

int HardwareSerialPort::readLine(char *out, size_t maximumReadSize)
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

void HardwareSerialPort::setEnabled(bool enabled) 
{ 
    this->m_isEnabled = enabled; 
}

int HardwareSerialPort::rxPin() const 
{ 
    return this->m_rxPin; 
}

int HardwareSerialPort::txPin() const 
{ 
    return this->m_txPin; 
}

long long HardwareSerialPort::baudRate() const
{
    return this->m_baudRate;
}

long long HardwareSerialPort::timeout() const
{
    return this->m_timeout;
}

bool HardwareSerialPort::serialPortIsNull() const
{
    return this->m_serialPort == nullptr;
}

bool HardwareSerialPort::isEnabled() const 
{ 
    return this->m_isEnabled; 
}

bool HardwareSerialPort::initialize()
{
    if (this->m_serialPort) {
        this->m_serialPort->begin(this->m_baudRate);
        return true;
    } else {
        return false;
    }
    return false;
}

const char *HardwareSerialPort::lineEnding() const
{
    return this->m_lineEnding;
}

void HardwareSerialPort::print(const char *stringToPrint)
{
    this->m_serialPort->print(stringToPrint);
}

void HardwareSerialPort::print(char *stringToPrint)
{
    this->m_serialPort->print(stringToPrint);
}

void HardwareSerialPort::print(char charToPrint)
{
    this->m_serialPort->print(charToPrint);
}

void HardwareSerialPort::print(short shortToPrint)
{
    this->m_serialPort->print(shortToPrint);
}

void HardwareSerialPort::print(int intToPrint)
{
    this->m_serialPort->print(intToPrint);
}

void HardwareSerialPort::print(bool boolToPrint)
{
    this->m_serialPort->print(boolToPrint);
}

void HardwareSerialPort::println(const char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint);
    strncpy(temp, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

void HardwareSerialPort::println(char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint);
    strncpy(temp, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

void HardwareSerialPort::println(char charToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    temp[0] = charToPrint;
    temp[1] = '\0';
    strncpy(temp, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

void HardwareSerialPort::println(short shortToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", (int)shortToPrint, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

void HardwareSerialPort::println(int intToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", intToPrint, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

void HardwareSerialPort::println(bool boolToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", (int)boolToPrint, this->m_lineEnding);
    this->m_serialPort->print(temp);
    delete temp;
}

IByteStream &HardwareSerialPort::operator<<(const char *rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(char *rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(char rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(short rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(int rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(unsigned long rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

IByteStream &HardwareSerialPort::operator<<(bool rhs)
{
    this->m_serialPort->print(rhs);
    return *this;
}

void HardwareSerialPort::syncStringListener()
{
    long long int startTime = millis();
    long long int endTime = millis();
    do {
        char byteRead{static_cast<char>(this->m_serialPort->read())};
        if (FirmwareUtilities::isValidByte(byteRead)) {
            addToStringBuilderQueue(byteRead);
            startTime = millis();
        } else {
            break;
        }
        endTime = millis();
    } while ((endTime - startTime) <= this->m_timeout);
}

void HardwareSerialPort::addToStringBuilderQueue(char byte)
{
    using namespace FirmwareUtilities;
    if (strlen(this->m_stringBuilderQueue) >= SERIAL_PORT_BUFFER_MAX) {
        (void)substring(this->m_stringBuilderQueue, 1, this->m_stringBuilderQueue, SERIAL_PORT_BUFFER_MAX);
    }
    size_t stringLength{strlen(this->m_stringBuilderQueue)};
    this->m_stringBuilderQueue[stringLength] = byte;
    this->m_stringBuilderQueue[stringLength+1] = '\0'; 
    while (substringExists(this->m_stringBuilderQueue, this->m_lineEnding)) {
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