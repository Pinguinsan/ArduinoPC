#include "include/arduinoinit.h"
#include <Arduino.h>

int main() 
{
    arduinoInit();
    Serial.begin(9600);
    const int LED_PIN{1};
    pinMode(LED_PIN, OUTPUT);
    bool ledState{false};
    int timeout{1000};
    long long startTime{millis()};
    long long endTime{millis()};
    while (true) {

        if (Serial.available()) {
            String stringToPrint{""};
            do {
              int readChar{Serial.read()};
              if (readChar != 0) {
                  stringToPrint += static_cast<char>(readChar);
              } else {
                  break;
              }
            } while (readChar != 0);
            Serial.print(stringToPrint + " - ");
            Serial.println(rand());
        }
        endTime = millis();
        if ((endTime - startTime) >= timeout) {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
            startTime = millis();
        }
    }
    return 0;
}

