#include "Arduino.h"
#include "lin.h"
#include "bitset.h"
#include "linmessage.h"

#define CS_PIN 3
#define FAULT_TXE_PIN 4
#define LOCK_LED_PIN 5
#define UNLOCK_LED_PIN 12
#define LIN_SERIAL_TX_PIN 7
#define LIN_BAUD 9600L
#define MESSAGE_LENGTH 8
#define LIN_REVISION 1
#define SEND_ID 0x3C
#define RECEIVE_ID 0x3D
#define DATA_DELIMITER ':'
#define CHANGE_TIMEOUT 1000

LIN *linController;

long long sendStartTime{millis()};
long long sendEndTime{millis()};
bool sendState{false};
LinMessage baseMessage{SEND_ID, LIN_REVISION};
LinMessage allOnMessage{SEND_ID, LIN_REVISION};

void setup() {
    linController = new LIN{Serial, LIN_SERIAL_TX_PIN};
    linController->begin(LIN_BAUD);
    
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(LOCK_LED_PIN, OUTPUT);
    pinMode(UNLOCK_LED_PIN, OUTPUT);
    digitalWrite(LOCK_LED_PIN, LOW);
    digitalWrite(UNLOCK_LED_PIN, LOW);

    pinMode(FAULT_TXE_PIN, INPUT_PULLUP);
    
    uint8_t *allOnMessageData{static_cast<uint8_t *>(calloc(MESSAGE_LENGTH, sizeof(uint8_t)))};
    allOnMessageData[0] = 0xEC;
    allOnMessageData[1] = 0x01;
    allOnMessageData[2] = 0xD9;
    allOnMessageData[3] = 0b01100000;
    allOnMessageData[4] = 0b01110000;
    allOnMessageData[5] = 0x00;
    allOnMessageData[6] = 0x00;
    allOnMessageData[7] = 0x00;
    allOnMessage = LinMessage{SEND_ID, LIN_REVISION, MESSAGE_LENGTH, allOnMessageData};
    free(allOnMessageData);
    linController->sendTo(allOnMessage);
}

void loop() {
    
    int status{0};
    linController->sendTo(allOnMessage);
    LinMessage linMessage{linController->receiveFrom(RECEIVE_ID, MESSAGE_LENGTH, LIN_REVISION, status)};
    if (status == 0xff) {
        digitalWrite(LOCK_LED_PIN, LOW);
        digitalWrite(UNLOCK_LED_PIN, HIGH);
    } else {
        digitalWrite(LOCK_LED_PIN, HIGH);
        digitalWrite(UNLOCK_LED_PIN, LOW);
    }
    doImAliveBlink();

}

void doImAliveBlink()
{
    #define LED_PIN 1
    #define BLINK_TIMEOUT 750
    #define BLINK_COUNT_THRESHOLD 1
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

