#include "i2cslaveserialport.h"

uint8_t I2CSlaveSerialPort::DEFAULT_SLAVE{1};

I2CSlaveSerialPort::I2CSlaveSerialPort(TwoWire *i2cStream, uint8_t slaveNumber, long long timeout, bool enabled, const char *lineEnding) :
    ByteStream(i2cStream, 0, 0, 0, timeout, enabled, lineEnding),
    m_i2cStream{i2cStream},
    m_slaveNumber{slaveNumber}
{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

I2CSlaveSerialPort::I2CSlaveSerialPort(TwoWire *i2cStream, long long timeout, bool enabled, const char *lineEnding) :
    I2CSlaveSerialPort{i2cStream, DEFAULT_SLAVE, timeout, enabled, lineEnding}
{

}

void I2CSlaveSerialPort::bindReceiveCallback(void (*receiveCallback)(int))
{
    this->m_onAfterReceiveCallback = receiveCallback;
}

void I2CSlaveSerialPort::bindRequestCallback(void (*requestCallback)())
{
    this->m_i2cStream->onRequest(requestCallback);
}

bool I2CSlaveSerialPort::initialize()
{
    if (this->m_i2cStream) {
        this->m_i2cStream->begin();
        this->m_onAfterReceiveCallback = nullptr;
        this->m_onAfterRequestCallback = nullptr;
        this->m_i2cStream->onReceive(I2CSlaveSerialPort::onDataReceive);
        this->m_i2cStream->onRequest(I2CSlaveSerialPort::onDataRequest);
        return true;
    } else {
        return false;
    }
}

void I2CSlaveSerialPort::onDataReceive(int howMuch)
{
    (void)howMuch;
    while (this->m_i2cStream->available()) {
        this->syncStringListener();
    }
    if (this->m_onAfterReceiveCallback) {
        this->m_onAfterReceiveCallback();
    }
}

void I2CSlaveSerialPort::onDataRequest()
{
    //TODO: What should be done here?
    if (this->m_onAfterRequestCallback) {
        this->m_onAfterRequestCallback();
    }
}

void I2CSlaveSerialPort::setSlaveNumber(uint8_t slaveNumber)
{
    this->m_slaveNumber = slaveNumber;
}

uint8_t I2CSlaveSerialPort::slaveNumber() const
{
    return this->m_slaveNumber;
}

I2CSlaveSerialPort::~I2CSlaveSerialPort()
{
    free(this->m_lineEnding);
    free(this->m_stringBuilderQueue);
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        free(this->m_stringQueue[i]);
    }
    free(this->m_stringQueue);
}
