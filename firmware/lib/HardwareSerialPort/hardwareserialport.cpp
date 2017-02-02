#include "hardwareserialport.h"
    
HardwareSerialPort::HardwareSerialPort(HardwareSerial *serialPort, 
                                       uint8_t rxPin, 
                                       uint8_t txPin, 
                                       long long baudRate, 
                                       long long timeout,
                                       bool enabled,
                                       const char *lineEnding) :
    ByteStream(serialPort, rxPin, txPin, baudRate, timeout, enabled, lineEnding),
    m_hardwareSerialStream{serialPort}

{
    if (this->m_isEnabled) {
        this->initialize();
    }
}

bool HardwareSerialPort::initialize()
{
    if (this->m_hardwareSerialStream) {
        this->m_hardwareSerialStream->begin(this->m_baudRate);
        return true;
    } else {
        return false;
    }
}

HardwareSerialPort::~HardwareSerialPort()
{
    free(this->m_lineEnding);
    free(this->m_stringBuilderQueue);
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        free(this->m_stringQueue[i]);
    }
    free(this->m_stringQueue);
}