#include <Arduino.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void doImAliveBlink();
void resetRelays();
void doShiftOut(uint8_t byteToShift, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin);

#define ACTION_TIMEOUT 500
#define MAX_COUNT 65535
#define BYTE_COUNT 8
#define DATA_PIN 0
#define LATCH_PIN 2
#define CLOCK_PIN 3
#define OUTPUT_ENABLE_PIN 4
#define RESET_PIN 5

uint16_t outputCount{MAX_COUNT};
unsigned long actionStartTime{millis()};
unsigned long actionEndTime{millis()};

void setup() {
    outputCount = 0;
    pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    resetRelays();
}

void loop() {
    actionEndTime = millis();
    if ((actionEndTime - actionStartTime) >= ACTION_TIMEOUT) {
        outputCount++;
        doShiftOut(outputCount, DATA_PIN, LATCH_PIN, CLOCK_PIN);
        actionStartTime = millis();
        if (outputCount >= MAX_COUNT) {
            outputCount = 0;
        }
    }
    doImAliveBlink();
}

void doShiftOut(uint8_t byteToShift, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin)
{
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, byteToShift);
    digitalWrite(latchPin, HIGH);
}

void doShiftOut(uint16_t dataToShift, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin)
{
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, (dataToShift >> BYTE_COUNT));  
    shiftOut(dataPin, clockPin, MSBFIRST, dataToShift);
    digitalWrite(latchPin, HIGH);
}

void resetRelays()
{
    //Trigger master reset (active LOW)
    digitalWrite(RESET_PIN, LOW);
    delay(5);
    digitalWrite(RESET_PIN, HIGH);
    digitalWrite(OUTPUT_ENABLE_PIN, LOW);
    
    
    //Shift out a 255, which will turn Q0 - Q7 high, turning the relay board off
    digitalWrite(LATCH_PIN, LOW);
    doShiftOut((uint16_t)MAX_COUNT, DATA_PIN, LATCH_PIN, CLOCK_PIN);
    digitalWrite(LATCH_PIN, HIGH);
}
        
void doImAliveBlink()
{
    #define LED_PIN 1
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
