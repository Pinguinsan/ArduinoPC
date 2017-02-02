#include "i2cmasterserialport.h"

uint8_t I2CMasterSerialPort::DEFAULT_TARGET_SLAVE{1};

I2CMasterSerialPort::I2CMasterSerialPort(TwoWire *i2cStream, uint8_t targetSlave, long long timeout, bool enabled, const char *lineEnding) :
    ByteStream(i2cStream, 0, 0, 0, timeout, enabled, lineEnding),
    m_i2cStream{i2cStream},
    m_targetSlave{targetSlave}
{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

I2CMasterSerialPort::I2CMasterSerialPort(TwoWire *i2cStream, long long timeout, bool enabled, const char *lineEnding) :
    I2CMasterSerialPort{i2cStream, DEFAULT_TARGET_SLAVE, timeout, enabled, lineEnding}
{

}

bool I2CMasterSerialPort::initialize()
{
    if (this->m_i2cStream) {
        this->m_i2cStream->begin();
        return true;
    } else {
        return false;
    }
}

void I2CMasterSerialPort::setSlave(uint8_t targetSlave)
{
    this->m_targetSlave = targetSlave;
}

uint8_t I2CMasterSerialPort::targetSlave() const
{
    return this->m_targetSlave;
}

void I2CMasterSerialPort::requestFromSlave(uint8_t howManyBytes)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->requestFrom(this->m_targetSlave, howManyBytes);
    }
}

void I2CMasterSerialPort::print(const char *stringToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(stringToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(char *stringToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(stringToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(char charToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(charToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(short shortToPrint)
{
    if (this->m_i2cStream) {   
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(shortToPrint);
        this->m_i2cStream->endTransmission();
    }    
}

void I2CMasterSerialPort::print(unsigned short ushortToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(ushortToPrint);
        this->m_i2cStream->endTransmission();    
    }
}

void I2CMasterSerialPort::print(int intToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(intToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(unsigned int uintToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(uintToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(long longToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(longToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(unsigned long ulongToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(ulongToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(long long longLongToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print((long)longLongToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(unsigned long long ulongLongToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print((unsigned long)ulongLongToPrint);
        this->m_i2cStream->endTransmission();
    }
}

void I2CMasterSerialPort::print(bool boolToPrint)
{
    if (this->m_i2cStream) {
        this->m_i2cStream->beginTransmission(this->m_targetSlave);
        this->m_i2cStream->print(boolToPrint);
        this->m_i2cStream->endTransmission();
    }    
}  

I2CMasterSerialPort::~I2CMasterSerialPort()
{
    free(this->m_lineEnding);
    free(this->m_stringBuilderQueue);
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        free(this->m_stringQueue[i]);
    }
    free(this->m_stringQueue);
}
