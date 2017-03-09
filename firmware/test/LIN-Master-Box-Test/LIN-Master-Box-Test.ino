#include "linmaster.h"
#include "bitset.h"

#define CS_PIN 10
#define FAULT_TXE_PIN A15
#define LOCK_LED_PIN 22
#define UNLOCK_LED_PIN 23
#define LIN_SERIAL_TX_PIN 18
#define LIN_BAUD 9600
#define RECEIVE_ID 0x2B
#define MESSAGE_LENGTH 4
#define LIN_REVISION LinVersion::RevisionOne
#define DATA_DELIMITER ':'
#define CHANGE_TIMEOUT 1000

LinMaster<HardwareSerial> *linController;

static const uint8_t sendIDs[]{0x28, 0x3C, 0x29};
static const uint8_t receiveIDs[]{0x2B, 0x3D}; 

long long sendStartTime{millis()};
long long sendEndTime{millis()};
bool sendState{false};
LinMessage firstMessage{sendIDs[0], LIN_REVISION};
LinMessage secondMessage{sendIDs[1], LIN_REVISION};
LinMessage thirdMessage{sendIDs[2], LIN_REVISION};

void messageScan(LinMaster<HardwareSerial> *controller, const uint8_t *receiveIDs, uint8_t receiveIDLength, Stream *stream);
void messageScan(LinMaster<HardwareSerial> *controller, Stream *stream);
void doImAliveBlink();

void setup() {
    linController = new LinMaster<HardwareSerial>{Serial1, LIN_SERIAL_TX_PIN};
    linController->begin(LIN_BAUD);

    Serial.begin(115200L);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    
    pinMode(LOCK_LED_PIN, OUTPUT);
    pinMode(UNLOCK_LED_PIN, OUTPUT);
    digitalWrite(LOCK_LED_PIN, HIGH);
    digitalWrite(UNLOCK_LED_PIN, HIGH);

    pinMode(FAULT_TXE_PIN, INPUT_PULLUP);
    Serial.print("initial faultTxePin state: ");
    digitalRead(FAULT_TXE_PIN) ? Serial.println("HIGH") : Serial.println("LOW");

}

void loop() {
  /*
    sendEndTime = millis();
    if ((sendEndTime - sendStartTime) >= CHANGE_TIMEOUT) {
        sendState = !sendState;
        if (sendState) {
            linController->sendTo(firstMessage);
            linController->sendTo(thirdMessage);
        } else {
            linController->sendTo(secondMessage);
        }
        sendStartTime = millis();
    }
    ?*/
    //messageScan(linController, receiveIDs, ARRAY_SIZE(receiveIDs), &Serial);
    //messageScan(linController, &Serial);
    
    uint8_t status{0};
    LinMessage linMessage{linController->receiveFrom(RECEIVE_ID, MESSAGE_LENGTH, LIN_REVISION, status)};
    if (status != 0) {
        Serial.print("Received from 0x");
        Serial.print(linMessage.address(), HEX);
        Serial.print(" (");
        Serial.print("status = 0x");
        Serial.print(status, HEX);
        Serial.print("): ");
        for (uint8_t i = 0; i < MESSAGE_LENGTH; i++) {
            bitset<8> messageBits{};
            messageBits.setMultiple(linMessage[i]);
            char temp[SMALL_BUFFER_SIZE];
            messageBits.toString(temp);
            Serial.print("0b");
            Serial.print(temp);
            if ((i+1) != MESSAGE_LENGTH) {
                Serial.print(":");
            }
        }
        Serial.println();
        digitalWrite(LOCK_LED_PIN, LOW);
        digitalWrite(UNLOCK_LED_PIN, HIGH);
    } else {
        digitalWrite(LOCK_LED_PIN, HIGH);
        digitalWrite(UNLOCK_LED_PIN, LOW);
    }
    //*/
    doImAliveBlink();

}

void doImAliveBlink()
{
    #define LED_PIN 13
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

void messageScan(LinMaster<HardwareSerial> *controller, Stream *stream)
{
    if ((!controller) || (!stream)) {
        return;
    }
    for (uint8_t i = 0; i < 0xFF; i++) {
        uint8_t msgStatus{0};
        LinMessage message{controller->receiveFrom(i, MESSAGE_LENGTH, LIN_REVISION, msgStatus)};
        if (msgStatus == 0xFF) {
            stream->print("Received from 0x");
            stream->print(message.address(), HEX);
            stream->print(" : ");
            bitset<8> bitset;
            for (uint8_t i = 0; i < MESSAGE_LENGTH; i++) {
                bitset.reset();
                bitset.setMultiple(message[i]);
                char temp[SMALL_BUFFER_SIZE];
                bitset.toString(temp);
                stream->print("0b");
                stream->print(temp);
                if ((i+1) != MESSAGE_LENGTH) {
                    stream->print(" : ");
                }
            }
            stream->println();
        } else {
            stream->print("No message received from 0x");
            stream->println(i, HEX);
        }
    }

    delay(500);
}

void messageScan(LinMaster<HardwareSerial> *controller, const uint8_t *receiveIDs, uint8_t receiveIDLength, Stream *stream)
{
    if ((!controller) || (!stream) || (!receiveIDs)) {
        return;
    }
    for (uint8_t i = 0; i < receiveIDLength; i++) {
        uint8_t msgStatus{0};
        LinMessage message{controller->receiveFrom(receiveIDs[i], MESSAGE_LENGTH, LIN_REVISION, msgStatus)};
        if (msgStatus == 0xFF) {
            stream->print("Received from 0x");
            stream->print(message.address(), HEX);
            stream->print(" : ");
            bitset<8> bitset;
            for (uint8_t i = 0; i < MESSAGE_LENGTH; i++) {
                bitset.reset();
                bitset.setMultiple(message[i]);
                char temp[SMALL_BUFFER_SIZE];
                bitset.toString(temp);
                stream->print("0b");
                stream->print(temp);
                if ((i+1) != MESSAGE_LENGTH) {
                    stream->print(" : ");
                }
            }
            stream->println();
        } else {
            stream->print("No message received from 0x");
            stream->println(receiveIDs[i], HEX);
        }
    }

    delay(500);
}

