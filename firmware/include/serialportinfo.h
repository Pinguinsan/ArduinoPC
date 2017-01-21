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

#include "firmwareutilities.h"

class SerialPortInfo
{
public:
    virtual ~SerialPortInfo() { }
    virtual bool available() = 0;
    virtual String readString() = 0;
    virtual String readStringUntil(char until) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual short rxPin() const = 0;
    virtual short txPin() const  = 0;
    virtual long long baudRate() const = 0;
    virtual long long timeout() const = 0;
    virtual bool serialPortIsNull() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool initialize() = 0;

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
    virtual SerialPortInfo &operator<<(bool rhs) = 0;
};

class HardwareSerialPortInfo : public SerialPortInfo
{
public:
     HardwareSerialPortInfo(HardwareSerial *serialPort, 
                            short rxPin, 
                            short txPin, 
                            long long baudRate, 
                            long long timeout,
                            bool enabled) :
        m_serialPort{serialPort},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled}
    {

    }

    virtual ~HardwareSerialPortInfo()
    {

    }   

    bool available()
    {
        return this->m_serialPort->available();
    }

    String readString()
    {
        return this->m_serialPort->readString();
    }

    String readStringUntil(char until)
    {
        return this->m_serialPort->readStringUntil(until);
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
            this->m_serialPort->setTimeout(this->m_timeout);
            this->m_serialPort->flush();
            return true;
        } else {
            return false;
        }
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

};



class SoftwareSerialPortInfo : public SerialPortInfo
{
public:
       SoftwareSerialPortInfo( short rxPin, 
                               short txPin, 
                               long long baudRate, 
                               long long timeout,
                               bool enabled) :
        m_serialPort{new SoftwareSerial{static_cast<uint8_t>(rxPin), static_cast<uint8_t>(txPin)}},
        m_rxPin{rxPin},
        m_txPin{txPin},
        m_baudRate{baudRate},
        m_timeout{timeout},
        m_isEnabled{enabled}
    {
        this->m_serialPort->begin(baudRate);
    }

    virtual ~SoftwareSerialPortInfo()
    {
        //delete this->m_serialPort;
    }

    bool available()
    {
        return this->m_serialPort->available();
    }

    String readString()
    {
        unsigned long long startTime{millis()};
        unsigned long long endTime{millis()};
        unsigned long long elapsedTime{0};
        String returnString{""};
        char readChar{0};
        do {
            if (this->m_serialPort->available()) {
                readChar = this->m_serialPort->read();
                if (isPrintable(readChar)) {
                    returnString += readChar;
                }
            }
            endTime = millis();
            elapsedTime = endTime - startTime;
        } while (elapsedTime <= static_cast<unsigned long long>(this->m_timeout));
        return returnString;
    }

    String readStringUntil(char until)
    {
        unsigned long long startTime{millis()};
        unsigned long long endTime{millis()};
        unsigned long long elapsedTime{0};
        String returnString{""};
        char readChar{0};
        do {
            if (this->m_serialPort->available()) {
                readChar = this->m_serialPort->read();
                if (isPrintable(readChar)) {
                    returnString += readChar;
                }
            }
            endTime = millis();
            elapsedTime = endTime - startTime;
        } while ((elapsedTime <= static_cast<unsigned long long>(this->m_timeout)) && (readChar != until));
        return returnString;
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

};

#endif //ARDUINOPC_SERIALPORTINFO_H