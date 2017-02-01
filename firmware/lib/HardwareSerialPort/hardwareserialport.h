#ifndef ARDUINOPC_HARDWARESERIALPORT_H
#define ARDUINOPC_HARDWARESERIALPORT_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "bytestream.h"

class HardwareSerialPort : public ByteStream
{
public:
     HardwareSerialPort(HardwareSerial *serialPort, 
                        int rxPin, 
                        int txPin, 
                        long long baudRate, 
                        long long timeout,
                        bool enabled,
                        const char *lineEnding);

    ~HardwareSerialPort();
};

#endif //ARDUINOPC_HARDWARESERIALPORT_H