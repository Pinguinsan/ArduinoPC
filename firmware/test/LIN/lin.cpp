#include "Arduino.h"
#include "lin.h"
#include <util/delay.h>

LIN::LIN(LIN_SERIAL& serial, uint8_t txPin): 
    m_serial{serial}, 
    m_txPin{txPin}
{

}

LIN::LIN(LIN_SERIAL& serial, uint8_t txPin, int baudRate): 
    m_serial{serial}, 
    m_txPin{txPin},
    m_baudRate{baudRate}
{

}

void LIN::loop()
{
    LinScheduleEntry& e = this->m_scheduler.front();
    
    if (e.trigger < millis()) {
        // remove this frame from the top of the heap.
        this->m_scheduler.pop();
        uint8_t linVersion{(e.flags & Lin2Frame) ? static_cast<uint8_t>(2) : static_cast<uint8_t>(1)}; 

        // Do the correct LIN operation
        if (e.flags & LinReadFrame) {
            this->receiveFrom(e.targetAddress, e.message, e.messageLength, linVersion);
        } else {
            sendTo(e.targetAddress, e.message, e.messageLength, linVersion);
        }

        // If there is a callback function, call it.
        if (e.callback) {
            uint16_t reschedule{e.callback(&e)};
            if (reschedule) {
                add(e,reschedule);  // Put the frame back on if the caller wants to repeat.
            }
        }
    }  
}

void LIN::setTxPin(uint8_t txPin)
{
    this->m_txPin = txPin;
}

void LIN::setBaudRate(int baudRate)
{
    this->m_serial.end();
    this->m_baudRate = baudRate;
    this->begin(this->m_baudRate);
}

void LIN::setTimeout(unsigned long timeout)
{
   //Meh
   (void)timeout;
}

uint8_t LIN::txPin() const
{
    return this->m_txPin;
}

int LIN::baudRate() const
{
    return this->m_baudRate;
}

unsigned long LIN::timeout() const
{
    return this->m_timeout;
}

bool LIN::linIsActive() const
{
    return this->m_serialPortIsActive;
}

void LIN::begin()
{
    return this->begin(this->m_baudRate);
}

void LIN::begin(int baudRate)
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


// Generate a BREAK signal (a low signal for longer than a byte) across the serial line
void LIN::generateSerialBreak(void)
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

/* LIN defines its checksum as an inverted 8 bit sum with carry */
uint8_t LIN::dataChecksum(const uint8_t *message, uint8_t numberOfBytes, uint16_t sum)
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

/* Create the LIN ID parity */
#define BIT(data,shift) ((targetAddress&(1<<shift))>>shift)
uint8_t LIN::addressParity(uint8_t targetAddress)
{
    uint8_t p0{BIT(targetAddress, 0) ^ BIT(targetAddress, 1) ^ BIT(targetAddress, 2) ^ BIT(targetAddress, 4)};
    uint8_t p1{~(BIT(targetAddress, 1) ^ BIT(targetAddress, 3) ^ BIT(targetAddress, 4) ^ BIT(targetAddress, 5))};
    return (p0 | (p1 << 1)) << 6;
}

/* send a message across the LIN bus */
void LIN::sendTo(uint8_t targetAddress, const uint8_t* message, uint8_t numberOfBytes, uint8_t linVersion)
{
    uint8_t addressByte{(targetAddress & ADDRESS_PARITY_BYTE) | addressParity(targetAddress)};
    uint8_t checksum{dataChecksum(message, numberOfBytes,(linVersion  == 1) ? 0 : addressByte)};
    generateSerialBreak();       // Generate the low signal that exceeds 1 char.
    this->m_serial.write(SYNC_BYTE);  // Sync byte
    this->m_serial.write(addressByte);  // ID byte
    this->m_serial.write(message, numberOfBytes);  // data bytes
    this->m_serial.write(checksum);  // checksum  
}

void LIN::sendTo(const LinMessage &linMessage)
{
    LIN::sendTo(linMessage.address(), linMessage.message(), linMessage.length(), linMessage.version());
}

LinMessage LIN::receiveFrom(uint8_t targetAddress, uint8_t numberOfBytes, LinVersion linVersion, int &status)
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

uint8_t LIN::receiveFrom(uint8_t targetAddress, uint8_t *message, uint8_t numberOfBytes, uint8_t linVersion)
{
    uint8_t bytesReceived{0};
    unsigned int timeoutCount{0};
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
                break;
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
        // This while loop strategy does not take into account the added time for the logic.  So the actual this->m_timeout will be slightly longer then written here.
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

