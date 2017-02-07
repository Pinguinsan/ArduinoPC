#ifndef ARDUINOPC_SOFTWARESERIALPORT_H
#define ARDUINOPC_SOFTWARESERIALPORT_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "bytestream.h"

class SoftwareSerialPort : public ByteStream
{
public:
     SoftwareSerialPort(SoftwareSerial *serialPort,
                        uint8_t rxPin, 
                        uint8_t txPin, 
                        uint32_t baudRate, 
                        uint32_t timeout,
                        bool enabled,
                        const char *lineEnding);

    
    ~SoftwareSerialPort();
    virtual bool initialize() override;

private:
    SoftwareSerial *m_softwareSerialStream;
};

#endif //ARDUINOPC_SOFTWARESERIALPORT_H