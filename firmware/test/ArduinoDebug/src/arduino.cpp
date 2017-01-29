#include "include/Arduino.h"

void analogWrite(int pinNumber, int state)
{
    std::cout << "pinNumber " << pinNumber << " was analogWritten to " << state << std::endl;
}

int analogRead(int pinNumber)
{
    std::cout << "pinNumber " << pinNumber << " was analogRead" << std::endl;
    return 0;
}

void digitalWrite(int pinNumber, bool state)
{
    std::cout << "pinNumber " << pinNumber << " was digitalWritten to " << state << std::endl;
}

bool digitalRead(int pinNumber)
{
    std::cout << "pinNumber " << pinNumber << " was digitalRead" << std::endl;
    return false;
}

void pinMode(int pinNumber, int type)
{
    std::cout << "pinNumber " << pinNumber << " was pinModed to " << type << std::endl;
}
