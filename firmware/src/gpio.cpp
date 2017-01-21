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

int GPIO::getIOAgnosticState()
{
    if ((this->m_ioType == IOType::DIGITAL_INPUT) || (this->m_ioType == IOType::DIGITAL_INPUT_PULLUP)) {
        return this->g_digitalRead();
    } else if (this->m_ioType == IOType::DIGITAL_OUTPUT) {
        return this->g_softDigitalRead();
    } else if (this->m_ioType == IOType::ANALOG_INPUT) {
        return this->g_analogRead();
    } else if (this->m_ioType == IOType::ANALOG_OUTPUT) {
        return this->g_softAnalogRead();
    } else {
        return 0;
    }
}

std::vector<unsigned char> GPIO::getEEPROMWritableState()
{
    return GPIO::toEEPROMWritableState(this->getIOAgnosticState());
}

std::vector<unsigned char> GPIO::toEEPROMWritableState(int longState)
{
    std::vector<unsigned char> result;
    result.reserve(5);
    int copyInt{longState};
    while (copyInt > 0) {
        if (copyInt > 254) {
            result.insert(result.begin(), 255);
        } else {
            result.insert(result.begin(), static_cast<unsigned char>(copyInt));
        }
        copyInt -= 255;
    }
    while (result.size() != 5) {
        result.insert(result.begin(), 0);
    }
    return result;
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
        pinModeFast(this->m_pinNumber, OUTPUT);
    } else if (this->m_ioType == IOType::ANALOG_OUTPUT) {
        pinModeFast(this->m_pinNumber, OUTPUT);
    } else if (this->m_ioType == IOType::DIGITAL_INPUT) {
        pinModeFast(this->m_pinNumber, INPUT);
    } else if (this->m_ioType == IOType::ANALOG_INPUT) {
        pinModeFast(this->m_pinNumber, INPUT);
    } else if (this->m_ioType == IOType::DIGITAL_INPUT_PULLUP) {
        pinModeFast(this->m_pinNumber, INPUT_PULLUP);
    }
    this->m_analogState = 0;
    this->m_logicState = false;
}

bool GPIO::g_digitalRead()
{
    if ((this->m_ioType != IOType::DIGITAL_INPUT) && (this->m_ioType != IOType::DIGITAL_INPUT_PULLUP)) {
        setIOType(IOType::DIGITAL_INPUT);
    }
    return (this->m_logicState = digitalReadFast(this->m_pinNumber));
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
    digitalWriteFast(this->m_pinNumber, logicState);
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