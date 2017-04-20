#ifndef ARDUINOPC_LINMASTER_H
#define ARDUINOPC_LINMASTER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <inttypes.h>
#include <util/delay.h>

#define LIN_BREAK_DURATION 15    // Number of bits in the break.
#define LIN_TIMEOUT_IN_FRAMES 2     // Wait this many max frame times before declaring a read timeout.
#define SYNC_BYTE 0x55
#define ADDRESS_PARITY_BYTE 0x3F
#define LIN_RECEIVE_SUCCESS 0xFF
#define LIN_WAIT_INTERVAL 100
#define MAXIMUM_DELAY_MICROSECONDS 16383
#define MICROSECONDS_PER_MILLISECOND 1000

#include "heapskew.h"
#include "linmessage.h"

#define BIT(data,shift) ((data&(1<<shift))>>shift)

template <typename StreamType>
class LinMaster
{
    using SchedulerType = HeapSkew<LinMessage>;

public:
    LinMaster<StreamType>(StreamType &serial, uint8_t txPin) : 
        m_serial{serial}, 
        m_txPin{txPin}
    {

    }
    
    LinMaster<StreamType>(StreamType &serial, uint8_t txPin, unsigned long baudRate) :
        m_serial{serial}, 
        m_txPin{txPin},
        m_baudRate{baudRate}
    {

    }
    
    void setTxPin(uint8_t txPin)
    {
        this->m_txPin = txPin;
    }

    void setBaudRate(unsigned long baudRate)
    {
        if (this->m_baudRate == baudRate) {
            return;
        } else {
            this->m_serial.end();
            this->m_baudRate = baudRate;
            this->begin(this->m_baudRate);
        }
    }
    
    void setTimeout(unsigned long timeout)
    {
        //Meh
        (void)timeout;
    }

    uint8_t txPin() const
    {
        return this->m_txPin;
    }

    unsigned long baudRate() const
    {
        return this->m_baudRate;
    }
    
    unsigned long timeout() const
    {
        return this->m_timeout;
    }
    
    bool linIsActive() const
    {
        return this->m_serialPortIsActive;
    }

    void begin(unsigned long baudRate)
    {
        if (this->m_baudRate != baudRate) {
            this->m_baudRate = baudRate;
        }
        this->m_serial.begin(this->m_baudRate);
        this->m_serialPortIsActive = true;

        unsigned long int Tbit{100000UL/this->m_baudRate};  // Not quite in uSec, I'm saving an extra 10 to change a 1.4 (40%) to 14 below...
        unsigned long int nominalFrameTime{((34*Tbit)+90*Tbit)};  // 90 = 10*max # payload bytes + checksum (9). 
        this->m_timeout = LIN_TIMEOUT_IN_FRAMES * 14 * nominalFrameTime;  // 14 is the specced addtl 40% space above normal*10 -- the extra 10 is just pulled out of the 1000000 needed to convert to uSec (so that there are no decimal #s).
    }
    
    void begin()
    {
        this->begin(this->m_baudRate);
    }

    // Send a message right now, ignoring the schedule table.
    void sendTo(uint8_t targetAddress, const uint8_t *message, uint8_t numberOfBytes, uint8_t linVersion)
    {
        uint8_t addressByte{(targetAddress & ADDRESS_PARITY_BYTE) | addressParity(targetAddress)};
        uint8_t checksum{dataChecksum(message, numberOfBytes,(linVersion  == 1) ? 0 : addressByte)};
        this->generateSerialBreak();       // Generate the low signal that exceeds 1 char.
        this->m_serial.write(SYNC_BYTE);  // Sync byte
        this->m_serial.write(addressByte);  // ID byte
        this->m_serial.write(message, numberOfBytes);  // data bytes
        this->m_serial.write(checksum);  // checksum  
    }
    
    void sendTo(const LinMessage &linMessage)
    {
        LinMaster<StreamType>::sendTo(linMessage.address(), linMessage.message(), linMessage.length(), linMessage.version());
    }

    // Receive a message right now, returns 0xff if good checksum, # bytes received (including checksum) if checksum is bad.
    LinMessage receiveFrom(uint8_t targetAddress, uint8_t numberOfBytes, uint8_t version, uint8_t &status)
    {
        LinVersion linVersion{LinMessage::toLinVersion(version)};
        uint8_t *tempBuffer{(uint8_t *)calloc(numberOfBytes, sizeof(uint8_t))};
        uint8_t receivedSize{this->receiveFrom(targetAddress, tempBuffer, numberOfBytes, linVersion)};
        LinMessage linMessage{numberOfBytes};
        linMessage.setAddress(targetAddress);
        linMessage.setVersion((linVersion == LinVersion::RevisionOne) ? LinVersion::RevisionOne : LinVersion::RevisionTwo);
        if (receivedSize == LIN_RECEIVE_SUCCESS) {
            linMessage.setMessage(tempBuffer, numberOfBytes);
            status = LIN_RECEIVE_SUCCESS;
        } else {
            status = receivedSize;
        }
        free(tempBuffer);
        return linMessage;
    }

    LinMessage receiveFrom(uint8_t targetAddress, uint8_t numberOfBytes, LinVersion linVersion, uint8_t &status)
    {
        uint8_t *tempBuffer{(uint8_t *)calloc(numberOfBytes, sizeof(uint8_t))};
        uint8_t receivedSize{this->receiveFrom(targetAddress, tempBuffer, numberOfBytes, linVersion)};
        LinMessage linMessage{numberOfBytes};
        linMessage.setAddress(targetAddress);
        linMessage.setVersion((linVersion == LinVersion::RevisionOne) ? LinVersion::RevisionOne : LinVersion::RevisionTwo);
        if (receivedSize == LIN_RECEIVE_SUCCESS) {
            linMessage.setMessage(tempBuffer, numberOfBytes);
            status = LIN_RECEIVE_SUCCESS;
        } else {
            status = receivedSize;
        }
        free(tempBuffer);
        return linMessage;
    }

    uint8_t receiveFrom(uint8_t targetAddress, uint8_t *message, uint8_t numberOfBytes, uint8_t linVersion)
    {
        uint8_t bytesReceived{0};
        uint32_t timeoutCount{0};
        generateSerialBreak();       // Generate the low signal that exceeds 1 char.
        this->m_serial.flush();
        this->m_serial.write(SYNC_BYTE);  // Sync byte
        uint8_t idByte{(targetAddress & ADDRESS_PARITY_BYTE) | addressParity(targetAddress)};
        this->m_serial.write(idByte);  // ID byte
        pinMode(this->m_txPin, INPUT);
        digitalWrite(this->m_txPin, LOW);  // don't pull up
        do {
            while(!this->m_serial.available()) {
                _delay_us(LIN_WAIT_INTERVAL); 
                timeoutCount += LIN_WAIT_INTERVAL; 
                if (timeoutCount >= this->m_timeout) {
                    goto done;
                } 
            }

        } while(this->m_serial.read() != SYNC_BYTE);
        do {
            while(!this->m_serial.available()) { 
                _delay_us(LIN_WAIT_INTERVAL); 
                timeoutCount += LIN_WAIT_INTERVAL; 
                if (timeoutCount >= this->m_timeout) {
                    goto done; 
                }
            }
        } while(this->m_serial.read() != idByte);
        for (uint8_t i = 0; i < numberOfBytes; i++) {
            // This while loop strategy does not take into account the added time for the logic.  
            // So the actual this->m_timeout will be slightly longer then written here.
            while(!this->m_serial.available()) { 
                _delay_us(LIN_WAIT_INTERVAL); 
                timeoutCount += LIN_WAIT_INTERVAL; 
                if (timeoutCount >= this->m_timeout) {
                goto done; 
                }
            } 
            message[i] = this->m_serial.read();
            bytesReceived++;
        }
        while(!this->m_serial.available())  {
            _delay_us(LIN_WAIT_INTERVAL); 
            timeoutCount += LIN_WAIT_INTERVAL; 
            if (timeoutCount >= this->m_timeout) {
                goto done; 
            }
        }
        if (this->m_serial.available()) {
            uint8_t receivedChecksum{static_cast<uint8_t>(this->m_serial.read())};
            bytesReceived++;
            if (linVersion == 1) {
                idByte = 0;
            }
            if (dataChecksum(message, numberOfBytes, idByte) == receivedChecksum) {
                bytesReceived = LIN_RECEIVE_SUCCESS;
            }
        }

        done:
        pinMode(this->m_txPin, OUTPUT);
        return bytesReceived;
    }


    // Add an element to the schedule.  To remove, either clear the whole thing, or remove it when it next plays
    void addToScheduleTable(LinMessage& entry, uint16_t when = 0)
    {
         entry.setTriggerTime(millis() + when);
         this->m_scheduler.push(entry); 
    }

    void clearScheduleTable()
    {
        this->m_scheduler.clear();
    }

    void runScheduleTable()
    {
        LinMessage &e = this->m_scheduler.front();
        if (e.triggerTime() < millis()) {
            // remove this frame from the top of the heap.
            this->m_scheduler.pop();
            
            // Do the correct LIN operation
            if (e.frameType() == FrameType::ReadFrame) {
                this->receiveFrom(e);
            } else {
                this->sendTo(e);
            }

            //If there is a callback function, call it.
            if (e.callback()) {
                uint16_t reschedule{e.callback()(&e)};
                if (reschedule) {
                    this->add(e,reschedule);  // Put the frame back on if the caller wants to repeat.
                }
            }
        }  
    }

protected:
    SchedulerType m_scheduler;
    StreamType &m_serial;
    uint8_t m_txPin;
    unsigned long m_baudRate;
    bool m_serialPortIsActive;
    unsigned long m_timeout;

    void generateSerialBreak()
    {
        if (this->m_serialPortIsActive) {
            this->m_serial.end();
        }

        pinMode(this->m_txPin, OUTPUT);
        digitalWrite(this->m_txPin, LOW);  // send BREAK
        unsigned long int endOfBreak = (1000000UL/((unsigned long int)this->m_baudRate));
        unsigned long int beginningOfBreak{endOfBreak * LIN_BREAK_DURATION};
        if (beginningOfBreak > MAXIMUM_DELAY_MICROSECONDS) {
            delay(beginningOfBreak / MICROSECONDS_PER_MILLISECOND);  // delayMicroseconds unreliable above MAXIMUM_DELAY_MICROSECONDS see arduino man pages
        } else {
            delayMicroseconds(beginningOfBreak);
        }
        
        digitalWrite(this->m_txPin, HIGH);  // BREAK delimiter
    
        if (endOfBreak > MAXIMUM_DELAY_MICROSECONDS) {
            delay(endOfBreak / MICROSECONDS_PER_MILLISECOND);  // delayMicroseconds unreliable above MAXIMUM_DELAY_MICROSECONDS see arduino man pages
        } else {
            delayMicroseconds(endOfBreak);
        }

        this->m_serial.begin(this->m_baudRate);
        this->m_serialPortIsActive = 1;

    }
    
    // For LIN 1.X "start" should = 0, for LIN 2.X "start" should be the addr byte. 
    uint8_t dataChecksum(const uint8_t* message, uint8_t numberOfBytes, uint16_t sum = 0)
    {
        uint16_t returnSum{sum};
        while (numberOfBytes-- > 0) {
            returnSum += *(message++);
        }
        // Add the carry
        while (returnSum >> 8)  { // In case adding the carry causes another carry
            returnSum = (returnSum & 255) + (returnSum >> 8);
        }
        return (~returnSum);
    }

    uint8_t addressParity(uint8_t targetAddress)
    {
        (void)targetAddress;
        uint8_t p0{BIT(targetAddress, 0) ^ BIT(targetAddress, 1) ^ BIT(targetAddress, 2) ^ BIT(targetAddress, 4)};
        uint8_t p1{~(BIT(targetAddress, 1) ^ BIT(targetAddress, 3) ^ BIT(targetAddress, 4) ^ BIT(targetAddress, 5))};
        return (p0 | (p1 << 1)) << 6;
    }

};

#endif //ARDUINOPC_LINMASTER_H
