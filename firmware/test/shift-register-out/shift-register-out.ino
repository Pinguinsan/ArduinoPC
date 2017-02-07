#include <Arduino.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void doImAliveBlink();
void resetRelays();
void doShiftOut(uint8_t byteToShift);

#define ACTION_TIMEOUT 1000
#define MAX_COUNT 255
#define DATA_PIN 3
#define LATCH_PIN 4
#define CLOCK_PIN 5
#define RESET_PIN 2
   
enum Endian
{ 
    LittleEndian, 
    BigEndian
};

class Bitset
{
public:
    Bitset(uint8_t numberOfBits, Endian endian = Endian::LittleEndian) :
        m_numberOfBits{numberOfBits},
        m_underlyingValue{0},
        m_endian{endian}
    {

    }

    void setAllBits()
    {
        for (int i = 0; i < this->m_numberOfBits; i++) {
            this->setBit(i);
        }
    }

    void resetAllBits()
    {
        this->m_underlyingValue = 0;
    }

    void setBit(uint8_t whichOne)
    {
        if (whichOne > this->m_numberOfBits) {

        }
        if (this->m_endian == Endian::LittleEndian) {
            this->m_underlyingValue |= 1 << (this->m_numberOfBits - whichOne - 1);
        } else {
            this->m_underlyingValue |= 1 << whichOne;
        }
    }

    void resetBit(uint8_t whichOne)
    {
        if (whichOne > this->m_numberOfBits) {

        }
        if (this->m_endian == Endian::LittleEndian) {
            this->m_underlyingValue &= ~(1 << (this->m_numberOfBits - whichOne - 1));
        } else {
            this->m_underlyingValue &= ~(1 << whichOne);
        }
    }

    void setByte(uint8_t whatByte)
    {
        for (int i = 0; i < this->m_numberOfBits; i++) {
            ((whatByte >> i) & 1) ? this->setBit(i) : this->resetBit(i);
        }
    }

    bool operator[](uint8_t whichOne)       
    { 
        if (whichOne > this->m_numberOfBits) {

        }
        if (this->m_endian == Endian::LittleEndian) {
            return static_cast<bool>((this->m_underlyingValue >> (this->m_numberOfBits - whichOne - 1)) & 1);
        } else {
            return static_cast<bool>((this->m_underlyingValue >> whichOne) & 1);
        }
    }

    uint8_t underlyingValue() const
    {
        return this->m_underlyingValue;
    }

    uint8_t numberOfBits() const
    {
        return this->m_numberOfBits;
    }

    size_t toString(char *out)
    {
        char temp[2];
        if (this->m_endian == Endian::LittleEndian) {
            for (int i = this->m_numberOfBits - 1; i >= 0; i--) {
                snprintf(temp, 2, "%i", this->operator[](i));
                if (i == (this->m_numberOfBits - 1)) {
                    strncpy(out, temp, this->m_numberOfBits + 1);
                } else {
                strncat(out, temp, this->m_numberOfBits + 1);
                } 
            }
        } else if (this->m_endian == Endian::BigEndian) {
            for (int i = 0; i < this->m_numberOfBits; i++) {
                snprintf(temp, 2, "%i", this->operator[](i));
                if (i == 0) {
                    strncpy(out, temp, this->m_numberOfBits + 1);
                } else {
                strncat(out, temp, this->m_numberOfBits + 1);
                } 
            }
        } else {

        }
        return strlen(out);
    }

private:
    uint8_t m_numberOfBits;
    uint8_t m_underlyingValue;
    Endian m_endian;

};


int main()
{
    arduinoInit();
    pinMode(DATA_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    resetRelays();
    Serial.begin(115200L);
    uint8_t outputCount{MAX_COUNT};
    unsigned long actionStartTime{millis()};
    unsigned long actionEndTime{millis()};

    while (true) {
        actionEndTime = millis();
        if ((actionEndTime - actionStartTime) >= ACTION_TIMEOUT) {
            doShiftOut(outputCount--);
            if (outputCount == 0) {
                outputCount = MAX_COUNT;
            }
            actionStartTime = millis();
        }
        doImAliveBlink();
    }
    return 0;
}

void doShiftOut(uint8_t byteToShift)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, byteToShift);
    digitalWrite(LATCH_PIN, HIGH);
}

void resetRelays()
{
    //Trigger master reset (active LOW)
    digitalWrite(RESET_PIN, LOW);
    delay(5);
    digitalWrite(RESET_PIN, HIGH);
    
    //Shift out a 255, which will turn Q0 - Q7 high, turning the relay board off
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 255);
    digitalWrite(LATCH_PIN, HIGH);
}
        
void doImAliveBlink()
{
    #define LED_PIN 13
    #define BLINK_TIMEOUT 750
    #define BLINK_COUNT_THRESHOLD 2
    static bool ledState{true};
    static long long startTime{millis()};
    static long long endTime{millis()};
    static int blinkCount{0};
    
    pinMode(LED_PIN, OUTPUT);
    endTime = millis();
    if ((endTime - startTime) >= BLINK_TIMEOUT) {
        if (blinkCount++ > BLINK_COUNT_THRESHOLD) {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
        } else {
            digitalWrite(LED_PIN, HIGH);
        }
        startTime = millis();
    }
}
