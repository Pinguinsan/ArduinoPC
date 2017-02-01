#include "i2cmasterserialport.h"

uint8_t I2CMasterSerialPort::DEFAULT_TARGET_SLAVE{1};

I2CMasterSerialPort::I2CMasterSerialPort(uint8_t targetSlave, long long timeout, bool enabled, const char *lineEnding) :
    ByteStream(&Wire, 0, 0, 0, timeout, enabled, lineEnding),
    m_i2cStream{&Wire},
    m_targetSlave{targetSlave}
{
    if (this->m_isEnabled) {
        this->initialize();
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
    this->m_i2cStream->requestFrom(this->m_targetSlave, howManyBytes);
}

void I2CMasterSerialPort::print(const char *stringToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(stringToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(char *stringToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(stringToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(char charToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(charToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(short shortToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(shortToPrint);
    this->m_i2cStream->endTransmission();    
}

void I2CMasterSerialPort::print(unsigned short ushortToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(ushortToPrint);
    this->m_i2cStream->endTransmission();    
}

void I2CMasterSerialPort::print(int intToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(intToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(unsigned int uintToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(uintToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(long longToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(longToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(unsigned long ulongToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(ulongToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(long long longLongToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print((long)longLongToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(unsigned long long ulongLongToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print((unsigned long)ulongLongToPrint);
    this->m_i2cStream->endTransmission();
}

void I2CMasterSerialPort::print(bool boolToPrint)
{
    this->m_i2cStream->beginTransmission(this->m_targetSlave);
    this->m_i2cStream->print(boolToPrint);
    this->m_i2cStream->endTransmission();    
}  