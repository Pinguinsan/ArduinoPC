#include "../include/gpio.h"

const int GPIO::ANALOG_MAX{1023};
int GPIO::s_analogToDigitalThreshold{510};

GPIO::GPIO(int pinNumber, IOType ioType) :
    m_pinNumber{pinNumber},
    m_ioType{ioType},
    m_logicState{false},
    m_analogState{0}
{
    setIOType(this->m_ioType);
}

void GPIO::setAnalogToDigitalThreshold(int threshold)
{
    using namespace FirmwareUtilities;
    if (threshold < 0) {
        threshold = 0;
    }
    GPIO::s_analogToDigitalThreshold = threshold;
}   

int GPIO::analogToDigitalThreshold()
{
    return GPIO::s_analogToDigitalThreshold;
}

void GPIO::setPinNumber(int pinNumber)
{
    this->m_pinNumber = pinNumber;
}

IOType GPIO::ioType() const
{
    return this->m_ioType;
}

void GPIO::setIOType(IOType ioType)
{
    this->m_ioType = ioType;
    if (this->m_ioType == IOType::DIGITAL_OUTPUT) {
        pinMode(this->m_pinNumber, OUTPUT);
    } else if (this->m_ioType == IOType::ANALOG_OUTPUT) {
        pinMode(this->m_pinNumber, OUTPUT);
    } else if (this->m_ioType == IOType::DIGITAL_INPUT) {
        pinMode(this->m_pinNumber, INPUT);
    } else if (this->m_ioType == IOType::ANALOG_INPUT) {
        pinMode(this->m_pinNumber, INPUT);
    } else if (this->m_ioType == IOType::DIGITAL_INPUT_PULLUP) {
        pinMode(this->m_pinNumber, INPUT_PULLUP);
    }
    this->m_analogState = 0;
    this->m_logicState = false;
}

bool GPIO::g_digitalRead()
{
    if ((this->m_ioType != IOType::DIGITAL_INPUT) && (this->m_ioType != IOType::DIGITAL_INPUT_PULLUP)) {
        setIOType(IOType::DIGITAL_INPUT);
    }
    return (this->m_logicState = digitalRead(this->m_pinNumber));
}

bool GPIO::g_softDigitalRead()
{
    if (this->m_ioType == IOType::DIGITAL_OUTPUT) {
        return this->m_logicState;
    } else if (this->m_ioType == IOType::ANALOG_INPUT) {
        (void)this->g_analogRead();
        return (this->m_analogState >= GPIO::s_analogToDigitalThreshold);
    } else if (this->m_ioType == IOType::ANALOG_OUTPUT) {
        return (this->m_analogState >= GPIO::s_analogToDigitalThreshold); 
    } else {
        return g_digitalRead();
    }
    return false;
}


int GPIO::g_softAnalogRead()
{
    if ((this->m_ioType == IOType::ANALOG_OUTPUT) || (this->m_ioType == IOType::ANALOG_INPUT)) {
        return this->m_analogState;
    } else {
        if (this->m_logicState) {
            return 5;
        } else {
            return 0;
        }
    }
}

void GPIO::g_digitalWrite(bool logicState)
{
    if (this->m_ioType != IOType::DIGITAL_OUTPUT) {
        setIOType(IOType::DIGITAL_OUTPUT);
    }
    digitalWrite(this->m_pinNumber, logicState);
    this->m_logicState = logicState;
} 

int GPIO::g_analogRead()
{
    if (this->m_ioType != IOType::ANALOG_INPUT) {
        setIOType(IOType::ANALOG_INPUT);
    }
    return (this->m_analogState = analogRead(this->m_pinNumber));
}

void GPIO::g_analogWrite(int state)
{
    if ((this->m_ioType != IOType::DIGITAL_OUTPUT) && (this->m_ioType != IOType::ANALOG_OUTPUT)) {
        setIOType(IOType::ANALOG_OUTPUT);
    }
    if (state > ANALOG_MAX) {
        state = ANALOG_MAX;
    }
    this->m_analogState = state;
    analogWrite(this->m_pinNumber, this->m_analogState);
}


int GPIO::pinNumber() const
{
    return this->m_pinNumber;
}