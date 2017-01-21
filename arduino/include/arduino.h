#ifndef TJLUTILS_ARDUINO_H
#define TJLUTILS_ARDUINO_H

#include <string>
#include <utility>
#include <memory>
#include <future>
#include <algorithm>
#include <mutex>
#include <set>
#include <vector>
#include "serialport.h"

#include "generalutilities.h"
#include "eventtimer.h"
#include "tstream.h"


enum class ArduinoType { UNO, NANO, MEGA };
enum IOType { DIGITAL_INPUT, DIGITAL_OUTPUT, ANALOG_INPUT, ANALOG_OUTPUT, DIGITAL_INPUT_PULLUP, UNSPECIFIED };
enum IOStatus { OPERATION_SUCCESS, OPERATION_FAILURE };
enum IOState { PIN_NUMBER, STATE, RETURN_CODE };
enum ArduinoTypeEnum { RETURN_STATE, OPERATION_RESULT };
enum IOReportEnum { IO_PIN_NUMBER, IO_TYPE, IO_STATE };
enum CanEnabledStatus { CAN_RETURN_STATE, CAN_OPERATION_RESULT };
enum ADThresholdReq { AD_RETURN_STATE, AD_OPERATION_RESULT };

#ifndef HIGH
    #define HIGH 0x1
#endif

#ifndef LOW
    #define LOW 0x0
#endif


class ArduinoUno;
class ArduinoNano;
class ArduinoMega;
class GPIO;
class IOReport;
class SerialReport;

class Arduino
{
public:
    Arduino(ArduinoType arduinoType, std::shared_ptr<TStream> ioStream);
    std::pair<IOStatus, bool> digitalRead(int pinNumber);
    std::pair<IOStatus, bool> digitalWrite(int pinNumber, bool state);
    std::pair<IOStatus, std::vector<int>> digitalWriteAll(bool state);
    std::pair<IOStatus, double> analogRead(int pinNumber);
    std::pair<IOStatus, int> analogReadRaw(int pinNumber);
    std::pair<IOStatus, double> analogWrite(int pinNumber, double state);
    std::pair<IOStatus, int> analogWriteRaw(int pinNumber, int state);
    std::pair<IOStatus, bool> softDigitalRead(int pinNumber);
    std::pair<IOStatus, double> softAnalogRead(int pinNumber);
    std::pair<IOStatus, int> softAnalogReadRaw(int pinNumber);
    std::pair<IOStatus, IOType> pinMode(int pinNumber, IOType ioType);
    std::pair<IOStatus, IOType> currentPinMode(int pinNumber);
    std::pair<IOStatus, bool> canCapability();
    std::pair<IOStatus, std::string> firmwareVersion();
    std::pair<IOStatus, std::string> arduinoTypeString();
    std::pair<IOStatus, int> analogToDigitalThreshold();
    std::pair<IOStatus, int> setAnalogToDigitalThreshold(int threshold);
    SerialReport serialReportRequest(const std::string &delimiter);
    IOReport ioReportRequest();

    std::string serialPortName() const;

    std::set<int> AVAILABLE_ANALOG_PINS() const;
    std::set<int> AVAILABLE_PWM_PINS() const;
    std::set<int> AVAILABLE_PINS() const;
    int NUMBER_OF_DIGITAL_PINS() const;
    ArduinoType arduinoType() const;
    
    std::string identifier() const;
    std::string longName() const;

    void setIOTryCount(unsigned int ioTryCount);
    unsigned int ioTryCount() const;

    void setStreamSendDelay(unsigned int streamSendDelay);
    unsigned int streamSendDelay() const;

    void assignPinsAndIdentifiers();

    static const BaudRate FIRMWARE_BAUD_RATE;
    static const DataBits FIRMWARE_DATA_BITS;
    static const StopBits FIRMWARE_STOP_BITS;
    static const Parity FIRMWARE_PARITY;
    static const int ANALOG_MAX;
    static const double VOLTAGE_MAX;
    static const unsigned int DEFAULT_IO_TRY_COUNT;
    
private:
    std::map<int, std::shared_ptr<GPIO>> m_gpioPins;
    std::shared_ptr<TStream> m_ioStream;
    std::mutex m_ioMutex;
    ArduinoType m_arduinoType;
    std::string m_identifier;
    std::string m_longName;
    std::set<int> m_availablePins;
    std::set<int> m_availablePwmPins;
    std::set<int> m_availableAnalogPins;
    int m_numberOfDigitalPins;
    unsigned int m_streamSendDelay;
    unsigned int m_ioTryCount;

    bool isValidAnalogPinIdentifier(const std::string &state) const;
    bool isValidDigitalStateIdentifier(const std::string &state) const;
    bool isValidAnalogStateIdentifier(const std::string &state) const;
    bool isValidAnalogRawStateIdentifier(const std::string &state) const;

    bool isValidDigitalOutputPin(int pinNumber) const;
    bool isValidDigitalInputPin(int pinNumber) const;
    bool isValidAnalogOutputPin(int pinNumber) const;
    bool isValidAnalogInputPin(int pinNumber) const;

    std::vector<std::string> genericIOTask(const std::string &stringToSend, const std::string &header, double delay);
    std::vector<std::string> genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, double delay); 
};

class ArduinoUno
{
public:
    ArduinoUno() = delete;
    virtual void doStuff() = 0;
    static std::set<int> s_availableAnalogPins;
    static std::set<int> s_availablePwmPins;
    static std::set<int> s_availablePins;
    static const char *IDENTIFIER;
    static const char *LONG_NAME;
    static int s_numberOfDigitalPins;
};

class ArduinoNano
{
public:
    ArduinoNano() = delete;
    virtual void doStuff() = 0;
    static std::set<int> s_availableAnalogPins;
    static std::set<int> s_availablePwmPins;
    static std::set<int> s_availablePins;
    static const char *IDENTIFIER;
    static const char *LONG_NAME;
    static int s_numberOfDigitalPins;
};

class ArduinoMega
{
public:
    ArduinoMega() = delete;
    virtual void doStuff() = 0;
    static std::set<int> s_availableAnalogPins;
    static std::set<int> s_availablePwmPins;
    static std::set<int> s_availablePins;
    static const char *IDENTIFIER;
    static const char *LONG_NAME;
    static int s_numberOfDigitalPins;
};

class GPIO
{
public:
    GPIO(int pinNumber, IOType ioType) :
        m_pinNumber{pinNumber},
        m_ioType{ioType} { }
    int pinNumber() const { return this->m_pinNumber; }
    IOType ioType() const { return this->m_ioType; }
    void setIOType(IOType ioType) { this->m_ioType = ioType; }
    friend bool operator==(const GPIO &lhs, const GPIO &rhs) { return (lhs.pinNumber() == rhs.pinNumber()); }

private:
    int m_pinNumber;
    IOType m_ioType;
};

class IOReport
{
public:
    void addDigitalInputResult(const std::pair<int, bool> &result) { this->m_digitalInputResults.emplace_back(result); }
    void addDigitalOutputResult(const std::pair<int, bool> &result) { this->m_digitalOutputResults.emplace_back(result); }
    void addAnalogInputResult(const std::pair<int, int> &result) { this->m_analogInputResults.emplace_back(result); }
    void addAnalogOutputResult(const std::pair<int, int> &result) { this->m_analogOutputResults.emplace_back(result); }

    std::vector<std::pair<int, bool>> digitalInputResults() const { return this->m_digitalInputResults; }
    std::vector<std::pair<int, bool>> digitalOutputResults() const { return this->m_digitalOutputResults; }
    std::vector<std::pair<int, int>> analogInputResults() const { return this->m_analogInputResults; }
    std::vector<std::pair<int, int>> analogOutputResults() const { return this->m_analogOutputResults; }

private:
    std::vector<std::pair<int, bool>> m_digitalInputResults;
    std::vector<std::pair<int, bool>> m_digitalOutputResults;
    std::vector<std::pair<int, int>> m_analogInputResults;
    std::vector<std::pair<int, int>> m_analogOutputResults;
};

class SerialReport
{
public:
    void addSerialResult(const std::string &result) { this->m_serialResults.emplace_back(result); }
    std::vector<std::string> serialResults() const { return this->m_serialResults; }

private:
    std::vector<std::string> m_serialResults;
};

const unsigned int IO_STATE_RETURN_SIZE{3};
const unsigned int ARDUINO_TYPE_RETURN_SIZE{2};
const unsigned int PIN_TYPE_RETURN_SIZE{3};
const unsigned int IO_REPORT_RETURN_SIZE{3};
const unsigned int A_TO_D_THRESHOLD_RETURN_SIZE{2};
const unsigned int RETURN_SIZE_HIGH_LIMIT{1000};
const int STATE_FAILURE{-1};
const int INVALID_PIN{-1};
const double SERIAL_TIMEOUT{50};
const int BLUETOOTH_RETRY_COUNT{10};
const double BOOTLOADER_BOOT_TIME{2000};
const double BLUETOOTH_SERIAL_SEND_DELAY{100};
const int DEFAULT_IO_STREAM_SEND_DELAY{20};
const double ANALOG_TO_VOLTAGE_SCALE_FACTOR{0.0049};
const double DEFAULT_BLUETOOTH_SEND_DELAY_MULTIPLIER{4.8};

const unsigned int CAN_BUS_ENABLED_RETURN_SIZE{2};
const unsigned int DIGITAL_WRITE_ALL_MINIMIM_RETURN_SIZE{2};
const unsigned int SERIAL_REPORT_REQUEST_TIME_LIMIT{50};
const unsigned int SERIAL_REPORT_OVERALL_TIME_LIMIT{50};

unsigned int SERIAL_PORT_TRY_COUNT_HIGH_LIMIT{4};
double bluetoothSendDelayMultiplier{DEFAULT_BLUETOOTH_SEND_DELAY_MULTIPLIER};

const int IO_TRY_COUNT{4};

const std::vector<const char *> VALID_DIGITAL_STATE_IDENTIFIERS{"1", "0", "HIGH", "high", "LOW", "low", "FALSE", "false", "TRUE", "true"};
const std::vector<char> VALID_ANALOG_STATE_IDENTIFIERS{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'};
const std::vector<const char *> DIGITAL_STATE_HIGH_IDENTIFIERS{"1", "high", "true", "on"};
const std::vector<const char *> DIGITAL_STATE_LOW_IDENTIFIERS{"0", "low", "false", "off"};
const char * const INVALID_HEADER{"{invalid"};
const char * const DIGITAL_READ_HEADER{"{dread"};
const char * const ANALOG_READ_HEADER{"{aread"};
const char * const DIGITAL_WRITE_HEADER{"{dwrite"};
const char * const DIGITAL_WRITE_ALL_HEADER{"{dwriteall"};
const char * const ANALOG_WRITE_HEADER{"{awrite"};
const char * const SOFT_DIGITAL_READ_HEADER{"{sdread"};
const char * const SOFT_ANALOG_READ_HEADER{"{saread"};
const char * const PIN_TYPE_HEADER{"{ptype"};
const char * const PIN_TYPE_CHANGE_HEADER{"{ptchange"};
const char * const ARDUINO_TYPE_HEADER{"{ardtype"};
const char * const CAN_BUS_ENABLED_HEADER{"{canbus"};
const char * const FIRMWARE_VERSION_HEADER{"{version"};
const char * const HEARTBEAT_HEADER{"{heartbeat"};
const char * const IO_REPORT_HEADER{"{ioreport"};
const char * const IO_REPORT_END_HEADER{"{ioreportend"};
const char * const CHANGE_A_TO_D_THRESHOLD_HEADER{"{atodchange"};
const char * const CURRENT_A_TO_D_THRESHOLD_HEADER{"{atodthresh"};
const char * const UNO_A0_STRING{"A0"};
const char * const UNO_A1_STRING{"A1"};
const char * const UNO_A2_STRING{"A2"};
const char * const UNO_A3_STRING{"A3"};
const char * const UNO_A4_STRING{"A4"};
const char * const UNO_A5_STRING{"A5"};
const char * const UNO_A0_EQUIVALENT_STRING{"14"};
const char * const UNO_A1_EQUIVALENT_STRING{"15"};
const char * const UNO_A2_EQUIVALENT_STRING{"16"};
const char * const UNO_A3_EQUIVALENT_STRING{"17"};
const char * const UNO_A4_EQUIVALENT_STRING{"18"};
const char * const UNO_A5_EQUIVALENT_STRING{"19"};

const char * const NANO_A0_STRING{"A0"};
const char * const NANO_A1_STRING{"A1"};
const char * const NANO_A2_STRING{"A2"};
const char * const NANO_A3_STRING{"A3"};
const char * const NANO_A4_STRING{"A4"};
const char * const NANO_A5_STRING{"A5"};
const char * const NANO_A6_STRING{"A6"};
const char * const NANO_A7_STRING{"A7"};
const char * const NANO_A0_EQUIVALENT_STRING{"14"};
const char * const NANO_A1_EQUIVALENT_STRING{"15"};
const char * const NANO_A2_EQUIVALENT_STRING{"16"};
const char * const NANO_A3_EQUIVALENT_STRING{"17"};
const char * const NANO_A4_EQUIVALENT_STRING{"18"};
const char * const NANO_A5_EQUIVALENT_STRING{"19"};
const char * const NANO_A6_EQUIVALENT_STRING{"20"};
const char * const NANO_A7_EQUIVALENT_STRING{"21"};

const char * const MEGA_A0_STRING{"A0"};
const char * const MEGA_A1_STRING{"A1"};
const char * const MEGA_A2_STRING{"A2"};
const char * const MEGA_A3_STRING{"A3"};
const char * const MEGA_A4_STRING{"A4"};
const char * const MEGA_A5_STRING{"A5"};
const char * const MEGA_A6_STRING{"A6"};
const char * const MEGA_A7_STRING{"A7"};
const char * const MEGA_A8_STRING{"A8"};
const char * const MEGA_A9_STRING{"A9"};
const char * const MEGA_A10_STRING{"A10"};
const char * const MEGA_A11_STRING{"A11"};
const char * const MEGA_A12_STRING{"A12"};
const char * const MEGA_A13_STRING{"A13"};
const char * const MEGA_A14_STRING{"A14"};
const char * const MEGA_A15_STRING{"A15"};
const char * const MEGA_A0_EQUIVALENT_STRING{"54"};
const char * const MEGA_A1_EQUIVALENT_STRING{"55"};
const char * const MEGA_A2_EQUIVALENT_STRING{"56"};
const char * const MEGA_A3_EQUIVALENT_STRING{"57"};
const char * const MEGA_A4_EQUIVALENT_STRING{"58"};
const char * const MEGA_A5_EQUIVALENT_STRING{"59"};
const char * const MEGA_A6_EQUIVALENT_STRING{"60"};
const char * const MEGA_A7_EQUIVALENT_STRING{"61"};
const char * const MEGA_A8_EQUIVALENT_STRING{"62"};
const char * const MEGA_A9_EQUIVALENT_STRING{"63"};
const char * const MEGA_A10_EQUIVALENT_STRING{"64"};
const char * const MEGA_A11_EQUIVALENT_STRING{"65"};
const char * const MEGA_A12_EQUIVALENT_STRING{"66"};
const char * const MEGA_A13_EQUIVALENT_STRING{"67"};
const char * const MEGA_A14_EQUIVALENT_STRING{"68"};
const char * const MEGA_A15_EQUIVALENT_STRING{"69"};
const char * const DIGITAL_INPUT_IDENTIFIER{"din"};
const char * const DIGITAL_OUTPUT_IDENTIFIER{"dout"};
const char * const ANALOG_INPUT_IDENTIFIER{"ain"};
const char * const ANALOG_OUTPUT_IDENTIFIER{"aout"};
const char * const DIGITAL_INPUT_PULLUP_IDENTIFIER{"dinpup"};
const char * const OPERATION_FAILURE_STRING{"-1"};
const char * const IO_REPORT_INVALID_DATA_STRING{"Arduino::ioReportRequest(int) timed out or received invalid data"};

const char * const BLUETOOTH_SERIAL_IDENTIFIER{"rfcomm"};
const char * const INVALID_PIN_ALIAS_STRING{"Invalid pin alias: "};
const char * const INVALID_GPIO_PIN_STRING{"Invalid gpio pin: "};
const char * const INVALID_PIN_NUMBER_STRING{"Invalid pin number: "};
const char * const INVALID_ANALOG_PIN_BASE_STRING{"Invalid analog pin: "};
const char * const INVALID_ANALOG_PIN_TAIL_STRING{" passed to Arduino::analogPinFromNumber(int)"};
const char * const INVALID_ANALOG_PIN_TO_INT_TAIL_STRING{" passed to Arduino::parseAnalogPin(ArduinoType, const std::string &"};
const char * const UNSPECIFIED_IO_TYPE_TO_PARSE_IO_TYPE_STRING{"Unspecified IOType passed to Arduino::parseIOType(IOType)"};
const char * const INVALID_IO_TYPE_BASE_STRING{"Invalid IOType: "};
const char * const INVALID_IO_TYPE_TAIL_STRING{" passed to Arduino::parseIOTypeFromString(const std::string &)"};
const char * const INVALID_STATE_TO_PARSE_TO_DIGITAL_STATE_STRING{"Invalid state passed to Arduino::parseToDigitalState(const std::string &): "};
const char * const INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_STRING{"Invalid state passed to Arduino::parseToAnalogState(const std::string &): "};
const char * const INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_RAW_STRING{"Invalid state passed to Arduino::parseToAnalogStateRaw(const std::string &): "};
const char * const FIRMWARE_VERSION_UNKNOWN_STRING{" unknown"};
const char * const FIRMWARE_VERSION_BASE_STRING{"firmware version "};
const char * const IO_TRY_COUNT_TOO_LOW_STRING{"Invalid  IO try count passed to Arduino::setIOTryCount(unsigned int), value must be greater than 0 ("};

int voltageToAnalog(double state)
{
    return (state / ANALOG_TO_VOLTAGE_SCALE_FACTOR);
}

double analogToVoltage(int state)
{
    return state * ANALOG_TO_VOLTAGE_SCALE_FACTOR;
}

bool parseToDigitalState(const std::string &state)
{
    std::string copyString{state};
    std::transform(copyString.begin(), copyString.end(), copyString.begin(), ::tolower);
    for (auto &it : DIGITAL_STATE_HIGH_IDENTIFIERS) {
        if (copyString == static_cast<std::string>(it)) {
            return HIGH;
        }
    }

    for (auto &it : DIGITAL_STATE_LOW_IDENTIFIERS) {
        if (copyString == static_cast<std::string>(it)) {
            return LOW;
        }
    }
    throw std::runtime_error(INVALID_STATE_TO_PARSE_TO_DIGITAL_STATE_STRING + state);
}

double parseToAnalogState(const std::string &state)
{
    try {
        double temp{GeneralUtilities::decStringToDouble(state.c_str())};
        if (temp > Arduino::VOLTAGE_MAX) {
            temp = Arduino::VOLTAGE_MAX;
        }
        return temp;
    } catch (std::exception &e) {
        throw std::runtime_error(INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_STRING + state);
    }
}

int parseToAnalogStateRaw(const std::string &state)
{
    try {
        int temp{GeneralUtilities::decStringToInt(state.c_str())};
        if (temp > Arduino::ANALOG_MAX) {
            temp = Arduino::ANALOG_MAX;
        }
        return temp;
    } catch (std::exception &e) {
        throw std::runtime_error(INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_RAW_STRING + state);
    }
}

bool isValidDigitalStateIdentifier(const std::string &state)
{
    std::string copyString{state};
    std::transform(copyString.begin(), copyString.end(), copyString.begin(), ::tolower);
    for (auto &it : DIGITAL_STATE_HIGH_IDENTIFIERS) {
        if (copyString == static_cast<std::string>(it)) {
            return true;
        }
    }

    for (auto &it : DIGITAL_STATE_LOW_IDENTIFIERS) {
        if (copyString == static_cast<std::string>(it)) {
            return true;
        }
    }
    return false;
}

bool isValidAnalogStateIdentifier(const std::string &state)
{
    for (auto &it : VALID_ANALOG_STATE_IDENTIFIERS) {
        if (state == std::string(1, it)) {
            return true;
        }
    }
    return false;
}

bool isValidAnalogRawStateIdentifier(const std::string &state)
{
    bool match{false};
    for (auto &it : state) {
        match = false;
        for (auto &charIt : VALID_ANALOG_STATE_IDENTIFIERS) {
            if ((it == charIt) && (charIt != '.')) {
                match = true;
            }
        }
        if (!match) {
            return false;
        }
    }
    return true;
}

std::string parseIOType(IOType ioType)
{
    if (ioType == IOType::DIGITAL_INPUT) {
        return DIGITAL_INPUT_IDENTIFIER;
    } else if (ioType == IOType::DIGITAL_OUTPUT) {
        return DIGITAL_OUTPUT_IDENTIFIER;
    } else if (ioType == IOType::ANALOG_INPUT) {
        return ANALOG_INPUT_IDENTIFIER;
    } else if (ioType == IOType::ANALOG_OUTPUT) {
        return ANALOG_OUTPUT_IDENTIFIER;
    } else if (ioType == IOType::DIGITAL_INPUT_PULLUP) {
        return DIGITAL_INPUT_PULLUP_IDENTIFIER;
    } else {
        throw std::runtime_error(UNSPECIFIED_IO_TYPE_TO_PARSE_IO_TYPE_STRING);
    }
}

IOType parseIOTypeFromString(const std::string &ioType)
{
    if (ioType == DIGITAL_INPUT_IDENTIFIER) {
        return IOType::DIGITAL_INPUT;
    } else if (ioType == DIGITAL_OUTPUT_IDENTIFIER) {
        return IOType::DIGITAL_OUTPUT;
    } else if (ioType == ANALOG_INPUT_IDENTIFIER) {
        return IOType::ANALOG_INPUT;
    } else if (ioType == ANALOG_OUTPUT_IDENTIFIER) {
        return IOType::ANALOG_OUTPUT;
    } else if (ioType == DIGITAL_INPUT_PULLUP_IDENTIFIER) {
        return IOType::DIGITAL_INPUT_PULLUP;
    } else {
        throw std::runtime_error(INVALID_IO_TYPE_BASE_STRING + ioType + INVALID_IO_TYPE_TAIL_STRING);
    }
}



int parseAnalogPin(ArduinoType arduinoType, const std::string &pinAlias)
{
    if (arduinoType == ArduinoType::UNO) {
        if ((pinAlias == UNO_A0_STRING) || (pinAlias == UNO_A0_EQUIVALENT_STRING)) {
            return 14;
        } else if ((pinAlias == UNO_A1_STRING) || (pinAlias == UNO_A1_EQUIVALENT_STRING)) {
            return 15;
        } else if ((pinAlias == UNO_A2_STRING) || (pinAlias == UNO_A2_EQUIVALENT_STRING)) {
            return 16;
        } else if ((pinAlias == UNO_A3_STRING) || (pinAlias == UNO_A3_EQUIVALENT_STRING)) {
            return 17;
        } else if ((pinAlias == UNO_A4_STRING) || (pinAlias == UNO_A4_EQUIVALENT_STRING)) {
            return 18;
        } else if ((pinAlias == UNO_A5_STRING) || (pinAlias == UNO_A5_EQUIVALENT_STRING)) {
            return 19;
        } else {
            throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + INVALID_ANALOG_PIN_TO_INT_TAIL_STRING);
        }

    } else if (arduinoType == ArduinoType::NANO) {
        if ((pinAlias == NANO_A0_STRING) || (pinAlias == NANO_A0_EQUIVALENT_STRING)) {
            return 14;
        } else if ((pinAlias == NANO_A1_STRING) || (pinAlias == NANO_A1_EQUIVALENT_STRING)) {
            return 15;
        } else if ((pinAlias == NANO_A2_STRING) || (pinAlias == NANO_A2_EQUIVALENT_STRING)) {
            return 16;
        } else if ((pinAlias == NANO_A3_STRING) || (pinAlias == NANO_A3_EQUIVALENT_STRING)) {
            return 17;
        } else if ((pinAlias == NANO_A4_STRING) || (pinAlias == NANO_A4_EQUIVALENT_STRING)) {
            return 18;
        } else if ((pinAlias == NANO_A5_STRING) || (pinAlias == NANO_A5_EQUIVALENT_STRING)) {
            return 19;
        } else if ((pinAlias == NANO_A6_STRING) || (pinAlias == NANO_A6_EQUIVALENT_STRING)) {
            return 20;
        } else if ((pinAlias == NANO_A7_STRING) || (pinAlias == NANO_A7_EQUIVALENT_STRING)) {
            return 21;
        } else {
            throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + INVALID_ANALOG_PIN_TO_INT_TAIL_STRING);
        }
    } else if (arduinoType == ArduinoType::MEGA) {
        if ((pinAlias == MEGA_A0_STRING) || (pinAlias == MEGA_A0_EQUIVALENT_STRING)) {
            return 54;
        } else if ((pinAlias == MEGA_A1_STRING) || (pinAlias == MEGA_A1_EQUIVALENT_STRING)) {
            return 55;
        } else if ((pinAlias == MEGA_A2_STRING) || (pinAlias == MEGA_A2_EQUIVALENT_STRING)) {
            return 56;
        } else if ((pinAlias == MEGA_A3_STRING) || (pinAlias == MEGA_A3_EQUIVALENT_STRING)) {
            return 57;
        } else if ((pinAlias == MEGA_A4_STRING) || (pinAlias == MEGA_A4_EQUIVALENT_STRING)) {
            return 58;
        } else if ((pinAlias == MEGA_A5_STRING) || (pinAlias == MEGA_A5_EQUIVALENT_STRING)) {
            return 59;
        } else if ((pinAlias == MEGA_A6_STRING) || (pinAlias == MEGA_A6_EQUIVALENT_STRING)) {
            return 60;
        } else if ((pinAlias == MEGA_A7_STRING) || (pinAlias == MEGA_A7_EQUIVALENT_STRING)) {
            return 61;
        } else if ((pinAlias == MEGA_A8_STRING) || (pinAlias == MEGA_A8_EQUIVALENT_STRING)) {
            return 62;
        } else if ((pinAlias == MEGA_A9_STRING) || (pinAlias == MEGA_A9_EQUIVALENT_STRING)) {
            return 63;
        } else if ((pinAlias == MEGA_A10_STRING) || (pinAlias == MEGA_A10_EQUIVALENT_STRING)) {
            return 64;
        } else if ((pinAlias == MEGA_A11_STRING) || (pinAlias == MEGA_A11_EQUIVALENT_STRING)) {
            return 65;
        } else if ((pinAlias == MEGA_A12_STRING) || (pinAlias == MEGA_A12_EQUIVALENT_STRING)) {
            return 66;
        } else if ((pinAlias == MEGA_A13_STRING) || (pinAlias == MEGA_A13_EQUIVALENT_STRING)) {
            return 67;
        } else if ((pinAlias == MEGA_A14_STRING) || (pinAlias == MEGA_A14_EQUIVALENT_STRING)) {
            return 68;
        } else if ((pinAlias == MEGA_A15_STRING) || (pinAlias == MEGA_A15_EQUIVALENT_STRING)) {
            return 69;
        } else {
            throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + INVALID_ANALOG_PIN_TO_INT_TAIL_STRING);
        }
    } else {
        throw std::runtime_error("");
    }
}

std::string analogPinFromNumber(ArduinoType arduinoType, int pinNumber)
{
    if (arduinoType == ArduinoType::UNO) {
        if (pinNumber == 14) {
            return UNO_A0_STRING;
        } else if (pinNumber == 15) {
            return UNO_A1_STRING;
        } else if (pinNumber == 16) {
            return UNO_A2_STRING;
        } else if (pinNumber == 17) {
            return UNO_A3_STRING;
        } else if (pinNumber == 18) {
            return UNO_A4_STRING;
        } else if (pinNumber == 19) {
            return UNO_A5_STRING;
        } else {
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + INVALID_ANALOG_PIN_TAIL_STRING);
        }
    } else if (arduinoType == ArduinoType::NANO) {
        if (pinNumber == 14) {
            return NANO_A0_STRING;
        } else if (pinNumber == 15) {
            return NANO_A1_STRING;
        } else if (pinNumber == 16) {
            return NANO_A2_STRING;
        } else if (pinNumber == 17) {
            return NANO_A3_STRING;
        } else if (pinNumber == 18) {
            return NANO_A4_STRING;
        } else if (pinNumber == 19) {
            return NANO_A5_STRING;
        } else if (pinNumber == 20) {
            return NANO_A6_STRING;
        } else if (pinNumber == 21) {
            return NANO_A7_STRING;
        } else {
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + INVALID_ANALOG_PIN_TAIL_STRING);
        }
    } else if (arduinoType == ArduinoType::MEGA) {
        if (pinNumber == 54) {
            return MEGA_A0_STRING;
        } else if (pinNumber == 55) {
            return MEGA_A1_STRING;
        } else if (pinNumber == 56) {
            return MEGA_A2_STRING;
        } else if (pinNumber == 57) {
            return MEGA_A3_STRING;
        } else if (pinNumber == 58) {
            return MEGA_A4_STRING;
        } else if (pinNumber == 59) {
            return MEGA_A5_STRING;
        } else if (pinNumber == 60) {
            return MEGA_A6_STRING;
        } else if (pinNumber == 61) {
            return MEGA_A7_STRING;
        } else if (pinNumber == 62) {
            return MEGA_A8_STRING;
        } else if (pinNumber == 63) {
            return MEGA_A9_STRING;
        } else if (pinNumber == 64) {
            return MEGA_A10_STRING;
        } else if (pinNumber == 65) {
            return MEGA_A11_STRING;
        } else if (pinNumber == 66) {
            return MEGA_A12_STRING;
        } else if (pinNumber == 67) {
            return MEGA_A13_STRING;
        } else if (pinNumber == 68) {
            return MEGA_A14_STRING;
        } else if (pinNumber == 69) {
            return MEGA_A15_STRING;
        } else {
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + INVALID_ANALOG_PIN_TAIL_STRING);
        }
    } else {
        throw std::runtime_error("");
    }
}

#endif
