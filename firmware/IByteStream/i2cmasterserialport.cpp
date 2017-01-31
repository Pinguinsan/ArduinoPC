#include "i2cmasterserialport.h"

I2CMasterSerialPort::DEFAULT_TARGET_SLAVE{1};

I2CMasterSerialPort::I2CMasterSerialPort(uint16_t targetSlave, long long timeout, bool enabled, const char *lineEnding) :
    m_timeout{timeout},
    m_isEnabled{enabled},
    m_stringQueueIndex{0},
    m_targetSlave{DEFAULT_TARGET_SLAVE}
{
    this->m_serialPort = &Wire;
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

I2CMasterSerialPort::I2CMasterSerialPort(long long timeout, bool enabled, const char *lineEnding) :
    I2CMasterSerialPort{DEFAULT_TARGET_SLAVE, timeout, enabled, lineEnding}
{

}

I2CMasterSerialPort::~I2CMasterSerialPort()
{
    delete this->m_lineEnding;
    delete this->m_stringBuilderQueue;
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        delete[] this->m_stringQueue[i];
    }
    delete[] this->m_stringQueue;
}

void I2CMasterSerialPort::setSlave(uint16_t targetSlave)
{
    this->m_targetSlave = targetSlave;
}

uint16_t I2CMasterSerialPort::targetSlave() const
{
    return this->m_targetSlave;
}

void I2CMasterSerialPort::requestFromSlave(uint8_t howManyBytes)
{
    this->m_serialPort->requestFrom(this->m_targetSlave, howManyBytes);
}

int I2CMasterSerialPort::available()
{
    return (this->m_serialPort->available());
}

int I2CMasterSerialPort::readUntil(char readUntilByte, char *out, size_t maximumReadSize)
{
    return this->readUntil(&readUntilByte, out, maximumReadSize);
}

int I2CMasterSerialPort::readUntil(const char *readUntilString, char *out, size_t maximumReadSize)
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

int I2CMasterSerialPort::readLine(char *out, size_t maximumReadSize)
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

void I2CMasterSerialPort::setEnabled(bool enabled) 
{ 
    this->m_isEnabled = enabled; 
}

int I2CMasterSerialPort::rxPin() const 
{ 
    return 0; 
}

int I2CMasterSerialPort::txPin() const 
{ 
    return 0;
}

long long I2CMasterSerialPort::baudRate() const
{
    return 0;
}

long long I2CMasterSerialPort::timeout() const
{
    return this->m_timeout;
}

bool I2CMasterSerialPort::serialPortIsNull() const
{
    return this->m_serialPort == nullptr;
}

bool I2CMasterSerialPort::isEnabled() const 
{ 
    return this->m_isEnabled; 
}

bool I2CMasterSerialPort::initialize()
{
    if (this->m_serialPort) {
        this->m_serialPort->begin();
        return true;
    } else {
        return false;
    }
    return false;
}

const char *I2CMasterSerialPort::lineEnding() const
{
    return this->m_lineEnding;
}

void I2CMasterSerialPort::print(const char *stringToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(stringToPrint);
    this->m_serialPort->endTransmission();
}

void I2CMasterSerialPort::print(char *stringToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(stringToPrint);
    this->m_serialPort->endTransmission();
}

void I2CMasterSerialPort::print(char charToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(charToPrint);
    this->m_serialPort->endTransmission();
}

void I2CMasterSerialPort::print(short shortToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(shortToPrint);
    this->m_serialPort->endTransmission();    
}

void I2CMasterSerialPort::print(int intToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(intToPrint);
    this->m_serialPort->endTransmission();
}

void I2CMasterSerialPort::print(bool boolToPrint)
{
    this->m_serialPort->beginTransmission(this->m_lineEnding);
    this->m_serialPort->print(boolToPrint);
    this->m_serialPort->endTransmission();    
}

void I2CMasterSerialPort::::println(const char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint);
    strncpy(temp, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void I2CMasterSerialPort::::println(char *stringToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    strncpy(temp, stringToPrint);
    strncpy(temp, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void I2CMasterSerialPort::::println(char charToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    temp[0] = charToPrint;
    temp[1] = '\0';
    strncpy(temp, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void I2CMasterSerialPort::::println(short shortToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", (int)shortToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void I2CMasterSerialPort::::println(int intToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", intToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

void I2CMasterSerialPort::::println(bool boolToPrint)
{
    char *temp = new char[SERIAL_PORT_BUFFER_MAX];
    snprintf("%i%s", (int)boolToPrint, this->m_lineEnding);
    this->print(temp);
    delete temp;
}

IByteStream &I2CMasterSerialPort::::operator<<(const char *rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(char *rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(char rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(short rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(int rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(unsigned long rhs)
{
    this->print(rhs);
    return *this;
}

IByteStream &I2CMasterSerialPort::::operator<<(bool rhs)
{
    this->print(rhs);
    return *this;
}

void I2CMasterSerialPort::syncStringListener()
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

void I2CMasterSerialPort::addToStringBuilderQueue(char byte)
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