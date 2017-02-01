#include "softwareserialport.h"

SoftwareSerialPort::SoftwareSerialPort(uint8_t rxPin, 
                                       uint8_t txPin, 
                                       long long baudRate, 
                                       long long timeout,
                                       bool enabled,
                                       const char *lineEnding) :
    ByteStream(new SoftwareSerial{rxPin, txPin)}, rxPin, txPin, baudRate, timeout, enabled, lineEnding)
    m_baudRate{baudRate}
{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

SoftwareSerialPort::~SoftwareSerialPort()
{
    delete this->m_lineEnding;
    delete this->m_stringBuilderQueue;
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        delete[] this->m_stringQueue[i];
    }
    delete[] this->m_stringQueue;
}