#ifndef ARDUINOPC_SerialPort_H
#define ARDUINOPC_SerialPort_H

#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#include "firmwareutilities.h"

class SerialPort
{
public:
     SerialPort(HardwareSerial *serialPort, 
                short rxPin, 
                short txPin, 
                long long baudRate, 
                long long timeout,
                bool enabled,
                char *lineEnding) :
        m_serialPort{serialPort},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled},
        m_lineEnding{lineEnding}
    {
        this->initialize();
    }

    SerialPort(short rxPin, 
               short txPin, 
               long long baudRate, 
               long long timeout,
               bool enabled,
               char *lineEnding) :
        m_serialPort{new SoftwareSerial{static_cast<uint8_t>(rxPin), static_cast<uint8_t>(txPin)}},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled},
        m_lineEnding{lineEnding}
    {
        this->initialize();
    }

    ~SerialPort()
    {
        delete m_lineEnding;
        delete[] m_stringBuilderQueue;
        delete m_stringQueue;
    }


    bool available()
    {
        return (this->m_serialPort->available());
    }

    int readUntil(char readUntil, char *out, size_t maximumReadSize)
    {
        return this->readUntil(&readUntil, out);
    }

    int readUntil(const char *readUntil, char *out, size_t maximumReadSize)
    {
        if (!readUntil) {
            return 0;
        }
        char *tempLineEnding{this->m_lineEnding};
        this->m_lineEnding = copyString;
        int readStuff{this->readLine(out)};
        this->m_lineEnding = tempLineEnding;
        return readStuff;
    }

    int readLine(char *out, size_t maximumReadSize)
    {
        this->syncStringListener();
        if (this->m_stringQueueSize == 0) {
            return 0;
        }
        out = this->m_stringQueue[0];
        this->m_stringQueue.pop_front();
        return stringToReturn;
    }

    void syncStringListener()
    {
        long long int startTime = FirmwareUtilities::tMillis();
        long long int endTime = FirmwareUtilities::tMillis();
        do {
            char byteRead{this->m_serialPort->read()};
            if (SerialPort::isValid(byteRead)) {
                addToStringBuilderQueue(byteRead);
                startTime = FirmwareUtilities::tMillis();
            } else {
                break;
            }
            endTime = FirmwareUtilities::tMillis();
        } while ((endTime - startTime) <= this->m_timeout);
    }

    void addToStringBuilderQueue(char byte)
    {
        if (this->m_stringBuilderQueue.size() >= SERIAL_PORT_BUF_MAX) {
            this->m_stringBuilderQueue = this->m_stringBuilderQueue.substr(1);
        }
        this->m_stringBuilderQueue += static_cast<char>(byte);
        while (this->m_stringBuilderQueue.find(this->m_lineEnding) != std::string::npos) {
            this->m_stringQueue.push_back(this->m_stringBuilderQueue.substr(0, this->m_stringBuilderQueue.find(this->m_lineEnding)));
            this->m_stringBuilderQueue = this->m_stringBuilderQueue.substr(this->m_stringBuilderQueue.find(this->m_lineEnding) + 1);
        }
    }

    void setEnabled(bool enabled) 
    { 
        this->m_isEnabled = enabled; 
    }

    short rxPin() const 
    { 
        return this->m_rxPin; 
    }
    
    short txPin() const 
    { 
        return this->m_txPin; 
    }

    long long baudRate() const
    {
        return this->m_baudRate;
    }

    long long timeout() const
    {
        return this->m_timeout;
    }

    bool serialPortIsNull() const
    {
        return this->m_serialPort == nullptr;
    }
    
    bool isEnabled() const 
    { 
        return this->m_isEnabled; 
    }

    bool initialize()
    {
        if (this->m_serialPort) {
            this->m_serialPort->begin(this->m_baudRate);
            return true;
        } else {
            return false;
        }
    }

    const char *lineEnding() const
    {
        return this->m_lineEnding;
    }

    void print(char *stringToPrint)
    {
        this->m_serialPort->print(stringToPrint);
    }

    void print(char charToPrint)
    {
        this->m_serialPort->print(charToPrint);
    }
    
    void print(short shortToPrint)
    {
        this->m_serialPort->print(shortToPrint);
    }

    void print(int intToPrint)
    {
        this->m_serialPort->print(intToPrint);
    }

    void print(bool boolToPrint)
    {
        this->m_serialPort->print(boolToPrint);
    }

    SerialPort &operator<<(const char *rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPort &operator<<(char rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPort &operator<<(short rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPort &operator<<(int rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPort &operator<<(unsigned long rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPort &operator<<(bool rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

private:
    HardwareSerial *m_serialPort;
    short m_rxPin;
    short m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char **m_stringQueue;
};

#endif //ARDUINOPC_SerialPort_H