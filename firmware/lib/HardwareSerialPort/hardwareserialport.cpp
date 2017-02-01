#include "hardwareserialport.h"
    
HardwareSerialPort::HardwareSerialPort(HardwareSerial *serialPort, 
                                       uint8_t rxPin, 
                                       uint8_t txPin, 
                                       long long baudRate, 
                                       long long timeout,
                                       bool enabled,
                                       const char *lineEnding) :
    ByteStream(serialPort, rxPin, txPin, baudRate, timeout, enabled, lineEnding)
{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

HardwareSerialPort::~HardwareSerialPort()
{
    delete this->m_lineEnding;
    delete this->m_stringBuilderQueue;
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        delete[] this->m_stringQueue[i];
    }
    delete[] this->m_stringQueue;
}