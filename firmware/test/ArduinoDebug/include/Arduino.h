#ifndef ARDUINO_H
#define ARDUINO_H

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#include <iostream>

void analogWrite(int pinNumber, int state);
int analogRead(int pinNumber);
void digitalWrite(int pinNumber, bool state);
bool digitalRead(int pinNumber);
void pinMode(int pinNumber, int type);


#endif // ARDUINO_H
