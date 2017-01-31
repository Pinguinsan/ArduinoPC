#ifndef ARDUINOPC_SOFTWARESERIALPORT_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "ibytestream.h"

class SoftwareSerialPort : public IByteStream
{
public:
     SoftwareSerialPort(int rxPin, 
                        int txPin, 
                        long long baudRate, 
                        long long timeout,
                        bool enabled,
                        const char *lineEnding);

    ~SoftwareSerialPort();

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
    SoftwareSerial *m_serialPort;
    int m_rxPin;
    int m_txPin;
    long long m_baudRate;
    long long m_timeout;
    bool m_isEnabled;
    size_t m_stringQueueIndex;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char **m_stringQueue;

    void syncStringListener();
    void addToStringBuilderQueue(char byte);
};

#endif //ARDUINOPC_SOFTWARESERIALPORT_H