#ifndef ARDUINOPC_I2CSLAVESERIALPORT_H
#define ARDUINOPC_I2CSLAVESERIALPORT_H

#include <Arduino.h>
#include <Wire.h>
#include "bytestream.h"

class I2CSlaveSerialPort : public ByteStream
{
public:
     I2CSlaveSerialPort(TwoWire *i2cStream, uint8_t slaveNumber, long long timeout, bool enabled, const char *lineEnding);
     I2CSlaveSerialPort(TwoWire *i2cStream, long long timeout, bool enabled, const char *lineEnding);

    ~I2CSlaveSerialPort();
    void setSlaveNumber(uint8_t slaveNumber);
    uint8_t slaveNumber() const;

    void bindReceiveCallback(void (*receiveCallback)(int));
    void bindRequestCallback(void (*requestCallback)());
    void onDataReceive(int howMuch);
    void onDataRequest();
    
protected:    
    bool initialize() override;

private:
    TwoWire *m_i2cStream;
    void (*I2CSlaveSerialPort::m_onAfterReceiveCallback)()
    void (*I2CSlaveSerialPort::m_onAfterRequestCallback)()     
    uint8_t m_slaveNumber;

    static uint8_t DEFAULT_SLAVE;
};

#endif //ARDUINOPC_I2CSLAVESERIALPORT_H