#ifndef ARDUINOPC_I2CMASTERSERIALPORT_H
#define ARDUINOPC_I2CMASTERSERIALPORT_H

#include <Arduino.h>
#include <Wire.h>
#include "bytestream.h"

class I2CMasterSerialPort : public ByteStream
{
public:
     I2CMasterSerialPort(TwoWire *i2cStream, uint8_t targetSlave, long long timeout, bool enabled, const char *lineEnding);
     I2CMasterSerialPort(TwoWire *i2cStream, long long timeout, bool enabled, const char *lineEnding);

    ~I2CMasterSerialPort();
    void setSlave(uint8_t targetSlave);
    uint8_t targetSlave() const;
    void requestFromSlave(uint8_t howManyBytes);
    void print(const char *stringToPrint) override;
    void print(char *stringToPrint) override;
    void print(char charToPrint) override;
    void print(short shortToPrint) override;
    void print(unsigned short ushortToPrint) override;
    void print(int intToPrint) override;
    void print(unsigned int uintToPrint) override;
    void print(long longToPrint) override;
    void print(unsigned long ulongToPrint) override;
    void print(long long longLongToPrint) override;
    void print(unsigned long long ulongLongToPrint) override;
    void print(bool boolToPrint) override; 
    bool initialize() override;

private:
    TwoWire *m_i2cStream;
    uint8_t m_targetSlave;
    static uint8_t DEFAULT_TARGET_SLAVE;
};

#endif //ARDUINOPC_I2CMASTERSERIALPORT_H