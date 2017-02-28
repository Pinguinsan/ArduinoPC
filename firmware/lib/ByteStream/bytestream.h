#ifndef ARDUINOPC_BYTESTREAM_H
#define ARDUINOPC_BYTESTREAM_H

#include <Arduino.h>
#include "utilities.h"

#define MAXIMUM_LINE_ENDING_STRING 5
#define MAXIMUM_STRING_COUNT 2
#define SERIAL_PORT_BUFFER_MAX 255

class ByteStream
{
public:
    ByteStream(Stream *stream,
               uint8_t rxPin,
               uint8_t txPin,
               uint32_t baudRate,
               uint32_t timeout,
               bool enabled,
               const char *lineEnding);
    virtual ~ByteStream();

    virtual int available();
    virtual int readUntil(char readUntilByte, char *out, size_t maximumReadSize);
    virtual int readUntil(const char *readUntilString, char *out, size_t maximumReadSize);
    virtual int readLine(char *out, size_t maximumReadSize);
    virtual void setEnabled(bool enabled);
    virtual uint8_t rxPin() const;
    virtual uint8_t txPin() const;
    virtual uint32_t baudRate() const;
    virtual uint32_t timeout() const;
    virtual bool serialPortIsNull() const;
    virtual bool isEnabled() const;
    virtual const char *lineEnding() const;
    virtual void print(const char *stringToPrint);
    virtual void print(char *stringToPrint);
    virtual void print(char charToPrint);
    virtual void print(short shortToPrint);
    virtual void print(unsigned short ushortToPrint);
    virtual void print(int intToPrint);
    virtual void print(unsigned int uintToPrint);
    virtual void print(long longToPrint);
    virtual void print(unsigned long ulongToPrint);
    virtual void print(long long longLongToPrint);
    virtual void print(unsigned long long ulongLongToPrint);
    virtual void print(bool boolToPrint);
    virtual void println(const char *stringToPrint);
    virtual void println(char *stringToPrint);
    virtual void println(char charToPrint);
    virtual void println(unsigned short ushortToPrint);
    virtual void println(short shortToPrint);
    virtual void println(int intToPrint);
    virtual void println(unsigned int uintToPrint);
    virtual void println(long longToPrint);
    virtual void println(unsigned long ulongToPrint); 
    virtual void println(long long longLongToPrint);
    virtual void println(unsigned long long ulongLongToPrint);     
    virtual void println(bool boolToPrint);
    virtual ByteStream &operator<<(const char *rhs);
    virtual ByteStream &operator<<(char *rhs);
    virtual ByteStream &operator<<(char rhs);
    virtual ByteStream &operator<<(unsigned char *rhs);
    virtual ByteStream &operator<<(unsigned char rhs);
    virtual ByteStream &operator<<(unsigned short rhs);
    virtual ByteStream &operator<<(short rhs);
    virtual ByteStream &operator<<(int rhs);
    virtual ByteStream &operator<<(unsigned int rhs);
    virtual ByteStream &operator<<(long rhs);
    virtual ByteStream &operator<<(unsigned long rhs);
    virtual ByteStream &operator<<(long long rhs);
    virtual ByteStream &operator<<(unsigned long long rhs);
    virtual ByteStream &operator<<(bool rhs);
    virtual bool initialize() = 0;

protected:
    Stream *m_serialPort;
    uint8_t m_rxPin;
    uint8_t m_txPin;
    uint32_t m_baudRate;
    uint32_t m_timeout;
    bool m_isEnabled;
    size_t m_stringQueueIndex;
    char *m_lineEnding;
    char *m_stringBuilderQueue;
    char **m_stringQueue;

    virtual void syncStringListener();
    virtual void addToStringBuilderQueue(char byte);
};

#endif //ARDUINOPC_BYTESTREAM_H