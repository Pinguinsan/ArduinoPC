#include "softwareserialport.h"

SoftwareSerialPort::SoftwareSerialPort(int rxPin, 
                                       int txPin, 
                                       long long baudRate, 
                                       long long timeout,
                                       bool enabled,
                                       const char *lineEnding) :
    ByteStream(timeout, lineEnding)
    m_serialPort{new SoftwareSerial{static_cast<uint8_t>(rxPin), static_cast<uint8_t>(txPin)}},
    m_rxPin{rxPin},
    m_txPin{txPin},
    m_baudRate{baudRate},
    m_isEnabled{enabled},
    m_stringQueueIndex{0}
{
    /*
    this->m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
    strncpy(this->m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);
    this->m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
    this->m_stringQueue = new char*[MAXIMUM_STRING_COUNT];
    for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
        this->m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
        this->m_stringQueue[i][0] = '\0';
    }
    */
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