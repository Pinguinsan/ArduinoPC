/* This example code uses 2 different LIN LEDs of unknown type (acquired as 
   samples).  You will have to modify this code to work with whatever LIN
   devices you have.

   TODO: make a Arduino LIN slave so the code works without modification.
 */

#include "Arduino.h"
#include "lin.h"
#include "bitset.h"
#include "linmessage.h"

#define CS_PIN A0
#define FAULT_TXE_PIN A1
#define LIN_SERIAL_TX_PIN 18
#define LIN_BAUD 19200L
#define SERIAL_BAUD 115200L
#define MESSAGE_LENGTH 8
#define LIN_REVISION 2
#define SEND_ID 0x35
#define RECEIVE_ID 0x36
#define DATA_DELIMITER ':'
#define LINE_ENDING '\r'
#define CHANGE_TIMEOUT 1000


LIN *linController;
void onFaultTxePinStateChange()
{
    if (digitalRead(FAULT_TXE_PIN)) {
        onRisingEdgeOfFaultTxePin();
    } else {
        onFallingEdgeOfFaultTxePin();
    }
}

void onRisingEdgeOfFaultTxePin()
{
    Serial.println("faultTxePin transitioned to the HIGH state");
}

void onFallingEdgeOfFaultTxePin()
{
    Serial.println("faultTxePin transitioned to the LOW state");
}

void receiveTest()
{
    uint8_t tempBuffer[MESSAGE_LENGTH]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t receiveOkay = linController->receiveFrom(RECEIVE_ID, tempBuffer, MESSAGE_LENGTH, LIN_REVISION);
    if (receiveOkay == 0) {
        Serial.print("Receive timeout succeeded. ret = ");
        Serial.println(receiveOkay, HEX);
    }  else {
        Serial.print("Receive timeout test failed. ret = ");
        Serial.println(receiveOkay, HEX);
    }
}

bool faultTxePinState{false};
long long sendStartTime{millis()};
long long sendEndTime{millis()};
bool sendState{false};
LinMessage baseMessage{SEND_ID, LIN_REVISION};
LinMessage allOnMessage{SEND_ID, LIN_REVISION};

void setup() {
    linController = new LIN{Serial1, LIN_SERIAL_TX_PIN};
    linController->begin(LIN_BAUD);
    Serial.begin(SERIAL_BAUD);
  
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    pinMode(FAULT_TXE_PIN, INPUT_PULLUP);
    faultTxePinState = digitalRead(FAULT_TXE_PIN);
    Serial.print("initial faultTxePinState: ");
    Serial.println(faultTxePinState ? "HIGH" : "LOW");

    uint8_t *allOnMessageData{static_cast<uint8_t *>(calloc(MESSAGE_LENGTH, sizeof(uint8_t)))};
    allOnMessageData[0] = 0xFF;
    allOnMessageData[1] = 0x00;
    allOnMessageData[2] = 0x00;
    allOnMessageData[3] = 0x00;
    allOnMessageData[4] = 0x00;
    allOnMessageData[5] = 0x00;
    allOnMessageData[6] = 0x00;
    allOnMessageData[7] = 0x00;
    allOnMessage = LinMessage{SEND_ID, LIN_REVISION, MESSAGE_LENGTH, allOnMessageData};
    free(allOnMessageData);

    
}

void loop() {
    sendEndTime = millis();
    if ((sendEndTime - sendStartTime) >= CHANGE_TIMEOUT) {
        sendState = !sendState;
        if (sendState) {
            linController->sendTo(baseMessage);
        } else {
            linController->sendTo(allOnMessage);
        }
        sendStartTime = millis();
    }
    
    int status{0};
    LinMessage linMessage{linController->receiveFrom(RECEIVE_ID, MESSAGE_LENGTH, LIN_REVISION, status)};
    if (status == 0xff) {
        Serial.print("Received full ");
        Serial.print(MESSAGE_LENGTH, DEC);
        Serial.print(" byte LIN message from 0x");
        Serial.print(linMessage.address(), HEX);
        Serial.print(": ");
        for (int i = 0; i < linMessage.length(); i++) {
            if (i != 0) {
                Serial.print(" ");
            }
            Bitset bitset{MESSAGE_LENGTH};
            bitset.setByte(linMessage.nthByte(i));
            char tempBitset[SMALL_BUFFER_SIZE];
            bitset.toString(tempBitset, 0);
            Serial.print(tempBitset);
            if ((i + 1) != linMessage.length()) {
                Serial.print(" |||");
            }
        }
        Serial.println("");

        /*
        char tempMessage[SMALL_BUFFER_SIZE];
        linMessage.toString(tempMessage, SMALL_BUFFER_SIZE);
        Serial.println(tempMessage);
        */
    } else {

    }
    //delay(100);

    if (Serial.available()) {
        String readString{Serial.readStringUntil(LINE_ENDING)};
        while (Serial.available()) {
            int eatEmpty{Serial.read()};
            (void)eatEmpty;
        }
        LinMessage serialLinMessage{LinMessage::parse(readString.c_str(), DATA_DELIMITER, MESSAGE_LENGTH)};
        char temp[SMALL_BUFFER_SIZE];
        int result{serialLinMessage.toString(temp, SMALL_BUFFER_SIZE)};
        if (result < 0) {
            Serial.println("Invalid LIN message read and parsed from Serial");
        } else {
            Serial.print("Read and parsed valid LIN message from Serial: ");
            Serial.println(temp);
        }
        baseMessage = serialLinMessage;
    }   
    bool tempState = digitalRead(FAULT_TXE_PIN);
    if (tempState != faultTxePinState) {
        faultTxePinState = tempState;
        if (faultTxePinState) {
            onRisingEdgeOfFaultTxePin();
        } else {
            onFallingEdgeOfFaultTxePin();
        }
    }
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

