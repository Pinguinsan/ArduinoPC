#ifndef ARDUINOPC_SERIALPORTINFO_H
#define ARDUINOPC_SERIALPORTINFO_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

/* 
 * Standard C++ headers
 * Special thanks to maniacbug for the
 * nice port of the c++ stdlib 
 * https://github.com/maniacbug/StandardCplusplus
 */
#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <serstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>

#include "firmwareutilities.h"

class SerialPortInfo
{
public:
    virtual ~SerialPortInfo() { }
    virtual bool available() = 0;
    virtual std::string readLine() = 0;
    virtual std::string readUntil(char until) = 0;
    virtual std::string readUntil(const char *until) = 0;
    virtual std::string readUntil(const std::string &until) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual short rxPin() const = 0;
    virtual short txPin() const  = 0;
    virtual long long baudRate() const = 0;
    virtual long long timeout() const = 0;
    virtual bool serialPortIsNull() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool initialize() = 0;
    virtual std::string lineEnding() const = 0;

    virtual void print(const std::string &stringToPrint) = 0;
    virtual void print(const char *stringToPrint) = 0;
    virtual void print(char charToprint) = 0;
    virtual void print(short shortToPrint) = 0;
    virtual void print(int intToPrint) = 0;
    virtual void print(bool boolToPrint) = 0;

    virtual SerialPortInfo &operator<<(const std::string &rhs) = 0;
    virtual SerialPortInfo &operator<<(const char *rhs) = 0;
    virtual SerialPortInfo &operator<<(char rhs) = 0;
    virtual SerialPortInfo &operator<<(short rhs) = 0;
    virtual SerialPortInfo &operator<<(int rhs) = 0;
    virtual SerialPortInfo &operator<<(unsigned long rhs) = 0;
    virtual SerialPortInfo &operator<<(bool rhs) = 0;

    static const int SERIAL_PORT_BUF_MAX{8192};
    static bool isValid(char byteToCheck)
    {
        return (isPrintable(byteToCheck) || (byteToCheck == '\r'));
    }
};

class HardwareSerialPortInfo : public SerialPortInfo
{
public:
     HardwareSerialPortInfo(HardwareSerial *serialPort, 
                            short rxPin, 
                            short txPin, 
                            long long baudRate, 
                            long long timeout,
                            bool enabled,
                            const std::string &lineEnding) :
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

         HardwareSerialPortInfo(HardwareSerial *serialPort, 
                                short rxPin, 
                                short txPin, 
                                long long baudRate, 
                                long long timeout,
                                bool enabled,
                                char lineEnding) :
                            HardwareSerialPortInfo(serialPort, rxPin, txPin, baudRate, timeout, enabled, std::string{1, lineEnding})
    {

    }

    virtual ~HardwareSerialPortInfo()
    {

    }   

    bool available()
    {
        return ((this->m_serialPort->available()) || (!this->m_stringQueue.empty()));
    }

    std::string readUntil(char readUntil)
    {
        return this->readUntil(std::string{1, readUntil});
    }

    std::string readUntil(const char *readUntil)
    {
        return this->readUntil(static_cast<std::string>(readUntil));
    }

    std::string readUntil(const std::string &str)
    {
        std::string tempLineEnding{this->m_lineEnding};
        this->m_lineEnding = str;
        std::string readString{this->readLine()};
        this->m_lineEnding = tempLineEnding;
        return readString;
    }

    std::string readLine()
    {
        this->syncStringListener();
        if (this->m_stringQueue.size() == 0) {
            return "";
        }
        std::string stringToReturn{this->m_stringQueue.front()};
        this->m_stringQueue.pop_front();
        return stringToReturn;
    }

    void syncStringListener()
    {
        long long int startTime = millis();
        long long int endTime = millis();
        do {
            char byteRead{this->m_serialPort->read()};
            if (SerialPortInfo::isValid(byteRead)) {
                addToStringBuilderQueue(byteRead);
                startTime = millis();
            } else {
                break;
            }
            endTime = millis();
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
            this->m_serialPort->setTimeout(0);
            this->m_serialPort->flush();
            return true;
        } else {
            return false;
        }
    }

    std::string lineEnding() const
    {
        return this->m_lineEnding;
    }

    void print(const std::string &stringToPrint)
    {
        this->m_serialPort->print(stringToPrint.c_str());
    }

    void print(const char *stringToPrint)
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

    SerialPortInfo &operator<<(const std::string &rhs)
    {
        this->m_serialPort->print(rhs.c_str());
        return *this;
    }

    SerialPortInfo &operator<<(const char *rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(char rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(short rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(int rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(unsigned long rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(bool rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

private:
    HardwareSerial *m_serialPort;
    std::ohserialstream *m_outputStream;
    short m_rxPin;
    short m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    std::string m_lineEnding;
    std::string m_stringBuilderQueue;
    std::deque<std::string> m_stringQueue;
};



class SoftwareSerialPortInfo : public SerialPortInfo
{
public:
       SoftwareSerialPortInfo(short rxPin, 
                              short txPin, 
                              long long baudRate, 
                              long long timeout,
                              bool enabled,
                              const std::string &lineEnding) :
        m_serialPort{new SoftwareSerial{static_cast<uint8_t>(rxPin), static_cast<uint8_t>(txPin)}},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled},
        m_lineEnding{lineEnding}
    {
        this->m_serialPort->begin(baudRate);
    }

         SoftwareSerialPortInfo(short rxPin, 
                                short txPin, 
                                long long baudRate, 
                                long long timeout,
                                bool enabled,
                                char lineEnding) :
                            SoftwareSerialPortInfo(rxPin, txPin, baudRate, timeout, enabled, std::string{1, lineEnding})
    {

    }

    virtual ~SoftwareSerialPortInfo()
    {
        //delete this->m_serialPort;
    }

    bool available()
    {
        return ((this->m_serialPort->available()) || (!this->m_stringQueue.empty()));
    }

    std::string readUntil(char readUntil)
    {
        return this->readUntil(std::string{1, readUntil});
    }

    std::string readUntil(const char *readUntil)
    {
        return this->readUntil(static_cast<std::string>(readUntil));
    }

    std::string readUntil(const std::string &str)
    {
        std::string tempLineEnding{this->m_lineEnding};
        this->m_lineEnding = str;
        std::string readString{this->readLine()};
        this->m_lineEnding = tempLineEnding;
        return readString;
    }

    std::string readLine()
    {
        this->syncStringListener();
        if (this->m_stringQueue.size() == 0) {
            return "";
        }
        std::string stringToReturn{this->m_stringQueue.front()};
        this->m_stringQueue.pop_front();
        return stringToReturn;
    }

    void syncStringListener()
    {
        long long int startTime = millis();
        long long int endTime = millis();
        do {
            char byteRead = this->m_serialPort->read();
            if (SerialPortInfo::isValid(byteRead)) {
                addToStringBuilderQueue(byteRead);
                startTime = millis();
            } else {
                break;
            }
            endTime = millis();
        } while ((endTime - startTime) <= this->m_timeout);
    }

    void addToStringBuilderQueue(char byte)
    {
        if (this->m_stringBuilderQueue.size() >= SERIAL_PORT_BUF_MAX) {
            this->m_stringBuilderQueue = this->m_stringBuilderQueue.substr(1);
        }
        this->m_stringBuilderQueue += byte;
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
            this->m_serialPort->flush();
            return true;
        } else {
            return false;
        }
    }

    std::string lineEnding() const
    {
        return this->m_lineEnding;
    }

    void print(const std::string &stringToPrint)
    {
        this->m_serialPort->print(stringToPrint.c_str());
    }

    void print(const char *stringToPrint)
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

    SerialPortInfo &operator<<(const std::string &rhs)
    {
        this->m_serialPort->print(rhs.c_str());
        return *this;
    }

    SerialPortInfo &operator<<(const char *rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(char rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(short rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

    SerialPortInfo &operator<<(int rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }


    SerialPortInfo &operator<<(unsigned long rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }


    SerialPortInfo &operator<<(bool rhs)
    {
        this->m_serialPort->print(rhs);
        return *this;
    }

private:
    SoftwareSerial *m_serialPort;
    short m_rxPin;
    short m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    std::string m_lineEnding;
    std::string m_stringBuilderQueue;
    std::deque<std::string> m_stringQueue;
};

#endif //ARDUINOPC_SERIALPORTINFO_H