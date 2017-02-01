#ifndef ARDUINOPC_I2CMASTERSERIALPORT_H
#define ARDUINOPC_I2CMASTERSERIALPORT_H

#include <Arduino.h>
#include <Wire.h>
#include "bytestream.h"

class I2CMasterSerialPort : public ByteStream
{
public:
     I2CMasterSerialPort(uint16_t targetSlave, long long timeout, bool enabled, const char *lineEnding);
     I2CMasterSerialPort(long long timeout, bool enabled, const char *lineEnding);

    ~I2CMasterSerialPort();
    void setSlave(uint16_t targetSlave);
    uint16_t targetSlave() const;
    void requestFromSlave(uint8_t howManyBytes);
    override int I2CMasterSerialPort::rxPin() const;
    override int txPin() const;
    override long long baudRate() const;
    override void print(const char *stringToPrint);
    override void print(char *stringToPrint);
    override void print(char charToPrint);
    override void print(short shortToPrint);
    override void print(int intToPrint);
    override void print(bool boolToPrint);
private:
    static uint16_t DEFAULT_SLAVE;
};

#endif //ARDUINOPC_I2CMASTERSERIALPORT_H