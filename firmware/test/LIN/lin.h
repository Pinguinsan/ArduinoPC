#ifndef ARDUINOPC_LIN_H
#define ARDUINOPC_LIN_H

#include <inttypes.h>
#include "linmessage.h"
#include "Arduino.h"
#include <HardwareSerial.h>

#define LIN_SERIAL HardwareSerial
#define LIN_BREAK_DURATION 15    // Number of bits in the break.
#define LIN_TIMEOUT_IN_FRAMES 2     // Wait this many max frame times before declaring a read timeout.
#define SYNC_BYTE 0x55
#define ADDRESS_PARITY_BYTE 0x3F
#define LIN_RECEIVE_SUCCESS 0xFF
#define LIN_WAIT_INTERVAL 100
#define MAXIMUM_DELAY_MICROSECONDS 16383
#define MICROSECONDS_PER_MILLISECOND 1000

#include "heapskew.h"

// This code allows you to derive from the LINFrame class to define messages of different lengths and therefore save a few bytes of RAM.  But it relies on the compiler laying out the class memory in RAM so that the derived class data directly follows the base class.
// If this is not the case in your system, the easiest way to get something working is to only allow full frames, as shown in the #else clause.
// Also, you can over-malloc the LINFrame class to create an extended data buffer.  Of course this method has its own memory management overhead.

enum
{
    Lin1Frame = 0,
    Lin2Frame = 1,
    LinWriteFrame  = 0,
    LinReadFrame   = 2,
};

struct LinScheduleEntry
{
public:
    LinScheduleEntry()
    {
        skewChildren.left = 0; 
        skewChildren.right = 0;
        trigger = 0;
        callback = 0;
        targetAddress = 0;
        messageLength  = 0;
    }
    unsigned long int trigger;  // When to execute (if it is scheduled)
    HeapSkew<LinScheduleEntry>::HeapSkewElement skewChildren;
    uint16_t (*callback)(LinScheduleEntry* me);  // Called after this frame is processed.  Return 1 or greater to reschedule the frame that many milliseconds from now, 0 to drop it.  if callback is NULL, frame is considered a one-shot.
    uint8_t flags;  
    uint8_t targetAddress;
    uint8_t messageLength;
    uint8_t message[1];
    // The data bytes must follow the len field in RAM

    // So that the skew heap orders the elements by when they should be executed
    int operator > (LinScheduleEntry& ee)
    {
        return trigger > ee.trigger;
    }

};

/*
class LinSeFullFrame:public LinScheduleEntry
{
public:
  uint8_t  dontUse[7];  // Code will actually write this by overwriting the "data" buffer in the base class.
};
*/


class LIN
{
public:
    LIN(LIN_SERIAL &serial, uint8_t txPin);
    LIN(LIN_SERIAL& serial, uint8_t txPin, int baudRate);
    
    void setTxPin(uint8_t txPin);
    void setBaudRate(int baudRate);
    void setTimeout(unsigned long timeout);

    uint8_t txPin() const;
    int baudRate() const;
    unsigned long timeout() const;
    bool linIsActive() const;

    void begin(int baudRate);
    void begin();

    // Send a message right now, ignoring the schedule table.
    void sendTo(uint8_t targetAddress, const uint8_t *message, uint8_t numberOfBytes, uint8_t linVersion);
    void sendTo(const LinMessage &linMessage);

    // Receive a message right now, returns 0xff if good checksum, # bytes received (including checksum) if checksum is bad.
    uint8_t receiveFrom(uint8_t targetAddress, uint8_t *message, uint8_t numberOfBytes, uint8_t linVersion);
    LinMessage receiveFrom(uint8_t targetAddress, uint8_t numberOfBytes, LinVersion linVersion, int &status);


    // Add an element to the schedule.  To remove, either clear the whole thing, or remove it when it next plays
    void add(LinScheduleEntry& entry, uint16_t when=0);
    void clear();
    void loop();

protected:
    void generateSerialBreak();
    // For LIN 1.X "start" should = 0, for LIN 2.X "start" should be the addr byte. 
    static uint8_t dataChecksum(const uint8_t* message, uint8_t numberOfBytes, uint16_t start=0);
    static uint8_t addressParity(uint8_t addr);

    HeapSkew<LinScheduleEntry> m_scheduler;
    
    LIN_SERIAL& m_serial;
    uint8_t m_txPin;
    int m_baudRate;
    bool m_serialPortIsActive;
    unsigned long int m_timeout;
    

};

#endif //ARDUINOPC_LIN_H