#include "i2cmasterserialport.h"

I2CMasterSerialPort::DEFAULT_TARGET_SLAVE{1};

I2CMasterSerialPort::I2CMasterSerialPort(uint16_t targetSlave, long long timeout, bool enabled, const char *lineEnding) :
    m_serialPort{&Wire}
    m_timeout{timeout},
    m_isEnabled{enabled},
    m_stringQueueIndex{0},
    m_targetSlave{DEFAULT_TARGET_SLAVE}
{
    /*
    this->m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
    strncpy(this->m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);
    this->m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
    this->m_stringBuilderQueue[0] = '\0';
    this->m_stringQueue = new char*[MAXIMUM_STRING_COUNT];
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        this->m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
        this->m_stringQueue[i][0] = '\0';
    }
    */
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