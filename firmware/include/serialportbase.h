#ifndef ARDUINOPC_SerialPort_H
#define ARDUINOPC_SerialPort_H

#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#include "firmwareutilities.h"
#include "Arduino.h"

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

#define SERIAL_PORT_BUFFER_MAX 1024
#define MAXIMUM_LINE_ENDING_STRING 5
#define MAXIMUM_STRING_COUNT 10

class SerialPortBase
{
public:
    virtual int available() = 0;
    virtual int readUntil(char readUntilByte, char *out, size_t maximumReadSize) = 0;
    virtual int readUntil(const char *readUntilString, char *out, size_t maximumReadSize) = 0;
    virtual int readLine(char *out, size_t maximumReadSize) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual int rxPin() const = 0;
    virtual int txPin() const = 0;
    virtual long long baudRate() const = 0;
    virtual long long timeout() const = 0;
    virtual bool serialPortIsNull() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool initialize() = 0;
    virtual const char *lineEnding() const = 0;
    virtual void print(char *stringToPrint) = 0;
    virtual void print(char charToPrint) = 0;
    virtual void print(short shortToPrint) = 0;
    virtual void print(int intToPrint) = 0;
    virtual void print(bool boolToPrint) = 0;
    virtual SerialPortBase &operator<<(const char *rhs) = 0;
    virtual SerialPortBase &operator<<(char rhs) = 0;
    virtual SerialPortBase &operator<<(short rhs) = 0;
    virtual SerialPortBase &operator<<(int rhs) = 0;
    virtual SerialPortBase &operator<<(unsigned long rhs) = 0;
    virtual SerialPortBase &operator<<(bool rhs) = 0;
    virtual ~SerialPortBase() { }
};

class HardwareSerialPort : public SerialPortBase
{
public:
     HardwareSerialPort(HardwareSerial *serialPort, 
                        int rxPin, 
                        int txPin, 
                        long long baudRate, 
                        long long timeout,
                        bool enabled,
                        const char *lineEnding) :
        m_serialPort{serialPort},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled},
        m_stringQueueIndex{0}
    {
        this->m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
        strncpy(this->m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);
        this->m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
        for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
            this->m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
        }
    }

    ~HardwareSerialPort()
    {
        delete this->m_lineEnding;
        delete this->m_stringBuilderQueue;
    }


    int available()
    {
        return (this->m_serialPort->available());
    }

    int readUntil(char readUntilByte, char *out, size_t maximumReadSize)
    {
        return this->readUntil(&readUntilByte, out, maximumReadSize);
    }

    int readUntil(const char *readUntilString, char *out, size_t maximumReadSize)
    {
        if (!readUntilString) {
            return 0;
        }
        char tempLineEnding[SMALL_BUFFER_SIZE];
        strncpy(tempLineEnding, this->m_lineEnding, SMALL_BUFFER_SIZE); 
        strncpy(this->m_lineEnding, readUntilString, SMALL_BUFFER_SIZE);
        int readStuff{this->readLine(out, maximumReadSize)};
        strncpy(this->m_lineEnding, tempLineEnding, SMALL_BUFFER_SIZE);
        return readStuff;
    }

    int readLine(char *out, size_t maximumReadSize)
    {
        this->syncStringListener();
        Serial.println("Exited syncStringListener()");
        delay(1000);
        if (this->m_stringQueueIndex == 0) {
            Serial.println("this->m_stringQueueIndex == 0");
            delay(1000);
            return 0;
        } else {
            Serial.println("this->m_stringQueueIndex != 0");
            delay(1000);
            Serial.print("this->m_stringQueue[0] = ");
            Serial.println(this->m_stringQueue[--this->m_stringQueueIndex]);
            delay(1000);
            strncpy(out, this->m_stringQueue[0], maximumReadSize);
            memmove(this->m_stringQueue, this->m_stringQueue+1, (MAXIMUM_STRING_COUNT - 1)*sizeof(this->m_stringQueue[0]));
            return strlen(out);
        }
        return 0;
    }

    void setEnabled(bool enabled) 
    { 
        this->m_isEnabled = enabled; 
    }

    int rxPin() const 
    { 
        return this->m_rxPin; 
    }
    
    int txPin() const 
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
        return false;
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

    SerialPortBase &operator<<(const char *rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(char rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(short rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(int rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(unsigned long rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(bool rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

private:
    HardwareSerial *m_serialPort;
    int m_rxPin;
    int m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    size_t m_stringQueueIndex;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char *m_stringQueue[MAXIMUM_STRING_COUNT];


    void syncStringListener()
    {
        long long int startTime = millis();
        long long int endTime = millis();
        do {
            char byteRead{static_cast<char>(this->m_serialPort->read())};
            Serial.print("SerialPort read byte ");
            Serial.println(byteRead);
            if (FirmwareUtilities::isValidByte(byteRead)) {
                addToStringBuilderQueue(byteRead);
                startTime = millis();
            } else {
                break;
            }
            endTime = millis();
        } while ((endTime - startTime) <= this->m_timeout);
        Serial.println("Exiting syncStringListener()");
        delay(1000);
    }

    void addToStringBuilderQueue(char byte)
    {
        using namespace FirmwareUtilities;
        Serial.print("Entering addToStringBuilderQueue(");
        Serial.print(byte);
        Serial.println(")");
        if (strlen(this->m_stringBuilderQueue) >= SERIAL_PORT_BUFFER_MAX) {
            (void)substring(this->m_stringBuilderQueue, 1, this->m_stringBuilderQueue, SERIAL_PORT_BUFFER_MAX);
        }
        char temp[2];
        temp[0] = byte;
        temp[1] = '\0';
        strcat(this->m_stringBuilderQueue, temp);             
        while (substringExists(this->m_stringBuilderQueue, this->m_lineEnding)) {
            Serial.println("Line ending found in this->m_stringBuilderQueue");
            Serial.print("this->m_stringBuilderQueue = ");
            Serial.println(this->m_stringBuilderQueue);
            char tempString[SMALL_BUFFER_SIZE];
            (void)substring(this->m_stringBuilderQueue,
                            0, 
                            positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding), 
                            tempString,
                            SMALL_BUFFER_SIZE);
            Serial.print("this->m_stringQueueIndex = ");
            Serial.println(this->m_stringQueueIndex);
            strncpy(this->m_stringQueue[this->m_stringQueueIndex], tempString, SMALL_BUFFER_SIZE); 
            (void)substring(this->m_stringBuilderQueue,
                            positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding) + 1,
                            this->m_stringBuilderQueue,
                            strlen(this->m_stringBuilderQueue) + 1);
            Serial.print("this->m_stringBuilderQueue = ");
            Serial.println(this->m_stringBuilderQueue);
            Serial.print("this->m_stringQueue[0] = ");
            Serial.println(this->m_stringQueue[0]);
            Serial.print("this->m_stringQueue[1] = ");
            Serial.println(this->m_stringQueue[1]);
        }
        Serial.println("Exiting addToStringBuilderQueue()");
    }  
};

class SoftwareSerialPort : public SerialPortBase
{
public:
    SoftwareSerialPort(int rxPin, 
                       int txPin, 
                       long long baudRate, 
                       long long timeout,
                       bool enabled,
                       const char *lineEnding) :
        m_serialPort{new SoftwareSerial{static_cast<uint8_t>(rxPin), static_cast<uint8_t>(txPin)}},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled},
        m_stringQueueIndex{0}
    {
        this->m_lineEnding = new char[MAXIMUM_LINE_ENDING_STRING];
        strncpy(this->m_lineEnding, lineEnding, MAXIMUM_LINE_ENDING_STRING);
        this->m_stringBuilderQueue = new char[SERIAL_PORT_BUFFER_MAX];
        for (int i = 0; i < MAXIMUM_STRING_COUNT - 1; i++) {
            this->m_stringQueue[i] = new char[SMALL_BUFFER_SIZE];
        }
    }

    ~SoftwareSerialPort()
    {
        delete this->m_lineEnding;
        delete this->m_stringBuilderQueue;
    }

    int available()
    {
        return (this->m_serialPort->available());
    }

    int readUntil(char readUntilByte, char *out, size_t maximumReadSize)
    {
        return this->readUntil(&readUntilByte, out, maximumReadSize);
    }

    int readUntil(const char *readUntilString, char *out, size_t maximumReadSize)
    {
        if (!readUntilString) {
            return 0;
        }
        char tempLineEnding[SMALL_BUFFER_SIZE];
        strncpy(tempLineEnding, this->m_lineEnding, SMALL_BUFFER_SIZE); 
        strncpy(this->m_lineEnding, readUntilString, SMALL_BUFFER_SIZE);
        int readStuff{this->readLine(out, maximumReadSize)};
        strncpy(this->m_lineEnding, tempLineEnding, SMALL_BUFFER_SIZE);
        return readStuff;
    }

    int readLine(char *out, size_t maximumReadSize)
    {
        this->syncStringListener();
        if (this->m_stringQueueIndex == 0) {
            return 0;
        } else {
            strncpy(out, this->m_stringQueue[0], maximumReadSize);
            memmove(this->m_stringQueue, this->m_stringQueue+1, (MAXIMUM_STRING_COUNT - 1)*sizeof(this->m_stringQueue[0]));
            this->m_stringQueueIndex--;
            return strlen(out);
        }
        return 0;
    }

    void setEnabled(bool enabled) 
    { 
        this->m_isEnabled = enabled; 
    }

    int rxPin() const 
    { 
        return this->m_rxPin; 
    }
    
    int txPin() const 
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
        return false;
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

    SerialPortBase &operator<<(const char *rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(char rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(short rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(int rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(unsigned long rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortBase &operator<<(bool rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

private:
    SoftwareSerial *m_serialPort;
    int m_rxPin;
    int m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    size_t m_stringQueueIndex;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char *m_stringQueue[MAXIMUM_STRING_COUNT];


    void syncStringListener()
    {
        long long int startTime = millis();
        long long int endTime = millis();
        do {
            char byteRead{static_cast<char>(this->m_serialPort->read())};
            Serial.print("SerialPort read byte ");
            Serial.println(byteRead);
            if (FirmwareUtilities::isValidByte(byteRead)) {
                addToStringBuilderQueue(byteRead);
                startTime = millis();
            } else {
                break;
            }
            endTime = millis();
        } while ((endTime - startTime) <= this->m_timeout);
        Serial.println("Exiting syncStringListener()");
        delay(1000);
    }

    void addToStringBuilderQueue(char byte)
    {
        using namespace FirmwareUtilities;
        Serial.print("Entering addToStringBuilderQueue(");
        Serial.print(byte);
        Serial.println(")");
        if (strlen(this->m_stringBuilderQueue) >= SERIAL_PORT_BUFFER_MAX) {
            (void)substring(this->m_stringBuilderQueue, 1, this->m_stringBuilderQueue, SERIAL_PORT_BUFFER_MAX);
        }
        char temp[2];
        temp[0] = byte;
        temp[1] = '\0';
        strcat(this->m_stringBuilderQueue, temp);             
        while (substringExists(this->m_stringBuilderQueue, this->m_lineEnding)) {
            Serial.println("Line ending found in this->m_stringBuilderQueue");
            Serial.println(positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding));
            (void)substring(this->m_stringBuilderQueue,
                            0, 
                            positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding), 
                            this->m_stringQueue[this->m_stringQueueIndex++],
                            SMALL_BUFFER_SIZE);

            (void)substring(this->m_stringBuilderQueue,
                            positionOfSubstring(this->m_stringBuilderQueue, this->m_lineEnding) + 1,
                            this->m_stringBuilderQueue,
                            strlen(this->m_stringBuilderQueue) + 1);
            Serial.print("this->m_stringQueue[0] = ");
            Serial.println(this->m_stringQueue[0]);
        }
        Serial.println("Exiting addToStringBuilderQueue()");
    }  
};

#endif //ARDUINOPC_SERIALPORT_H