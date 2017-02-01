#ifndef ARDUINOPC_SOFTWARESERIALPORT_H
#define ARDUINOPC_SOFTWARESERIALPORT_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "bytestream.h"

class SoftwareSerialPort : public ByteStream
{
public:
     SoftwareSerialPort(int rxPin, 
                        int txPin, 
                        long long baudRate, 
                        long long timeout,
                        bool enabled,
                        const char *lineEnding);

    ~SoftwareSerialPort();
};

#endif //ARDUINOPC_SOFTWARESERIALPORT_H