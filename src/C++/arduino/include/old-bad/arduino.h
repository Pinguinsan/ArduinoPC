#ifndef TJLUTILS_ARDUINO_H
#define TJLUTILS_ARDUINO_H

#include <list>
#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <algorithm>

#include "serialport.h"
#include "generalutilities.h"
#include "eventtimer.h"
#include "tstream.h"

enum class ArduinoType { UNO, NANO, MEGA };
enum IOType { DIGITAL_INPUT, DIGITAL_OUTPUT, ANALOG_INPUT, ANALOG_OUTPUT, DIGITAL_INPUT_PULLUP, UNSPECIFIED };
enum class CanMaskType { POSITIVE, NEGATIVE, ALL };
enum IOStatus { OPERATION_SUCCESS, OPERATION_FAILURE };
enum IOState { PIN_NUMBER, STATE, RETURN_CODE };
enum ArduinoTypeEnum { RETURN_STATE, OPERATION_RESULT };
enum IOReportEnum { IO_PIN_NUMBER, IO_TYPE, IO_STATE };
enum CanIOStatus { MESSAGE_ID, BYTE_0, BYTE_1, BYTE_2, BYTE_3, BYTE_4, BYTE_5, BYTE_6, BYTE_7 , CAN_IO_OPERATION_RESULT};
enum CanEnabledStatus { CAN_RETURN_STATE, CAN_OPERATION_RESULT };
enum ADThresholdReq { AD_RETURN_STATE, AD_OPERATION_RESULT };
enum CanMask { CAN_MASK_RETURN_STATE, CAN_MASK_OPERATION_RESULT };

class LinMessage;
class CanMessage;
class ArduinoUno;
class ArduinoNano;
class ArduinoMega;
class ProtectedSerialPort;
class GPIO;
class IOReport;
class SerialReport;
class CanReport;

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW 0x0
#endif

class Arduino
{
public:
    static std::shared_ptr<Arduino> makeArduino(std::shared_ptr<SerialPort> serialPort);

    std::pair<IOStatus, std::string> getArduinoType(int serialPortIndex);
    static std::pair<IOStatus, int> getAnalogToDigitalThreshold(std::shared_ptr<SerialPort> serialPort);


    std::pair<IOStatus, CanMessage> canListen(int screenIndex, double delay = Arduino::BLUETOOTH_SERIAL_SEND_DELAY);
    static std::vector<std::string> genericIOTask(const std::string &stringToSend, const std::string &header, int serialPortIndex, double delay = static_cast<double>(Arduino::BLUETOOTH_SERIAL_SEND_DELAY));
    static std::vector<std::string> genericIOTask(const std::string &stringToSend, const std::string &header, std::shared_ptr<SerialPort> serialPort, double delay = static_cast<double>(Arduino::BLUETOOTH_SERIAL_SEND_DELAY));
    static std::vector<std::string> genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, int serialPortIndex, double delay = static_cast<double>(Arduino::BLUETOOTH_SERIAL_SEND_DELAY));
    static std::vector<std::string> genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, std::shared_ptr<SerialPort> serialPort, double delay = static_cast<double>(Arduino::BLUETOOTH_SERIAL_SEND_DELAY));

    std::pair<IOStatus, bool> digitalRead(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, bool> digitalWrite(int pinNumber, bool state, int serialPortIndex);
    std::pair<IOStatus, double> analogRead(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, int> analogReadRaw(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, double> analogWrite(int pinNumber, double state, int serialPortIndex);
    std::pair<IOStatus, int> analogWriteRaw(int pinNumber, int state, int serialPortIndex);
    std::pair<IOStatus, bool> softDigitalRead(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, double> softAnalogRead(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, int> softAnalogReadRaw(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, IOType> pinMode(int pinNumber, IOType ioType, int serialPortIndex);
    std::pair<IOStatus, IOType> currentPinMode(int pinNumber, int serialPortIndex);
    std::pair<IOStatus, int> changeAnalogToDigitalThreshold(int threshold, int serialPortIndex);
    std::pair<IOStatus, uint32_t> addCanMask(CanMaskType canMaskType, const std::string &mask, int serialPortIndex);
    std::pair<IOStatus, uint32_t> removeCanMask(CanMaskType canMaskType, const std::string &mask, int serialPortIndex);
    std::pair<IOStatus, bool> removeAllCanMasks(CanMaskType canMaskType, int serialPortIndex);
    std::pair<IOStatus, CanMessage> canWrite(const CanMessage &message, int serialPortIndex);

    void initializeIO();
    void assignPinsAndIdentifiers();
    int parseAnalogPin(const std::string &pinAlias) const;
    std::string analogPinFromNumber(int pinNumber) const;



    IOReport ioReportRequest(int serialPortIndex);
    SerialReport serialReportRequest(int serialPortIndex, const std::string &delimiter = "");
    CanReport canReportRequest(int serialPortIndex);
    void writeRawString(const std::string &str, int serialPortIndex);
    std::string readRawString(int serialPortIndex);
    void flushRX(int serialPortIndex);
    void flushTX(int serialPortIndex);
    void flushRXTX(int serialPortIndex);
    void flushTXRX(int serialPortIndex);
    std::pair<IOStatus, bool> canAutoUpdate(bool state, int serialPortIndex);
    std::pair<IOStatus, bool> initializeCanBus(int serialPortIndex);
    std::pair<IOStatus, CanMessage> canRead(int serialPortIndex);


    bool parseToDigitalState(const std::string &state) const;
    double parseToAnalogState(const std::string &state) const;
    int parseToAnalogStateRaw(const std::string &state) const;
    bool isValidDigitalStateIdentifier(const std::string &state) const;
    bool isValidAnalogStateIdentifier(const std::string &state) const;
    bool isValidAnalogRawStateIdentifier(const std::string &state) const;

    std::pair<IOStatus, bool> digitalRead(const std::string &pinAlias);
    std::pair<IOStatus, bool> digitalWrite(const std::string &pinAlias, bool state);
    std::pair<IOStatus, double> analogRead(const std::string &pinAlias);
    std::pair<IOStatus, int> analogReadRaw(const std::string &pinAlias);
    std::pair<IOStatus, double> analogWrite(const std::string &pinAlias, double state);
    std::pair<IOStatus, int> analogWriteRaw(const std::string &pinAlias, int state);
    std::pair<IOStatus, bool> softDigitalRead(const std::string &pinAlias);
    std::pair<IOStatus, double> softAnalogRead(const std::string &pinAlias);
    std::pair<IOStatus, int> softAnalogReadRaw(const std::string &pinAlias);
    std::pair<IOStatus, IOType> pinMode(const std::string &pinAlias, IOType ioType);
    std::pair<IOStatus, IOType> currentPinMode(const std::string &pinAlias);

    std::pair<IOStatus, bool> digitalRead(const GPIO &gpio);
    std::pair<IOStatus, bool> digitalWrite(const GPIO &gpio, bool state);
    std::pair<IOStatus, double> analogRead(const GPIO &gpio);
    std::pair<IOStatus, int> analogReadRaw(const GPIO &gpio);
    std::pair<IOStatus, double> analogWrite(const GPIO &gpio, double state);
    std::pair<IOStatus, int> analogWriteRaw(const GPIO &gpio, int state);
    std::pair<IOStatus, bool> softDigitalRead(const GPIO &gpio);
    std::pair<IOStatus, double> softAnalogRead(const GPIO &gpio);
    std::pair<IOStatus, int> softAnalogReadRaw(const GPIO &gpio);
    std::pair<IOStatus, IOType> pinMode(GPIO &gpio, IOType ioType);
    std::pair<IOStatus, IOType> currentPinMode(const GPIO &gpio);
    void writeRawString(const std::string &str);
    std::string readRawString();
    void flushRX();
    void flushTX();
    void flushRXTX();
    void flushTXRX();
    std::string serialPortName() const;
    
    void assignAliasesFromMap(const std::map<int, std::string> aliasesMap);
    void assignIOTypesFromMap(const std::map<int, std::string> ioTypesMap);
    void assignInitialStatesFromMap(const std::map<int, std::string> initialStatesMap);

    std::pair<IOStatus, bool> digitalRead(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, bool> digitalWrite(std::shared_ptr<GPIO> gpioPtr, bool state);
    std::pair<IOStatus, double> analogRead(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, int> analogReadRaw(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, double> analogWrite(std::shared_ptr<GPIO> gpioPtr, double state);
    std::pair<IOStatus, int> analogWriteRaw(std::shared_ptr<GPIO> gpioPtr, int state);
    std::pair<IOStatus, bool> softDigitalRead(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, double> softAnalogRead(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, int> softAnalogReadRaw(std::shared_ptr<GPIO> gpioPtr);
    std::pair<IOStatus, IOType> pinMode(std::shared_ptr<GPIO> gpioPtr, IOType ioType);
    std::pair<IOStatus, IOType> currentPinMode(std::shared_ptr<GPIO> gpioPtr);

    std::string firmwareVersion();
    std::pair<bool, bool> canCapability();
    std::shared_ptr<SerialPort> serialPort() const;
    bool isValidAnalogPinIdentifier(const std::string &state) const;
    void eraseCanPin();
    bool isBluetooth() const;

    std::shared_ptr<GPIO> gpioPinByPinAlias(const std::string &pinAlias) const;
    std::shared_ptr<GPIO> gpioPinByPinNumber(int pinNumber) const;
    std::string gpioAliasByPinNumber(int number) const;
    void assignIOTypes();

    bool isValidDigitalOutputPin(int pinNumber) const;
    bool isValidDigitalInputPin(int pinNumber) const;
    bool isValidAnalogOutputPin(int pinNumber) const;
    bool isValidAnalogInputPin(int pinNumber) const;

    void setFirmwareVersion(const std::string &firmwareVersion);
    void setCanCapability(const std::pair<bool, bool> &capability);
    void setAnalogToDigitalThreshold(int state);
    int analogToDigitalThreshold();
    bool setPinAlias(int pinNumber, const std::string &newAlias);
    bool setPinAlias(const GPIO &gpio, const std::string &newAlias);
    bool setPinAlias(std::shared_ptr<GPIO> gpio, const std::string &newAlias);

    bool confirmValidAlias(const std::pair<int, std::string> &alias) const;
    bool confirmValidIOType(const std::pair<int, std::string> &io) const;
    bool confirmValidStates(const std::pair<int, std::string> &alias) const;

    std::set<int> AVAILABLE_ANALOG_PINS() const;
    std::set<int> AVAILABLE_PWM_PINS() const;
    std::set<int> AVAILABLE_PINS() const;
    int NUMBER_OF_DIGITAL_PINS() const;
    ArduinoType arduinoType() const;
    std::string identifier() const;
    std::string longName() const;
    int numberOfSerialPorts() const;

    static int staticParseAnalogPin(ArduinoType arduinoType, const std::string &pinAlias);
    static std::string staticAnalogPinFromNumber(ArduinoType arduinoType, int pinNumber);
    static unsigned int SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
    static std::pair<IOStatus, std::string> getArduinoType(std::shared_ptr<SerialPort> serialPort);
    static std::pair<IOStatus, std::string> getFirmwareVersion(std::shared_ptr<SerialPort> serialPort);
    static std::pair<IOStatus, bool> getCanCapability(std::shared_ptr<SerialPort> serialPort);
    static double analogToVoltage(int state);
    static int voltageToAnalog(double state);
    static std::string parseIOType(IOType ioType);
    static IOType parseIOTypeFromString(const std::string &ioType);
    static double bluetoothSendDelayMultiplier;

    static bool isBluetooth(int serialPortIndex);
    static bool isBluetooth(const std::string &name);
    static bool isBluetooth(std::shared_ptr<SerialPort> serialPort);

    static const BaudRate FIRMWARE_BAUD;
    static const DataBits FIRMWARE_DATA_BITS;
    static const StopBits FIRMWARE_STOP_BITS;
    static const Parity FIRMWARE_PARITY;
    static std::shared_ptr<SerialPort> serialPortAtIndex(unsigned int serialPortIndex);
    static std::shared_ptr<std::mutex> ioMutexAtIndex(unsigned int ioMutexIndex);
    int addSerialPort(std::shared_ptr<SerialPort> serialPort);

    static const unsigned int CAN_READ_BLANK_RETURN_SIZE;
    static const unsigned int REMOVE_CAN_MASKS_RETURN_SIZE;
    static const unsigned int IO_STATE_RETURN_SIZE;
    static const unsigned int ARDUINO_TYPE_RETURN_SIZE;
    static const unsigned int PIN_TYPE_RETURN_SIZE;
    static const unsigned int IO_REPORT_RETURN_SIZE;
    static const unsigned int CAN_ID_WIDTH;
    static const unsigned int CAN_BYTE_WIDTH;
    static const int OPERATION_FAILURE;
    static const int OPERATION_SUCCESS;
    static const int INVALID_PIN;
    static const int STATE_FAILURE;
    static const int ANALOG_MAX;
    static const int CAN_BUS_PIN;
    static const unsigned int RAW_CAN_MESSAGE_SIZE;
    static const unsigned char CAN_MESSAGE_LENGTH;
    static const unsigned char CAN_FRAME;
    static const double VOLTAGE_MAX;
    static const int IO_TRY_COUNT;
    static const unsigned int CAN_READ_RETURN_SIZE;
    static const unsigned int CAN_WRITE_RETURN_SIZE;
    static const unsigned int CAN_BUS_ENABLED_RETURN_SIZE;
    static const unsigned int CAN_AUTO_UPDATE_RETURN_SIZE;
    static const unsigned int A_TO_D_THRESHOLD_RETURN_SIZE;
    static const unsigned int CAN_INIT_RETURN_SIZE;
    static const unsigned int ADD_CAN_MASK_RETURN_SIZE;
    static const unsigned int REMOVE_CAN_MASK_RETURN_SIZE;
    static const unsigned int RETURN_SIZE_HIGH_LIMIT;
    static const double BOOTLOADER_BOOT_TIME;
    static const double SERIAL_TIMEOUT;
    static const int BLUETOOTH_RETRY_COUNT;
    static const double BLUETOOTH_SERIAL_SEND_DELAY;
    static const double DEFAULT_BLUETOOTH_SEND_DELAY_MULTIPLIER;
    static const unsigned int SERIAL_REPORT_REQUEST_TIME_LIMIT;
    static const unsigned int SERIAL_REPORT_OVERALL_TIME_LIMIT;
    static const double ANALOG_TO_VOLTAGE_SCALE_FACTOR;

    static const char *GENERIC_FAILED_STRING;
    static const char *GENERIC_SUCCESS_STRING;
    static const char *USING_ALIAS_STRING;
    static const char *SETTING_IO_TYPE_STRING;
    static const char *USING_INITIAL_STATE_STRING;
    static const char *FOR_PIN_NUMBER_STRING;
    static const char *ELIPSES_STRING;

private:
    Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort);
    Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, bool canCapability);
    Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, const std::string &firmwareVersion);
    Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, const std::string &firmwareVersion, bool canCapability);    

    std::map<std::string, std::shared_ptr<GPIO>> m_gpioPinsAlias;
    std::map<int, std::string> m_gpioPinIterationAliasMap;
    std::map<int, std::shared_ptr<GPIO>> m_gpioPins;
    ArduinoType m_arduinoType;
    int m_serialPortIndex;
    std::string m_identifier;
    std::string m_longName;
    std::string m_firmwareVersion;
    std::pair<bool, bool> m_canCapability;
    std::string m_canPinAlias;
    std::set<int> m_availablePins;
    std::set<int> m_availablePwmPins;
    std::set<int> m_availableAnalogPins;
    int m_numberOfDigitalPins;
    int m_analogToDigitalThreshold;

    static std::vector<ProtectedSerialPort> s_serialPorts;
    static unsigned  int s_SERIAL_PORT_TRY_COUNT_HIGH_LIMIT;

    static const char *HEARTBEAT_HEADER;
    static const char *IO_REPORT_HEADER;
    static const char *IO_REPORT_END_HEADER;
    static const char *INVALID_HEADER;
    static const char *DIGITAL_READ_HEADER;
    static const char *ANALOG_READ_HEADER;
    static const char *DIGITAL_WRITE_HEADER;
    static const char *ANALOG_WRITE_HEADER;
    static const char *SOFT_DIGITAL_READ_HEADER;
    static const char *SOFT_ANALOG_READ_HEADER;
    static const char *PIN_TYPE_HEADER;
    static const char *PIN_TYPE_CHANGE_HEADER;
    static const char *ARDUINO_TYPE_HEADER;
    static const char *CAN_BUS_ENABLED_HEADER;
    static const char *FIRMWARE_VERSION_HEADER;
    static const char *DIGITAL_INPUT_IDENTIFIER;
    static const char *DIGITAL_OUTPUT_IDENTIFIER;
    static const char *ANALOG_INPUT_IDENTIFIER;
    static const char *ANALOG_OUTPUT_IDENTIFIER;
    static const char *DIGITAL_INPUT_PULLUP_IDENTIFIER;
    static const char *OPERATION_FAILURE_STRING;
    static const char *ADD_POSITIVE_CAN_MASK_HEADER;
    static const char *ADD_NEGATIVE_CAN_MASK_HEADER;
    static const char *REMOVE_POSITIVE_CAN_MASK_HEADER;
    static const char *REMOVE_NEGATIVE_CAN_MASK_HEADER;
    static const char *REMOVE_ALL_POSITIVE_CAN_MASKS_HEADER;
    static const char *REMOVE_ALL_NEGATIVE_CAN_MASKS_HEADER;
    static const char *REMOVE_ALL_CAN_MASKS_HEADER;

    static const char *UNO_A0_STRING;
    static const char *UNO_A1_STRING;
    static const char *UNO_A2_STRING;
    static const char *UNO_A3_STRING;
    static const char *UNO_A4_STRING;
    static const char *UNO_A5_STRING;
    static const char *UNO_A0_EQUIVALENT_STRING;
    static const char *UNO_A1_EQUIVALENT_STRING;
    static const char *UNO_A2_EQUIVALENT_STRING;
    static const char *UNO_A3_EQUIVALENT_STRING;
    static const char *UNO_A4_EQUIVALENT_STRING;
    static const char *UNO_A5_EQUIVALENT_STRING;
    static const char *NANO_A0_STRING;
    static const char *NANO_A1_STRING;
    static const char *NANO_A2_STRING;
    static const char *NANO_A3_STRING;
    static const char *NANO_A4_STRING;
    static const char *NANO_A5_STRING;
    static const char *NANO_A6_STRING;
    static const char *NANO_A7_STRING;
    static const char *NANO_A0_EQUIVALENT_STRING;
    static const char *NANO_A1_EQUIVALENT_STRING;
    static const char *NANO_A2_EQUIVALENT_STRING;
    static const char *NANO_A3_EQUIVALENT_STRING;
    static const char *NANO_A4_EQUIVALENT_STRING;
    static const char *NANO_A5_EQUIVALENT_STRING;
    static const char *NANO_A6_EQUIVALENT_STRING;
    static const char *NANO_A7_EQUIVALENT_STRING;
    static const char *MEGA_A0_STRING;
    static const char *MEGA_A1_STRING;
    static const char *MEGA_A2_STRING;
    static const char *MEGA_A3_STRING;
    static const char *MEGA_A4_STRING;
    static const char *MEGA_A5_STRING;
    static const char *MEGA_A6_STRING;
    static const char *MEGA_A7_STRING;
    static const char *MEGA_A8_STRING;
    static const char *MEGA_A9_STRING;
    static const char *MEGA_A10_STRING;
    static const char *MEGA_A11_STRING;
    static const char *MEGA_A12_STRING;
    static const char *MEGA_A13_STRING;
    static const char *MEGA_A14_STRING;
    static const char *MEGA_A15_STRING;
    static const char *MEGA_A0_EQUIVALENT_STRING;
    static const char *MEGA_A1_EQUIVALENT_STRING;
    static const char *MEGA_A2_EQUIVALENT_STRING;
    static const char *MEGA_A3_EQUIVALENT_STRING;
    static const char *MEGA_A4_EQUIVALENT_STRING;
    static const char *MEGA_A5_EQUIVALENT_STRING;
    static const char *MEGA_A6_EQUIVALENT_STRING;
    static const char *MEGA_A7_EQUIVALENT_STRING;
    static const char *MEGA_A8_EQUIVALENT_STRING;
    static const char *MEGA_A9_EQUIVALENT_STRING;
    static const char *MEGA_A10_EQUIVALENT_STRING;
    static const char *MEGA_A11_EQUIVALENT_STRING;
    static const char *MEGA_A12_EQUIVALENT_STRING;
    static const char *MEGA_A13_EQUIVALENT_STRING;
    static const char *MEGA_A14_EQUIVALENT_STRING;
    static const char *MEGA_A15_EQUIVALENT_STRING;
    static const char *UNO_INVALID_ANALOG_STRING_TAIL_STRING;
    static const char *UNO_INVALID_ANALOG_INT_TAIL_STRING;
    static const char *NANO_INVALID_ANALOG_STRING_TAIL_STRING;
    static const char *NANO_INVALID_ANALOG_INT_TAIL_STRING;
    static const char *MEGA_INVALID_ANALOG_STRING_TAIL_STRING;
    static const char *MEGA_INVALID_ANALOG_INT_TAIL_STRING;

    static const char *INVALID_ANALOG_PIN_BASE_STRING;
    static const char *UNSPECIFIED_IO_TYPE_TO_PARSE_IO_TYPE_STRING;
    static const char *INVALID_IO_TYPE_BASE_STRING;
    static const char *INVALID_IO_TYPE_TAIL_STRING;
    static const char *INVALID_STATE_TO_PARSE_TO_DIGITAL_STATE_STRING;
    static const char *INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_STRING;
    static const char *INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_RAW_STRING;
    static const char *FIRMWARE_VERSION_UNKNOWN_STRING;
    static const char *FIRMWARE_VERSION_BASE_STRING;
    static const char *SERIAL_PORT_INVALID_INDEX_STRING;
    static const char *IO_MUTEX_INVALID_INDEX_STRING;
    static const char *IO_REPORT_INVALID_DATA_STRING;
    static const char *CAN_REPORT_INVALID_DATA_STRING;
    static const char *CAN_EMPTY_READ_SUCCESS_STRING;

    static const char *INVALID_PIN_ALIAS_STRING;
    static const char *INVALID_PIN_NUMBER_STRING;
    static const char *PASSED_TO_DIGITAL_READ_STRING_TAIL_STRING;
    static const char *PASSED_TO_DIGITAL_WRITE_STRING_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_READ_STRING_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_READ_RAW_STRING_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_WRITE_STRING_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_WRITE_RAW_STRING_TAIL_STRING;
    static const char *PASSED_TO_SOFT_DIGITAL_READ_STRING_TAIL_STRING;
    static const char *PASSED_TO_SOFT_ANALOG_READ_STRING_TAIL_STRING;
    static const char *PASSED_TO_SOFT_ANALOG_READ_RAW_STRING_TAIL_STRING;
    static const char *PASSED_TO_PIN_MODE_STRING_TAIL_STRING;
    static const char *PASSED_TO_CURRENT_PIN_MODE_STRING_TAIL_STRING;
    static const char *PASSED_TO_DIGITAL_READ_GPIO_TAIL_STRING;
    static const char *PASSED_TO_DIGITAL_WRITE_GPIO_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_READ_GPIO_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_READ_RAW_GPIO_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_WRITE_GPIO_TAIL_STRING;
    static const char *PASSED_TO_ANALOG_WRITE_RAW_GPIO_TAIL_STRING;
    static const char *PASSED_TO_SOFT_DIGITAL_READ_GPIO_TAIL_STRING;
    static const char *PASSED_TO_SOFT_ANALOG_READ_GPIO_TAIL_STRING;
    static const char *PASSED_TO_SOFT_ANALOG_READ_RAW_GPIO_TAIL_STRING;
    static const char *PASSED_TO_PIN_MODE_GPIO_TAIL_STRING;
    static const char *PASSED_TO_CURRENT_PIN_MODE_GPIO_TAIL_STRING;
    static const char *INVALID_GPIO_PIN_STRING;
    static const char *NULL_GPIO_PTR_TO_DIGITAL_READ_STRING;
    static const char *NULL_GPIO_PTR_TO_DIGITAL_WRITE_STRING;
    static const char *NULL_GPIO_PTR_TO_ANALOG_READ_STRING;
    static const char *NULL_GPIO_PTR_TO_ANALOG_READ_RAW_STRING;
    static const char *NULL_GPIO_PTR_TO_ANALOG_WRITE_STRING;
    static const char *NULL_GPIO_PTR_TO_ANALOG_WRITE_RAW_STRING;
    static const char *NULL_GPIO_PTR_TO_SOFT_DIGITAL_READ_STRING;
    static const char *NULL_GPIO_PTR_TO_SOFT_ANALOG_READ_STRING;
    static const char *NULL_GPIO_PTR_TO_SOFT_ANALOG_READ_RAW_STRING;
    static const char *NULL_GPIO_PTR_TO_PIN_MODE_STRING;
    static const char *NULL_GPIO_PTR_TO_CURRENT_PIN_MODE_STRING;
    static const char *CAN_INIT_HEADER;
    static const char *CAN_READ_HEADER;
    static const char *CAN_WRITE_HEADER;
    static const char *CAN_LIVE_UPDATE_HEADER;
    static const char *CAN_CLEAR_MESSAGE_HEADER;
    static const char *CHANGE_A_TO_D_THRESHOLD_HEADER;
    static const char *CURRENT_A_TO_D_THRESHOLD_HEADER;
    static const char *BLUETOOTH_SERIAL_IDENTIFIER;
    static const std::vector<const char *> VALID_DIGITAL_STATE_IDENTIFIERS;
    static const std::vector<char> VALID_ANALOG_STATE_IDENTIFIERS;
    static const std::vector<const char *> DIGITAL_STATE_HIGH_IDENTIFIERS;
    static const std::vector<const char *> DIGITAL_STATE_LOW_IDENTIFIERS;
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
        m_ioType{ioType}
    {

    }

    int pinNumber() const
    {
        return this->m_pinNumber;
    }

    IOType ioType() const
    {
        return this->m_ioType;
    }

    void setIOType(IOType ioType)
    {
        this->m_ioType = ioType;
    }

    friend bool operator==(const GPIO &lhs, const GPIO &rhs)
    {
        return (lhs.pinNumber() == rhs.pinNumber());
    }

private:
    int m_pinNumber;
    IOType m_ioType;
};

class ProtectedSerialPort
{
public:
    ProtectedSerialPort(std::shared_ptr<SerialPort> serialPort) :
        m_serialPort{serialPort},
        m_ioMutex{std::make_shared<std::mutex>()}
    {

    }

    ~ProtectedSerialPort() {  }
    std::shared_ptr<SerialPort> serialPort() const
    {
        return this->m_serialPort;
    }

    std::shared_ptr<std::mutex> ioMutex() const
    {
        return this->m_ioMutex;
    }

private:
    std::shared_ptr<SerialPort> m_serialPort;
    std::shared_ptr<std::mutex> m_ioMutex;
};


class IOReport
{
public:
    void addDigitalInputResult(const std::pair<int, bool> &result)
    {
        this->m_digitalInputResults.emplace_back(result);
    }

    void addDigitalOutputResult(const std::pair<int, bool> &result)
    {
        this->m_digitalOutputResults.emplace_back(result);
    }

    void addAnalogInputResult(const std::pair<int, int> &result)
    {
        this->m_analogInputResults.emplace_back(result);
    }

    void addAnalogOutputResult(const std::pair<int, int> &result)
    {
        this->m_analogOutputResults.emplace_back(result);
    }

    std::vector<std::pair<int, bool>> digitalInputResults() const
    {
        return this->m_digitalInputResults;
    }

    std::vector<std::pair<int, bool>> digitalOutputResults() const
    {
        return this->m_digitalOutputResults;
    }

    std::vector<std::pair<int, int>> analogInputResults() const
    {
        return this->m_analogInputResults;
    }

    std::vector<std::pair<int, int>> analogOutputResults() const
    {
        return this->m_analogOutputResults;
    }

private:
    std::vector<std::pair<int, bool>> m_digitalInputResults;
    std::vector<std::pair<int, bool>> m_digitalOutputResults;
    std::vector<std::pair<int, int>> m_analogInputResults;
    std::vector<std::pair<int, int>> m_analogOutputResults;
};

class CanReport
{
public:
    void addCanMessageResult(const CanMessage &result)
    {
        this->m_canMessageResults.emplace_back(result);
    }

    std::vector<CanMessage> canMessageResults() const
    {
        return this->m_canMessageResults;
    }

private:
    std::vector<CanMessage> m_canMessageResults;
};

class SerialReport
{
public:
    void addSerialResult(const std::string &result)
    {
        this->m_serialResults.emplace_back(result);
    }

    std::vector<std::string> serialResults() const
    {
        return this->m_serialResults;
    }

private:
    std::vector<std::string> m_serialResults;
};


class CanDataPacket
{
public:
    CanDataPacket() :
        m_dataPacket{std::vector<unsigned char>{0, 0, 0, 0, 0, 0, 0, 0}}
    {

    }

    CanDataPacket(unsigned char first, unsigned char second, 
                                unsigned char third, unsigned char fourth, 
                                unsigned char fifth, unsigned char sixth, 
                                unsigned char seventh, unsigned char eighth) :
        m_dataPacket{std::vector<unsigned char>{first, second, third, fourth, 
                                                fifth, sixth, seventh, eighth}}
    {

    }                                            

    CanDataPacket(const std::vector<unsigned char> &dataPacket) :
        m_dataPacket(dataPacket)
    {

    }

    CanDataPacket(const CanDataPacket &dataPacket) :
        m_dataPacket{dataPacket.m_dataPacket}
    {

    }

    void setDataPacket(const std::vector<unsigned char> &dataPacket)
    {
        this->m_dataPacket = dataPacket;
    }

    void setDataPacket(unsigned char first, unsigned char second, 
                                    unsigned char third, unsigned char fourth, 
                                    unsigned char fifth, unsigned char sixth, 
                                    unsigned char seventh, unsigned char eighth)
    {
        this->m_dataPacket = std::vector<unsigned char>{first, second, third, fourth,
                                                        fifth, sixth, seventh, eighth};
    }                         

    bool setNthByte(int index, unsigned char nth)
    {
        if ((index >= 0) && (index < 8)) {
            this->m_dataPacket.at(index) = nth;
            return true;
        } else {
            return false;
        }
    }

    unsigned char nthByte(int index)
    {
        return this->m_dataPacket.at(index);
    }

    void toBasicArray(unsigned char copyArray[8]) const
    {
        int i = 0;
        for (auto &it : this->m_dataPacket) {
            copyArray[i++] = it;
        }
    }

    std::vector<unsigned char> dataPacket() const
    {
        return m_dataPacket;
    }

    CanDataPacket combineDataPackets(const CanDataPacket &first, const CanDataPacket &second)
    {
        std::vector<unsigned char> constructorArg;
        std::vector<unsigned char> firstCopy = first.dataPacket();
        std::vector<unsigned char> secondCopy = second.dataPacket();
        std::vector<unsigned char>::const_iterator firstIter = firstCopy.begin();
        std::vector<unsigned char>::const_iterator secondIter = secondCopy.begin();
        while (firstIter != firstCopy.end()) {
            constructorArg.push_back((*firstIter++) | (*secondIter++));
        }
        return CanDataPacket(constructorArg);
    }

    friend bool operator==(const CanDataPacket &lhs, const CanDataPacket &rhs)
    {
        if (lhs.m_dataPacket.size() != rhs.m_dataPacket.size()) {
            return false;
        }
        for (unsigned int i = 0; i < lhs.m_dataPacket.size(); i++) {
            if (lhs.m_dataPacket.at(i) != rhs.m_dataPacket.at(i)) {
                return false;
            }
        }
        return true;
    }


private:
    std::vector<unsigned char> m_dataPacket;

};

class CanMessage
{
public:
    CanMessage(uint32_t id, uint8_t frame, uint8_t length, const CanDataPacket &dataPacket) :
        m_id{id},
        m_frame{frame},
        m_length{length},
        m_dataPacket{dataPacket}
    {

    }

    CanMessage() :
        m_id{0},
        m_frame{0},
        m_length{0},
        m_dataPacket{CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0}}
    {

    }

    void setID(uint32_t id)
    {
        this->m_id = id;
    }

    void setFrame(uint8_t frame)
    {
        this->m_frame = frame;
    }

    void setLength(uint8_t length)
    {
        this->m_length = length;
    }

    void setDataPacket(const CanDataPacket &dataPacket)
    {
        this->m_dataPacket = dataPacket;
    }

    uint32_t id() const
    {
        return this->m_id;
    }

    uint8_t frame() const
    {
        return this->m_frame;
    }

    uint8_t length() const
    {
        return this->m_length;
    }

    unsigned char nthDataPacketByte(int index)
    {
        if ((index > 8) || (index < 0)) {
            throw std::runtime_error(NTH_DATA_PACKET_BYTE_INDEX_OUT_OF_RANGE_STRING + std::to_string(index));
        }
        return this->m_dataPacket.nthByte(index);
    }

    bool setDataPacketNthByte(int index, unsigned char nth)
    {
        return this->m_dataPacket.setNthByte(index, nth);
    }

    CanDataPacket dataPacket() const
    {
        return this->m_dataPacket;
    }

    std::string toString() const
    {
        using namespace GeneralUtilities;
        if ((this->m_id == 0) &&
            (this->m_frame == 0) &&
            (this->m_length == 0) &&
            (this->m_dataPacket == CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
            return "";
        }
        std::string returnString{"0x" + toFixedWidth(toHexString(this->m_id), CAN_ID_WIDTH) + ":"};
        unsigned int i{0};
        for (auto &it : this->m_dataPacket.dataPacket()) {
            returnString += "0x" + toFixedWidth(toHexString(it), CAN_BYTE_WIDTH);
            if (i++ != (this->m_dataPacket.dataPacket().size()-1)) {
                returnString += ":";
            }
        }
        return returnString;
    }

    std::string toPrettyString() const
    {
        using namespace GeneralUtilities;
        if ((this->m_id == 0) &&
            (this->m_frame == 0) &&
            (this->m_length == 0) &&
            (this->m_dataPacket == CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
            return "";
        }
        std::string returnString{"0x" + toFixedWidth(toHexString(this->m_id), CAN_ID_WIDTH) + " : "};
        unsigned int i{0};
        for (auto &it : this->m_dataPacket.dataPacket()) {
            returnString += "0x" + toFixedWidth(toHexString(it), CAN_BYTE_WIDTH);
            if (i++ != (this->m_dataPacket.dataPacket().size()-1)) {
                returnString += " : ";
            }
        }
        return returnString;
    }

    static uint32_t parseCanID(const std::string &str)
    {
        using namespace GeneralUtilities;
        return hexStringToUInt(str);
    }

    static uint8_t parseCanByte(const std::string &str)
    {
        using namespace GeneralUtilities;
        return hexStringToUChar(str);
    }

    static CanMessage parseCanMessage(const std::string &str)
    {
        using namespace GeneralUtilities;
        std::vector<std::string> rawMsg{parseToVector(str, ':')};
        if (rawMsg.size() != CAN_MESSAGE_SIZE) {
            return CanMessage{};
        }
        CanMessage returnMessage;
        int i{0};
        returnMessage.setFrame(CAN_FRAME);
        returnMessage.setLength(CAN_MESSAGE_LENGTH);
        for (auto &it : rawMsg) {
            if (i++ == 0) {
                returnMessage.setID(parseCanID(it));
            } else {
                returnMessage.setDataPacketNthByte(i-2, parseCanByte(it));
            }
        }
        return returnMessage;
    }

    friend bool operator==(const CanMessage &lhs, const CanMessage &rhs)
    {
        if (lhs.m_dataPacket.dataPacket().size() != lhs.m_dataPacket.dataPacket().size()) {
            return false;
        }
        for (unsigned int i = 0; i < lhs.m_dataPacket.dataPacket().size(); i++) {
            if (lhs.m_dataPacket.dataPacket().at(i) != rhs.m_dataPacket.dataPacket().at(i)) {
                return false;
            }
        }
        return ((lhs.m_id == rhs.m_id) &&
                (lhs.m_frame == rhs.m_frame) &&
                (lhs.m_length == rhs.m_length));
    }

    static const int CAN_BYTE_WIDTH;
    static const int CAN_ID_WIDTH;
    static const unsigned int CAN_MESSAGE_SIZE;
    static const unsigned char CAN_FRAME;
    static const unsigned char CAN_MESSAGE_LENGTH;


private:
    uint32_t m_id;
    uint8_t m_frame;
    uint8_t m_length;
    CanDataPacket m_dataPacket;

    static const char *NTH_DATA_PACKET_BYTE_INDEX_OUT_OF_RANGE_STRING;
};


class LinDataPacket
{
public:
    LinDataPacket() :
        m_dataPacket{std::vector<unsigned char>{0, 0, 0, 0, 0, 0, 0, 0}}
    {

    }

    LinDataPacket(unsigned char first, unsigned char second, 
                                unsigned char third, unsigned char fourth, 
                                unsigned char fifth, unsigned char sixth, 
                                unsigned char seventh, unsigned char eighth) :
        m_dataPacket{std::vector<unsigned char>{first, second, third, fourth, 
                                                fifth, sixth, seventh, eighth}}
    {

    }                                            

    LinDataPacket(const std::vector<unsigned char> &dataPacket) :
        m_dataPacket(dataPacket)
    {

    }

    LinDataPacket(const LinDataPacket &dataPacket) :
        m_dataPacket{dataPacket.m_dataPacket}
    {

    }

    void setDataPacket(const std::vector<unsigned char> &dataPacket)
    {
        this->m_dataPacket = dataPacket;
    }

    void setDataPacket(unsigned char first, unsigned char second, 
                                    unsigned char third, unsigned char fourth, 
                                    unsigned char fifth, unsigned char sixth, 
                                    unsigned char seventh, unsigned char eighth)
    {
        this->m_dataPacket = std::vector<unsigned char>{first, second, third, fourth,
                                                        fifth, sixth, seventh, eighth};
    }                         

    bool setNthByte(int index, unsigned char nth)
    {
        if ((index >= 0) && (index < 8)) {
            this->m_dataPacket.at(index) = nth;
            return true;
        } else {
            return false;
        }
    }

    unsigned char nthByte(int index)
    {
        return this->m_dataPacket.at(index);
    }

    void toBasicArray(unsigned char copyArray[8]) const
    {
        int i = 0;
        for (auto &it : this->m_dataPacket) {
            copyArray[i++] = it;
        }
    }

    std::vector<unsigned char> dataPacket() const
    {
        return m_dataPacket;
    }

    LinDataPacket combineDataPackets(const LinDataPacket &first, const LinDataPacket &second)
    {
        std::vector<unsigned char> constructorArg;
        std::vector<unsigned char> firstCopy = first.dataPacket();
        std::vector<unsigned char> secondCopy = second.dataPacket();
        std::vector<unsigned char>::const_iterator firstIter = firstCopy.begin();
        std::vector<unsigned char>::const_iterator secondIter = secondCopy.begin();
        while (firstIter != firstCopy.end()) {
            constructorArg.push_back((*firstIter++) | (*secondIter++));
        }
        return LinDataPacket(constructorArg);
    }

    friend bool operator==(const LinDataPacket &lhs, const LinDataPacket &rhs)
    {
        if (lhs.m_dataPacket.size() != rhs.m_dataPacket.size()) {
            return false;
        }
        for (unsigned int i = 0; i < lhs.m_dataPacket.size(); i++) {
            if (lhs.m_dataPacket.at(i) != rhs.m_dataPacket.at(i)) {
                return false;
            }
        }
        return true;
    }


private:
    std::vector<unsigned char> m_dataPacket;

};

class LinMessage
{
public:
    LinMessage(uint32_t id, uint8_t frame, uint8_t length, const LinDataPacket &dataPacket) :
        m_id{id},
        m_frame{frame},
        m_length{length},
        m_dataPacket{dataPacket}
    {

    }

    LinMessage() :
        m_id{0},
        m_frame{0},
        m_length{0},
        m_dataPacket{LinDataPacket{0, 0, 0, 0, 0, 0, 0, 0}}
    {

    }

    void setID(uint32_t id)
    {
        this->m_id = id;
    }

    void setFrame(uint8_t frame)
    {
        this->m_frame = frame;
    }

    void setLength(uint8_t length)
    {
        this->m_length = length;
    }

    void setDataPacket(const LinDataPacket &dataPacket)
    {
        this->m_dataPacket = dataPacket;
    }

    uint32_t id() const
    {
        return this->m_id;
    }

    uint8_t frame() const
    {
        return this->m_frame;
    }

    uint8_t length() const
    {
        return this->m_length;
    }

    unsigned char nthDataPacketByte(int index)
    {
        if ((index > 8) || (index < 0)) {
            throw std::runtime_error(NTH_DATA_PACKET_BYTE_INDEX_OUT_OF_RANGE_STRING + std::to_string(index));
        }
        return this->m_dataPacket.nthByte(index);
    }

    bool setDataPacketNthByte(int index, unsigned char nth)
    {
        return this->m_dataPacket.setNthByte(index, nth);
    }

    LinDataPacket dataPacket() const
    {
        return this->m_dataPacket;
    }

    std::string toString() const
    {
        using namespace GeneralUtilities;
        if ((this->m_id == 0) &&
            (this->m_frame == 0) &&
            (this->m_length == 0) &&
            (this->m_dataPacket == LinDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
            return "";
        }
        std::string returnString{"0x" + toFixedWidth(toHexString(this->m_id), LIN_ID_WIDTH) + ":"};
        unsigned int i{0};
        for (auto &it : this->m_dataPacket.dataPacket()) {
            returnString += "0x" + toFixedWidth(toHexString(it), LIN_BYTE_WIDTH);
            if (i++ != (this->m_dataPacket.dataPacket().size()-1)) {
                returnString += ":";
            }
        }
        return returnString;
    }

    std::string toPrettyString() const
    {
        using namespace GeneralUtilities;
        if ((this->m_id == 0) &&
            (this->m_frame == 0) &&
            (this->m_length == 0) &&
            (this->m_dataPacket == LinDataPacket{0, 0, 0, 0, 0, 0, 0, 0})) {
            return "";
        }
        std::string returnString{"0x" + toFixedWidth(toHexString(this->m_id), LIN_ID_WIDTH) + " : "};
        unsigned int i{0};
        for (auto &it : this->m_dataPacket.dataPacket()) {
            returnString += "0x" + toFixedWidth(toHexString(it), LIN_BYTE_WIDTH);
            if (i++ != (this->m_dataPacket.dataPacket().size()-1)) {
                returnString += " : ";
            }
        }
        return returnString;
    }

    static uint32_t parseLinID(const std::string &str)
    {
        using namespace GeneralUtilities;
        return hexStringToUInt(str);
    }

    static uint8_t parseLinByte(const std::string &str)
    {
        using namespace GeneralUtilities;
        return hexStringToUChar(str);
    }

    static LinMessage parseLinMessage(const std::string &str)
    {
        using namespace GeneralUtilities;
        std::vector<std::string> rawMsg{parseToVector(str, ':')};
        if (rawMsg.size() != LIN_MESSAGE_SIZE) {
            return LinMessage{};
        }
        LinMessage returnMessage;
        int i{0};
        returnMessage.setFrame(LIN_FRAME);
        returnMessage.setLength(LIN_MESSAGE_LENGTH);
        for (auto &it : rawMsg) {
            if (i++ == 0) {
                returnMessage.setID(parseLinID(it));
            } else {
                returnMessage.setDataPacketNthByte(i-2, parseLinByte(it));
            }
        }
        return returnMessage;
    }

    friend bool operator==(const LinMessage &lhs, const LinMessage &rhs)
    {
        if (lhs.m_dataPacket.dataPacket().size() != lhs.m_dataPacket.dataPacket().size()) {
            return false;
        }
        for (unsigned int i = 0; i < lhs.m_dataPacket.dataPacket().size(); i++) {
            if (lhs.m_dataPacket.dataPacket().at(i) != rhs.m_dataPacket.dataPacket().at(i)) {
                return false;
            }
        }
        return ((lhs.m_id == rhs.m_id) &&
                (lhs.m_frame == rhs.m_frame) &&
                (lhs.m_length == rhs.m_length));
    }

    static uint8_t calculateChecksum(const LinMessage &linMessage)
    {
        (void)linMessage;
        //TODO:
            /*
            Public Function CheckSumCalc1(ByVal v As String) As String
                Dim B As Integer
                Dim ABit As Integer
                Dim Rslt As Integer
                For x = 1 To v.Length Step 2
                    B = B + Val("&H" + Mid(v, x, 2))
                Next
                ABit = Int(B / 256)
                B = (B And &HFF) + ABit
                If B > 255 Then
                    ABit = B / 256
                    B = B + ABit
                End If
                Rslt = B Xor &HFF
                CheckSumCalc1 = Hex(Rslt)
            End Function
            */
        return 0;
    }

    static const int LIN_BYTE_WIDTH;
    static const int LIN_ID_WIDTH;
    static const unsigned int LIN_MESSAGE_SIZE;
    static const unsigned char LIN_FRAME;
    static const unsigned char LIN_MESSAGE_LENGTH;


private:
    uint32_t m_id;
    uint8_t m_frame;
    uint8_t m_length;
    LinDataPacket m_dataPacket;

    static const char *NTH_DATA_PACKET_BYTE_INDEX_OUT_OF_RANGE_STRING;
};

#endif //TJLUTILS_ARDUINO_H
