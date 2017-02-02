#include <Arduino.h>
#include <i2cmasterserialport.h>
#include <hardwareserialport.h>

#include <string.h>
#include <stdlib.h>

#define SERIAL_READ_MAX 255
#define SERIAL_BAUD_RATE 115200L
#define SERIAL_TIMEOUT 250
#define SLAVE_NUMBER 8
#define I2C_TIMEOUT 250
const char *LINE_ENDING{"\r"};

ByteStream *serialPort;
ByteStream *i2cSerialPort;


int main()
{
    arduinoInit();
    serialPort = new HardwareSerialPort{&Serial, 0, 1, SERIAL_BAUD_RATE, SERIAL_TIMEOUT, true, LINE_ENDING};
    i2cSerialPort = new I2CMasterSerialPort{&Wire, SLAVE_NUMBER, I2C_TIMEOUT, true, LINE_ENDING};
    while (true) {
        if (serialPort->available()) {
            char serialRead[SERIAL_READ_MAX];
            int amountRead{serialPort->readLine(serialRead, SERIAL_READ_MAX)};
            if (amountRead > 0) {
                *serialPort << "Sending string '" << serialRead << "' over i2c" << LINE_ENDING;
                //i2cSerialPort->println(serialRead);
            }
        }
        if (i2cSerialPort->available()) {
            char i2cRead[SERIAL_READ_MAX];
            int amountRead{i2cSerialPort->readLine(i2cRead, SERIAL_READ_MAX)};
            if (amountRead > 0) {
                *serialPort << "Received string '" << i2cRead << "' over i2c" << LINE_ENDING;
            }
        }
        doImAliveBlink();
        if (serialEventRun) serialEventRun();
    }
    delete serialPort;
    delete i2cSerialPort;
    return 0;
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
