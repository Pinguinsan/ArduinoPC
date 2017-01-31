#ifndef ARDUINOPC_HARDWARESERIALPORT_H
#define ARDUINOPC_HARDWARESERIALPORT_H

#include <Arduino.h>
#include <Wire.h>
#include "ibytestream.h"

class I2CMasterSerialPort : public IByteStream
{
public:
     I2CMasterSerialPort(uint16_t targetSlave, long long timeout, bool enabled, const char *lineEnding);
     I2CMasterSerialPort(long long timeout, bool enabled, const char *lineEnding);

    ~I2CMasterSerialPort();

    int available();
    int readUntil(char readUntilByte, char *out, size_t maximumReadSize);
    int readUntil(const char *readUntilString, char *out, size_t maximumReadSize);
    int readLine(char *out, size_t maximumReadSize);
    void setEnabled(bool enabled);
    int rxPin() const;
    int txPin() const;
    long long baudRate() const;
    long long timeout() const;
    bool serialPortIsNull() const;
    bool isEnabled() const;
    bool initialize();
    const char *lineEnding() const;
    void print(const char *stringToPrint);
    void print(char *stringToPrint);
    void print(char charToPrint);
    void print(short shortToPrint);
    void print(int intToPrint);
    void print(bool boolToPrint);
    void println(const char *stringToPrint);
    void println(char *stringToPrint);
    void println(char charToPrint);
    void println(short shortToPrint);
    void println(int intToPrint);
    void println(bool boolToPrint);
    IByteStream &operator<<(const char *rhs);
    IByteStream &operator<<(char *rhs);
    IByteStream &operator<<(char rhs);
    IByteStream &operator<<(short rhs);
    IByteStream &operator<<(int rhs);
    IByteStream &operator<<(unsigned long rhs);
    IByteStream &operator<<(bool rhs);

private:
    TwoWire *m_serialPort;
    long long m_timeout;
    bool m_isEnabled;
    size_t m_stringQueueIndex;
    uint16_t m_targetSlave;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char **m_stringQueue;

    void syncStringListener();
    void addToStringBuilderQueue(char byte);

    static uint16_t DEFAULT_SLAVE;
};

#endif //ARDUINOPC_HARDWARESERIALPORT_H