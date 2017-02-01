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

    virtual bool initialize() override;
};

#endif //ARDUINOPC_HARDWARESERIALPORT_H