#include "softwareserialport.h"

SoftwareSerialPort::SoftwareSerialPort(SoftwareSerial *serialPort,
                                       uint8_t rxPin, 
                                       uint8_t txPin, 
                                       long long baudRate, 
                                       long long timeout,
                                       bool enabled,
                                       const char *lineEnding) :
    ByteStream(serialPort, rxPin, txPin, baudRate, timeout, enabled, lineEnding),
    m_softwareSerialStream{serialPort}
{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

bool SoftwareSerialPort::initialize()
{
    if (this->m_softwareSerialStream) {
        this->m_softwareSerialStream->begin(this->m_baudRate);
        return true;
    } else {
        return false;
    }
}

SoftwareSerialPort::~SoftwareSerialPort()
{
    free(this->m_lineEnding);
    free(this->m_stringBuilderQueue);
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        free(this->m_stringQueue[i]);
    }
    free(this->m_stringQueue);
}