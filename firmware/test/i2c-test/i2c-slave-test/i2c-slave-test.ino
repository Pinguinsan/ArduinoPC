#include <Wire.h>
#include <string.h>
#include <stdlib.h>

#include <Arduino.h>

#define SERIAL_READ_MAX 255
#define SERIAL_BAUD_RATE 115200L
#define SLAVE_NUMBER 8
#define LINE_ENDING '}'

void reverseString(char *str, size_t stringLength);
void i2cReceiveEvent(int howMany);
void i2cRequestEvent(); 
int doSerialRead();
int doI2CRead();
void doI2CSlaveWrite(const char *str, size_t stringLength);
void doSerialWrite(const char *str, size_t stringLength);
char *i2cRead;
char *i2cWrite;

int main()
{
    arduinoInit();
    Wire.begin(SLAVE_NUMBER);
    Wire.onReceive(i2cReceiveEvent);
    Wire.onRequest(i2cRequestEvent);
    Serial.begin(SERIAL_BAUD_RATE);
    i2cRead = (char *)calloc(SERIAL_READ_MAX, sizeof(char));
    i2cWrite = (char *)calloc(SERIAL_READ_MAX, sizeof(char));

    while (true) {
        if (strlen(i2cRead) > 0) {
            if (i2cRead[strlen(i2cRead) - 1] == LINE_ENDING) {
                i2cRead[strlen(i2cRead) - 1] = '\0';
                Serial.print("Received '");
                Serial.print(i2cRead);
                Serial.println("' from i2c bus");
                strcpy(i2cWrite, i2cRead);
                doI2CSlaveWrite(i2cWrite, strlen(i2cWrite)); 
                memset(i2cRead, '\0', SERIAL_READ_MAX);
                memset(i2cWrite, '\0', SERIAL_READ_MAX);
                //reverseString(i2cRead, strlen(i2cRead));
            }
        }
        doImAliveBlink();
        if (serialEventRun) serialEventRun();
    }
    free(i2cRead);
    free(i2cWrite);
    return 0;
}

void doImAliveBlink()
{
    #define LED_PIN 13
    #define BLINK_TIMEOUT 250
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

void reverseString(char *str, size_t stringLength)
{
    char *temp = (char *)calloc(stringLength + 1, sizeof(char));
    for (int i = stringLength - 1; i >= 0; i--) {
        temp[stringLength - i - 1] = str[i];
    }
    strcpy(str, temp);
    free(temp);
}

void i2cReceiveEvent(int howMany) 
{
    (void)howMany;
    while (Wire.available()) {
        int readChar{doI2CRead()};
        if (readChar != 0) {
            size_t stringLength{strlen(i2cRead)};
            if (stringLength >= SERIAL_READ_MAX) {
                break;
            }
            i2cRead[stringLength] = (char)readChar;
            i2cRead[stringLength + 1] = '\0';
        }
    }
}

void i2cRequestEvent()
{
    /*
    if (strlen(i2cWrite) > 0) {
        doI2CSlaveWrite(i2cWrite, strlen(i2cRead));
        memset(i2cWrite, '\0', SERIAL_READ_MAX);
    }
    */
}

int doSerialRead()
{
    int readChar{Serial.read()};
    if (isPrintable((char)(readChar))) {
        return readChar;
    } else {
        return 0;
    }
}

int doI2CRead()
{
    int readChar{Wire.read()};
    if (isPrintable((char)(readChar))) {
        return readChar;
    } else {
        return 0;
    }
}

void doI2CSlaveWrite(const char *str, size_t stringLength)
{
    for (size_t i = 0; i < stringLength; i++) {
        Wire.write(str[i]);
    }
    Wire.write(LINE_ENDING);
}

void doSerialWrite(const char *str, size_t stringLength)
{
    for (size_t i = 0; i < stringLength; i++) {
        Serial.write(str[i]);
    }
}

