/* This example code uses 2 different LIN LEDs of unknown type (acquired as 
   samples).  You will have to modify this code to work with whatever LIN
   devices you have.

   TODO: make a Arduino LIN slave so the code works without modification.
 */

#include "Arduino.h"
#include "lin.h"
#include "linmessage.h"

#define CS_PIN A0
#define FAULT_TXE_PIN A1
#define LIN_SERIAL_TX_PIN 18
#define LIN_BAUD 9600
#define SERIAL_BAUD 115200L
#define MESSAGE_LENGTH 8
#define LIN_REVISION 1
#define SEND_ID 0x31
#define RECEIVE_ID 0x39
#define DATA_DELIMITER ':'
#define LINE_ENDING '\r'

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

int main()
{
    arduinoInit();
    linController = new LIN{Serial1, LIN_SERIAL_TX_PIN};
    linController->begin(LIN_BAUD);
    Serial.begin(SERIAL_BAUD);
    
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    pinMode(FAULT_TXE_PIN, INPUT_PULLUP);
    bool faultTxePinState{false};
    faultTxePinState = digitalRead(FAULT_TXE_PIN);
    Serial.print("initial faultTxePinState: ");
    Serial.println(faultTxePinState ? "HIGH" : "LOW");
    long long sendStartTime{millis()};
    long long sendEndTime{millis()};
    #define CHANGE_TIMEOUT 1000
    bool sendState{false};
    while (true) {
        uint8_t sendMessageOne[MESSAGE_LENGTH]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00};
        uint8_t sendMessageTwo[MESSAGE_LENGTH]{0x00, 0xAF, 0xBD, 0x40, 0x25, 0x38, 0x00 ,0x80};
        uint8_t receivedMessage[MESSAGE_LENGTH]{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        //sendEndTime = millis();
        if ((sendEndTime - sendStartTime) >= CHANGE_TIMEOUT) {
            sendState = !sendState;
            if (sendState) {
                linController->sendTo(SEND_ID, sendMessageTwo, MESSAGE_LENGTH, LIN_REVISION);
            } else {
                linController->sendTo(SEND_ID, sendMessageOne, MESSAGE_LENGTH, LIN_REVISION);
            }
            sendStartTime = millis();
        }
        
        int status{0};
        LinMessage linMessage{linController->receiveFrom(RECEIVE_ID, MESSAGE_LENGTH, (LIN_REVISION == LinVersion::RevisionOne ? 
                                                                                                       LinVersion::RevisionOne : 
                                                                                                       LinVersion::RevisionTwo), status)};
        if (status == 0xff) {
            Serial.print("Received full ");
            Serial.print(MESSAGE_LENGTH, DEC);
            Serial.print(" byte LIN message: ");
            char tempMessage[SMALL_BUFFER_SIZE];
            linMessage.toString(tempMessage, SMALL_BUFFER_SIZE);
            Serial.println(tempMessage);
        } else {

        }

        if (Serial.available()) {
            String readString{Serial.readStringUntil(LINE_ENDING)};
            while (Serial.available()) {
                int eatEmpty{Serial.read()};
                (void)eatEmpty;
            }
            LinMessage linMessage{LinMessage::parse(readString.c_str(), DATA_DELIMITER, MESSAGE_LENGTH)};
            char temp[SMALL_BUFFER_SIZE];
            int result{linMessage.toString(temp, SMALL_BUFFER_SIZE)};
            if (result < 0) {
                Serial.println("Invalid LIN message read and parsed from Serial");
            } else {
                Serial.print("Read and parsed valid LIN message from Serial: ");
                Serial.println(temp);
            }
            linController->sendTo(linMessage);
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
        if (serialEventRun) serialEventRun();
    }
    delete linController;
    /*
    free(sendMessage);
    free(receivedMessage);
    */
    return 0;
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

