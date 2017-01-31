#ifndef ARDUINOPC_IBYTESTREAM_H
#define ARDUINOPC_IBYTESTREAM_H

#include "utilities.h"

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef SMALL_BUFFER_SIZE
    #define SMALL_BUFFER_SIZE 255
#endif

#define SERIAL_PORT_BUFFER_MAX 1024
#define MAXIMUM_LINE_ENDING_STRING 5
#define MAXIMUM_STRING_COUNT 10

class IByteStream
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
    virtual void print(const char *stringToPrint) = 0;
    virtual void print(char *stringToPrint) = 0;
    virtual void print(char charToPrint) = 0;
    virtual void print(short shortToPrint) = 0;
    virtual void print(int intToPrint) = 0;
    virtual void print(bool boolToPrint) = 0;
    virtual void println(const char *stringToPrint) = 0;
    virtual void println(char *stringToPrint) = 0;
    virtual void println(char charToPrint) = 0;
    virtual void println(short shortToPrint) = 0;
    virtual void println(int intToPrint) = 0;
    virtual void println(bool boolToPrint) = 0;
    virtual IByteStream &operator<<(const char *rhs) = 0;
    virtual IByteStream &operator<<(char *rhs) = 0;
    virtual IByteStream &operator<<(char rhs) = 0;
    virtual IByteStream &operator<<(short rhs) = 0;
    virtual IByteStream &operator<<(int rhs) = 0;
    virtual IByteStream &operator<<(unsigned long rhs) = 0;
    virtual IByteStream &operator<<(bool rhs) = 0;
    virtual ~IByteStream() { }
};

template <typename T>
IByteStream *operator<<(IByteStream *lhs, T rhs)
{
    if (lhs) {
        return (*lhs << rhs);
    } else {
        return nullptr;
    }
}

#endif //ARDUINOPC_IBYTESTREAM_H