#ifndef ARDUINOPC_HARDWARESERIALPORT_H
#define ARDUINOPC_HARDWARESERIALPORT_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "bytestream.h"

class HardwareSerialPort : public ByteStream
{
public:
     HardwareSerialPort(HardwareSerial *serialPort, 
                        uint8_t rxPin, 
                        uint8_t txPin, 
                        long long baudRate, 
                        long long timeout,
                        bool enabled,
                        const char *lineEnding);

    ~HardwareSerialPort();

protected:
    virtual bool initialize() override;

private:
    HardwareSerial *m_hardwareSerialStream;
};

#endif //ARDUINOPC_HARDWARESERIALPORT_H