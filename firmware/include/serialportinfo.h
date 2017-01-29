#ifndef ARDUINOPC_SerialPort_H
#define ARDUINOPC_SerialPort_H

#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#include "firmwareutilities.h"

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

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
        m_stringQueueIndex{0}
    {
        this->m_lineEnding = (char *)malloc(strlen(lineEnding) * sizeof(char));
        strcpy(this->m_lineEnding, lineEnding);
        this->m_stringBuilderQueue = (char *)malloc(SERIAL_PORT_BUF_MAX * sizeof(char));
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
        m_stringQueueIndex{0}
    {
        this->m_lineEnding = (char *)malloc(strlen(lineEnding) * sizeof(char));
        strcpy(this->m_lineEnding, lineEnding);
        this->m_stringBuilderQueue = (char *)malloc(SERIAL_PORT_BUF_MAX * sizeof(char));
        this->initialize();
    }

    ~SerialPort()
    {
        delete this->m_lineEnding;
        delete[] this->m_stringBuilderQueue;
        delete this->m_stringQueue;
    }


    bool available()
    {
        return (this->m_serialPort->available());
    }

    int readUntil(char readUntil, char *out, size_t maximumReadSize)
    {
        char temp[1]{readUntil};
        return this->readUntil(temp, out);
    }

    int readUntil(const char *readUntil, char *out, size_t maximumReadSize)
    {
        if (!readUntil) {
            return 0;
        }
        char *tempLineEnding[SMALL_BUFFER_SIZE];
        strncpy(tempLineEnding, this->m_lineEnding, SMALL_BUFFER_SIZE); 
        strncpy(this->m_lineEnding, copyString, SMALL_BUFFER_SIZE);
        int readStuff{this->readLine(out)};
        strncpy(this->m_lineEnding, tempLineEnding, SMALL_BUFFER_SIZE);
        return readStuff;
    }

    int readLine(char *out, size_t maximumReadSize)
    {
        this->syncStringListener();
        if (this->m_stringQueueSize == 0) {
            return 0;
        }
        strncpy(out, this->m_stringQueue[0], maximumReadSize);
        memmove(this->m_stringQueue, this->m_stringQueue+1, (SERIAL_PORT_BUF_MAX - 1)*sizeof(this->m_stringQueue[0]));
        return strlen(out);
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
        if (strlen(this->m_stringBuilderQueue) >= SERIAL_PORT_BUF_MAX) {
            (void)FirmwareUtilities::substring(this->m_stringBuilderQueue, 1, this->m_stringBuilderQueue, SERIAL_PORT_BUF_MAX);
        }
        char temp[1]{byte};
        strcat(this->m_stringBuilderQueue, temp); 
        while (FirmwareUtilities::substringExists(this->m_stringBuilderQueue, this->m_lineEnding)) {
            char stringToAdd[SMALL_BUFFER_SIZE];
            (void)FirmwareUtilities::substring(this->m_stringBuilderQueue, 0, FirmwareUtilities::substringPosition(this->m_stringBuilderQueue, this->m_lineEnding));
            char newStringBuilderQueue[SERIAL_PORT_BUF_MAX];
            (void)FirmwareUtilities::substring(this->m_stringBuilderQueue, FirmwareUtilities::substringPosition(this->m_stringBuilderQueue, this->m_lineEnding) + 1);
            
            strcpy(this->m_stringQueue[i++], stringToAdd);
            strcpy(this->m_stringBuilderQueue, newStringBuilderQueue);
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
    size_t m_stringQueueIndex;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char *m_stringQueue[SERIAL_PORT_BUF_MAX];
    
};

#endif //ARDUINOPC_SerialPort_H