#include "Arduino.h"
#include "lin.h"
#include "bitset.h"
#include "linmessage.h"

#define CS_PIN A0
#define FAULT_TXE_PIN A1

#define SECOND_CS_PIN A2
#define SECOND_FAULT_TXE_PIN A3

#define LIN_SERIAL_TX_PIN 18
#define SECOND_LIN_SERIAL_TX_PIN 16

#define LIN_BAUD 9600
#define SECOND_LIN_BAUD 19200L

#define SERIAL_BAUD 115200L
#define MESSAGE_LENGTH 8

#define LIN_REVISION 1
#define SECOND_LIN_REVISION 2

#define SEND_ID 0x31
#define RECEIVE_ID 0x39

#define SECOND_SEND_ID 0x35
#define SECOND_RECEIVE_ID 0x36

#define DATA_DELIMITER ':'
#define LINE_ENDING '\r'
#define CHANGE_TIMEOUT 1000


LIN *linController;
LIN *secondLinController;

void onRisingEdgeOfFaultTxePin();
void onFallingEdgeOfFaultTxePin();
void onFaultTxePinStateChange();
void onRisingEdgeOfSecondFaultTxePin();
void onFallingEdgeOfSecondFaultTxePin();
void onSecondFaultTxePinStateChange();

bool faultTxePinState{false};
bool secondFaultTxePinState{false};
long long sendStartTime{millis()};
long long sendEndTime{millis()};
bool sendState{false};
LinMessage baseMessage{SEND_ID, LIN_REVISION};
LinMessage allOnMessage{SEND_ID, LIN_REVISION};

LinMessage secondBaseMessage{SECOND_SEND_ID, SECOND_LIN_REVISION};
LinMessage secondAllOnMessage{SECOND_SEND_ID, SECOND_LIN_REVISION};

void setup() {
    linController = new LIN{Serial1, LIN_SERIAL_TX_PIN};
    linController->begin(LIN_BAUD);

    secondLinController = new LIN{Serial2, SECOND_LIN_SERIAL_TX_PIN};
    secondLinController->begin(SECOND_LIN_BAUD);
    Serial.begin(SERIAL_BAUD);
    Serial3.begin(SERIAL_BAUD);
  
    pinMode(CS_PIN, OUTPUT);
    pinMode(SECOND_CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    digitalWrite(SECOND_CS_PIN, HIGH);

    pinMode(FAULT_TXE_PIN, INPUT_PULLUP);
    pinMode(SECOND_FAULT_TXE_PIN, INPUT_PULLUP);
    faultTxePinState = digitalRead(FAULT_TXE_PIN);
    secondFaultTxePinState = digitalRead(SECOND_FAULT_TXE_PIN);
    Serial.print("Initial faultTxePinState: ");
    Serial.println(faultTxePinState ? "HIGH" : "LOW");

    Serial3.print("Initial secondFaultTxePinState: ");
    Serial3.println(secondFaultTxePinState ? "HIGH" : "LOW");

    uint8_t *allOnMessageData{static_cast<uint8_t *>(calloc(MESSAGE_LENGTH, sizeof(uint8_t)))};
    allOnMessageData[0] = 0x00;
    allOnMessageData[1] = 0xAF;
    allOnMessageData[2] = 0xBD;
    allOnMessageData[3] = 0x40;
    allOnMessageData[4] = 0x25;
    allOnMessageData[5] = 0x38;
    allOnMessageData[6] = 0x00;
    allOnMessageData[7] = 0x80;
    allOnMessage = LinMessage{SEND_ID, LIN_REVISION, MESSAGE_LENGTH, allOnMessageData};
    free(allOnMessageData);

    uint8_t *secondAllOnMessageData{static_cast<uint8_t *>(calloc(MESSAGE_LENGTH, sizeof(uint8_t)))};
    secondAllOnMessageData[0] = 0xFF;
    secondAllOnMessageData[1] = 0x00;
    secondAllOnMessageData[2] = 0x00;
    secondAllOnMessageData[3] = 0x00;
    secondAllOnMessageData[4] = 0x00;
    secondAllOnMessageData[5] = 0x00;
    secondAllOnMessageData[6] = 0x00;
    secondAllOnMessageData[7] = 0x00;
    secondAllOnMessage = LinMessage{SECOND_SEND_ID, SECOND_LIN_REVISION, MESSAGE_LENGTH, secondAllOnMessageData};
    free(secondAllOnMessageData);
}

void loop() {
    sendEndTime = millis();
    if ((sendEndTime - sendStartTime) >= CHANGE_TIMEOUT) {
        sendState = !sendState;
        if (sendState) {
            linController->sendTo(baseMessage);
            secondLinController->sendTo(secondBaseMessage);
        } else {
            linController->sendTo(allOnMessage);
            secondLinController->sendTo(secondAllOnMessage);
        }
        sendStartTime = millis();
    }
    
    int status{0};
    LinMessage linMessage{linController->receiveFrom(RECEIVE_ID, MESSAGE_LENGTH, LIN_REVISION, status)};
    if (status == 0xff) {
        Serial.print("From 0x");
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
                Serial.print(" |");
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

    status = 0;
    LinMessage secondLinMessage{secondLinController->receiveFrom(SECOND_RECEIVE_ID, MESSAGE_LENGTH, SECOND_LIN_REVISION, status)};
    if (status == 0xff) {
        Serial3.print("From 0x");
        Serial3.print(secondLinMessage.address(), HEX);
        Serial3.print(": ");
        for (int i = 0; i < secondLinMessage.length(); i++) {
            if (i != 0) {
                Serial3.print(" ");
            }
            Bitset secondBitset{MESSAGE_LENGTH};
            secondBitset.setByte(secondLinMessage.nthByte(i));
            char secondTempBitset[SMALL_BUFFER_SIZE];
            secondBitset.toString(secondTempBitset, 0);
            Serial3.print(secondTempBitset);
            if ((i + 1) != secondLinMessage.length()) {
                Serial3.print(" |");
            }
        }
        Serial3.println("");

        /*
        char tempMessage[SMALL_BUFFER_SIZE];
        linMessage.toString(tempMessage, SMALL_BUFFER_SIZE);
        Serial.println(tempMessage);
        */
    } else {

    }
    
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
    if (Serial3.available()) {
        String secondReadString{Serial3.readStringUntil(LINE_ENDING)};
        while (Serial3.available()) {
            int eatEmpty{Serial3.read()};
            (void)eatEmpty;
        }
        LinMessage secondSerialLinMessage{LinMessage::parse(secondReadString.c_str(), DATA_DELIMITER, MESSAGE_LENGTH)};
        char secondTemp[SMALL_BUFFER_SIZE];
        int secondResult{secondSerialLinMessage.toString(secondTemp, SMALL_BUFFER_SIZE)};
        if (secondResult < 0) {
            Serial3.println("Invalid LIN message read and parsed from Serial3");
        } else {
            Serial3.print("Read and parsed valid LIN message from Serial3: ");
            Serial3.println(secondTemp);
        }
        secondBaseMessage = secondSerialLinMessage;
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
    tempState = digitalRead(SECOND_FAULT_TXE_PIN);
    if (tempState != secondFaultTxePinState) {
        secondFaultTxePinState = tempState;
        if (secondFaultTxePinState) {
            onRisingEdgeOfSecondFaultTxePin();
        } else {
            onFallingEdgeOfSecondFaultTxePin();
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


void onRisingEdgeOfFaultTxePin()
{
    Serial.println("faultTxePin transitioned to the HIGH state");
}

void onFallingEdgeOfFaultTxePin()
{
    Serial.println("faultTxePin transitioned to the LOW state");
}

void onFaultTxePinStateChange()
{
    if (digitalRead(FAULT_TXE_PIN)) {
        onRisingEdgeOfFaultTxePin();
    } else {
        onFallingEdgeOfFaultTxePin();
    }
}

void onRisingEdgeOfSecondFaultTxePin()
{
    Serial.println("secondFaultTxePin transitioned to the HIGH state");
}

void onFallingEdgeOfSecondFaultTxePin()
{
    Serial.println("secondFaultTxePin transitioned to the LOW state");
}


void onSecondFaultTxePinStateChange()
{
    if (digitalRead(SECOND_FAULT_TXE_PIN)) {
        onRisingEdgeOfSecondFaultTxePin();
    } else {
        onFallingEdgeOfSecondFaultTxePin();
    }
}

