#include <Arduino.h>
#include <SoftwareSerial.h>
#include <MemoryFree.h>
#include <avr/pgmspace.h>
#include "include/arduinoinit.h"
#include "include/gpio.h"
#include "include/serialportinfo.h"
#include "include/firmwareutilities.h"
#include "include/arduinopcstrings.h"

/* 
 * Standard C++ headers
 * Special thanks to maniacbug for the
 * nice port of the c++ stdlib 
 * https://github.com/maniacbug/StandardCplusplus
 */
#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <serstream>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#if defined(__HAVE_CAN_BUS__)
    #include "include/mcp_can.h"
    #include "include/candatapacket.h"
    #include "include/canmessage.h"
#endif

using namespace ArduinoPCStrings;
using namespace FirmwareUtilities;

namespace std { ohserialstream cout{Serial}; }

#define ID_WIDTH 3
#define MESSAGE_WIDTH 2
#define MAXIMUM_SERIAL_READ_SIZE 175
#define PIN_OFFSET 2
#define NEXT_SERIAL_PORT_UNAVAILABLE -1
#define SERIAL_BAUD 115200L
#define SERIAL_TIMEOUT 1000

#define OPERATION_FAILURE -1
#define INVALID_PIN -1
#define STATE_FAILURE -1
#define OPERATION_SUCCESS 1
#define OPERATION_KIND_OF_SUCCESS 2
#define OPERATION_PIN_USED_BY_SERIAL_PORT 3
#define PIN_PLACEHOLDER 1
#define SOFT 1

void broadcastString(const std::string &str);
#if defined(__HAVE_CAN_BUS__)
    void printCanResult(const std::string &header, const std::string &str, int resultCode);
    void printCanResult(const std::string &header, const CanMessage &msg, int resultCode);
    void printBlankCanResult(const std::string &header, int resultCode); 
#endif

void handleSerialString(const std::string &str);
void handleSerialString(const char *str);

void digitalReadRequest(const std::string &str, bool soft);
void digitalWriteRequest(const std::string &str);
void digitalWriteAllRequest(const std::string &str);
void analogReadRequest(const std::string &str);
void analogWriteRequest(const std::string &str);
void softAnalogReadRequest(const std::string &str);
void addSoftwareSerialRequest(const std::string &str);
void addHardwareSerialRequest(const std::string &str);
void removeSoftwareSerialRequest(const std::string &str);
void removeSoftwareSerialRequest(const std::string &str);

void pinTypeRequest(const std::string &str);
void pinTypeChangeRequest(const std::string &str);

void changeAToDThresholdRequest(const std::string &str);
void currentAToDThresholdRequest();

void arduinoTypeRequest();
void firmwareVersionRequest();
void canBusEnabledRequest();
void ioReportRequest();

SerialPortInfo *getCurrentValidOutputStream();

bool isValidAnalogPinIdentifier(const std::string &str);
bool isValidPinIdentifier(const std::string &str);
bool isValidDigitalStateIdentifier(const std::string &str);
bool isValidAnalogStateIdentifier(const std::string &str);
bool isValidPwmPinIdentifier(const std::string &str);
bool isValidPinTypeIdentifier(const std::string &str);

bool checkValidIOChangeRequest(IOType ioType, int pinNumber);
bool checkValidRequestString(const std::string header); 

bool isValidDigitalOutputPin(int pinNumber);
bool isValidDigitalInputPin(int pinNumber);
bool isValidAnalogOutputPin(int pinNumber);
bool isValidAnalogInputPin(int pinNumber);
int parseAnalogPin(const std::string &str);
int parseToState(const std::string &str);

int parseToAnalogState(const std::string &str);
IOType parseIOType(const std::string &str);
int parseToDigitalState(const std::string &str);
int parsePin(const std::string &str);
void populateGpioMap();

std::string getIOTypeString(IOType type);
std::string getSerialPinIOTypeString(int pinNumber);
std::string analogPinFromNumber(int number);
GPIO *gpioPinByPinNumber(int pinNumber);
bool pinInUseBySerialPort(int pinNumber);
int nextAvailableSerialSlotNumber();
static std::map<int, GPIO*> gpioPins;

#if defined(ARDUINO_AVR_UNO)
    static const PROGMEM int AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, -1};
    static const PROGMEM int AVAILABLE_GENERAL_PINS[]{2, 4, 7, 8, 12, 13, -1};
    #define MAXIMUM_SERIAL_PORTS 1
    #define NUMBER_OF_ANALOG_PINS 6
    #define ANALOG_PIN_OFFSET 13
    #if defined(__HAVE_CAN_BUS__)
        #define NUMBER_OF_PINS 17
        static const PROGMEM int AVAILABLE_PWM_PINS[]{3, 5, 6, 10, 11, -1};
    #else
        #define NUMBER_OF_PINS 18
        static const PROGMEM int AVAILABLE_PWM_PINS[]{3, 5, 6, 9, 10, 11, -1};
    #endif
#elif defined(ARDUINO_AVR_NANO)
    #define MAXIMUM_SERIAL_PORTS 1
    static const PROGMEM int AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, A6, A7, -1};                                                
    static const PROGMEM int AVAILABLE_GENERAL_PINS[]{2, 4, 7, 8, 12, 13, -1};
    #define NUMBER_OF_ANALOG_PINS 8
    #define ANALOG_PIN_OFFSET 13
    #if defined(__HAVE_CAN_BUS__)
        #define NUMBER_OF_PINS 19
        static const PROGMEM int AVAILABLE_PWM_PINS[]{3, 5, 6, 10, 11, -1};
    #else
        #define NUMBER_OF_PINS 20
        static const PROGMEM int AVAILABLE_PWM_PINS[]{3, 5, 6, 9, 10, 11, -1};
    #endif
#elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    #define MAXIMUM_SERIAL_PORTS 8
    static const PROGMEM int AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, -1};
    static const PROGMEM int AVAILABLE_GENERAL_PINS[]{14, 15, 16, 17, 18, 19, 20,21, 22, 23, 24, 
                                                  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
                                                  38, 39, 40, 41, 42, 43, 47, 48, 49, 50, 51, 52, 53,-1};            

    static const int PROGMEM AVAILABLE_SERIAL_RX_PINS[]{10, 11, 12, 13, 14, 15, 50, 51, 52, 53,
                                                 A8, A9, A10, A11, A12, A13, A14, A15};                                    
    #define NUMBER_OF_ANALOG_PINS 16
    #define ANALOG_PIN_OFFSET 53
    #if defined(__HAVE_CAN_BUS__)
        #define NUMBER_OF_PINS 67
        static const PROGMEM int AVAILABLE_PWM_PINS[]{2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 44, 45, 46, -1};
    #else
        #define NUMBER_OF_PINS 68
        static const PROGMEM int AVAILABLE_PWM_PINS[]{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, 46, -1};
    #endif
#endif

#if defined(__HAVE_CAN_BUS__)
    #include <set>
    #ifndef INT32U
        #define INT32U unsigned long
    #endif
    #ifndef INT8U
        #define INT8U byte
    #endif
    void canInitRequest();
    void canReadRequest(bool autoUp);
    void canWriteRequest(const std::string &canMessage, bool once);
    void addNegativeCanMaskRequest(const std::string &canMask);
    void removePositiveCanMaskRequest(const std::string &canMask);
    void canLiveUpdateRequest(const std::string &str);
    void clearCurrentMessageByIdRequest(const std::string &str);
    void currentCachedCanMessageByIdRequest(const std::string &str);
    void clearCanMessagesRequest();
    void currentPositiveCanMasksRequest();
    void currentNegativeCanMasksRequest();
    void allCurrentCanMasksRequest();
    void clearPositiveCanMasksRequest();
    void clearNegativeCanMasksRequest();
    void clearCanMasksRequest();
    void currentCachedCanMessagesRequest();
    void clearAllCanMasksRequest();
    std::pair<int, std::string> parseToCanState(const std::string &str);
    void sendCanMessage(const CanMessage &msg);
    #define SPI_CS_PIN 9 
    MCP_CAN *canController{new MCP_CAN(SPI_CS_PIN)};
    #define CAN_CONNECTION_TIMEOUT 1000
    #define CAN_WRITE_REQUEST_SIZE 9
    static bool canBusInitialized{false};
    static bool canLiveUpdate{false};
    CanMessage parseCanMessage(const std::string &str);
    static std::map<uint32_t, CanMessage> lastCanMessages;
    static std::set<uint32_t> positiveCanMasks;
    static std::set<uint32_t> negativeCanMasks;
    void addLastCanMessage(const CanMessage &msg);
    #define CAN_MESSAGE_LENGTH 8
    #define CAN_FRAME 0
    #define CAN_COMMUNICATION_DOWN_TIME 1000
    static int canCommunicationStartTime{0};
    static int canCommunicationEndTime{0};
#endif
    
using HWSerial = HardwareSerialPortInfo;
using SWSerial = SoftwareSerialPortInfo;

#if defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    enum SerialIndex {
        BIT_BUCKET = -1,
        NATIVE_PORT_0 = 0,
        NATIVE_PORT_1 = 1,
        NATIVE_PORT_2 = 2,
        NATIVE_PORT_3 = 3,
        SOFTWARE_PORT_0 = 4,
        SOFTWARE_PORT_1 = 5,
        SOFTWARE_PORT_2 = 6,
        SOFTWARE_PORT_3 = 7,
        ALL_NATIVE_PORTS = 8,
        ALL_SOFTWARE_PORTS = 9,
        ALL_PORTS = 10
    };
    #define SOFTWARE_SERIAL_ENUM_OFFSET 4

    static std::vector<HWSerial *> hardwareSerialPorts {
        new HWSerial{&Serial,   0,   1,  SERIAL_BAUD, SERIAL_TIMEOUT, true},
        new HWSerial{&Serial1,  19,  18, SERIAL_BAUD, SERIAL_TIMEOUT, true},
        new HWSerial{&Serial2,  17,  16, SERIAL_BAUD, SERIAL_TIMEOUT, true},
        new HWSerial{&Serial3,  15,  14, SERIAL_BAUD, SERIAL_TIMEOUT, true}
    };

#else
    enum SerialIndex {
        BIT_BUCKET = -1,
        NATIVE_PORT_0 = 0,
        SOFTWARE_PORT_0 = 1,
        SOFTWARE_PORT_1 = 2,
        SOFTWARE_PORT_2 = 3,
        SOFTWARE_PORT_3 = 4,
        ALL_SOFTWARE_PORTS = 5,
        ALL_PORTS = 6
    };
    #define SOFTWARE_SERIAL_ENUM_OFFSET 1
    static std::vector<HWSerial *> hardwareSerialPorts {
        new HWSerial{&Serial, 0,  1, SERIAL_BAUD, SERIAL_TIMEOUT, true},
    };

#endif

static std::vector<SWSerial *> softwareSerialPorts {

};

void initializeSerialPorts();
void announceStartup();
bool isSoftwareCoutStream(int coutIndex);
SerialPortInfo *getHardwareCout(int coutIndex);
SerialPortInfo *getSoftwareCout(int coutIndex);
static SerialPortInfo *currentSerialStream{hardwareSerialPorts.at(0)};

SerialPortInfo *defaultNativePort{hardwareSerialPorts.at(0)};

template <typename Header, typename PinNumber, typename State, typename ResultCode> inline void printResult(const Header &header, PinNumber pinNumber, State state, ResultCode resultCode)
{    
    *getCurrentValidOutputStream() << header << ':' << pinNumber << ':' << state << ':' << resultCode << '}'; 
}

template <typename Header, typename ResultCode> inline void printSingleResult(const Header &header, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ':' << resultCode << '}'; 
}

template <typename Header, typename Type, typename ResultCode> inline void printTypeResult(const Header &header, Type type, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ':' << type << ':' << resultCode << '}'; 
}

template <typename Parameter> inline void printString(const Parameter &parameter)
{
    *getCurrentValidOutputStream() << parameter;
}

int main()
{
    arduinoInit();
    initializeSerialPorts();
    announceStartup();
    populateGpioMap();

    while (true) {
        for (auto &it : hardwareSerialPorts) {
            if ((it->isEnabled()) && (it->available())) {
                String serialRead{it->readStringUntil('}')};
                currentSerialStream = it;
                if (serialRead.length() > MAXIMUM_SERIAL_READ_SIZE) {
                    printString(INVALID_HEADER);
                } else {
                    handleSerialString(serialRead.c_str());
                }
            }
        }
        for (auto &it : softwareSerialPorts) {
            if ((it->isEnabled()) && (it->available())) {
                String serialRead{it->readStringUntil('}')};
                if (serialRead.length() > MAXIMUM_SERIAL_READ_SIZE) {
                    printString(INVALID_HEADER);
                } else {
                    handleSerialString(serialRead.c_str());
                }
            }
        }
        #if defined(__HAVE_CAN_BUS__)
            if (canLiveUpdate) {
                canReadRequest(canLiveUpdate, SerialIndex::ALL_NATIVE_PORTS);
            }
            bool canSend{false};
            for (auto &it : lastCanMessages) {
                if (it.second->toString() != "") {
                    canSend = true;
                }
            }
            if (canSend) {
                if (canCommunicationStartTime == 0) {
                    canCommunicationStartTime = millis();
                }
                canCommunicationEndTime = millis();
                if (static_cast<unsigned int>((canCommunicationEndTime - canCommunicationStartTime)) >= CAN_COMMUNICATION_DOWN_TIME) {    
                    for (auto &it : lastCanMessages) {
                        sendCanMessage(it.second);
                    }
                    canCommunicationStartTime = millis();
                }
            }
        #endif
        delay(500);
        if (serialEventRun) serialEventRun();
    }
    for (auto &it : gpioPins) {
        delete it.second;
    }
    for (auto &it : hardwareSerialPorts) {
        delete it;
    }
    for (auto &it : softwareSerialPorts) {
        delete it;
    }
    #if defined(__HAVE_CAN_BUS__)
        delete canController;
    #endif
    return 0;
}

void handleSerialString(const char *str)
{
    return handleSerialString(static_cast<std::string>(str));
}

void handleSerialString(const std::string &str)
{
    if (str.length() == 0) {
        return;
    }
    if (startsWith(str, ANALOG_READ_HEADER)) {
        if (checkValidRequestString(ANALOG_READ_HEADER, str)) {
            analogReadRequest(str.substr(static_cast<std::string>(ANALOG_READ_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ANALOG_WRITE_HEADER)) {
        if (checkValidRequestString(ANALOG_WRITE_HEADER, str)) {
            analogWriteRequest(str.substr(static_cast<std::string>(ANALOG_WRITE_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CHANGE_A_TO_D_THRESHOLD_HEADER)) {
        if (checkValidRequestString(CHANGE_A_TO_D_THRESHOLD_HEADER, str)) {
            changeAToDThresholdRequest(str.substr(static_cast<std::string>(CHANGE_A_TO_D_THRESHOLD_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_SOFTWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(ADD_SOFTWARE_SERIAL_HEADER, str)) {
            addSoftwareSerialRequest(str.substr(static_cast<std::string>(ADD_SOFTWARE_SERIAL_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_HARDWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(ADD_HARDWARE_SERIAL_HEADER, str)) {
            addHardwareSerialRequest(str.substr(static_cast<std::string>(ADD_HARDWARE_SERIAL_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }  
    } else if (startsWith(str, DIGITAL_READ_HEADER)) {
        if (checkValidRequestString(DIGITAL_READ_HEADER, str)) {
            digitalReadRequest(str.substr(static_cast<std::string>(DIGITAL_READ_HEADER).length()+1), false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_ALL_HEADER)) {
        if (checkValidRequestString(DIGITAL_WRITE_ALL_HEADER, str)) {
            digitalWriteAllRequest(str.substr(static_cast<std::string>(DIGITAL_WRITE_ALL_HEADER).length()+1));
        } else {
            printTypeResult(DIGITAL_WRITE_ALL_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_HEADER)) {
        if (checkValidRequestString(DIGITAL_WRITE_HEADER, str)) {
            digitalWriteRequest(str.substr(static_cast<std::string>(DIGITAL_WRITE_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_HEADER)) {
        if (checkValidRequestString(PIN_TYPE_HEADER, str)) {
            pinTypeRequest(str.substr(static_cast<std::string>(PIN_TYPE_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_CHANGE_HEADER)) {
        if (checkValidRequestString(PIN_TYPE_CHANGE_HEADER, str)) {
            pinTypeChangeRequest(str.substr(static_cast<std::string>(PIN_TYPE_CHANGE_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_SOFTWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(REMOVE_SOFTWARE_SERIAL_HEADER, str)) {
            removeSoftwareSerialRequest(str.substr(static_cast<std::string>(REMOVE_SOFTWARE_SERIAL_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_HARDWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(REMOVE_HARDWARE_SERIAL_HEADER, str)) {
            removeHardwareSerialRequest(str.substr(static_cast<std::string>(REMOVE_HARDWARE_SERIAL_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
    } else if (startsWith(str, SOFT_DIGITAL_READ_HEADER)) {
        if (checkValidRequestString(SOFT_DIGITAL_READ_HEADER, str)) {
            digitalReadRequest(str.substr(static_cast<std::string>(SOFT_DIGITAL_READ_HEADER).length()+1), SOFT);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, SOFT_ANALOG_READ_HEADER)) {
        if (checkValidRequestString(SOFT_ANALOG_READ_HEADER, str)) {
            softAnalogReadRequest(str.substr(static_cast<std::string>(SOFT_ANALOG_READ_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, FIRMWARE_VERSION_HEADER)) {
        firmwareVersionRequest();
    } else if (startsWith(str, IO_REPORT_HEADER)) {
        ioReportRequest();
    } else if (startsWith(str, CURRENT_A_TO_D_THRESHOLD_HEADER)) {
        currentAToDThresholdRequest();
    } else if (startsWith(str, ARDUINO_TYPE_HEADER)) {
        arduinoTypeRequest();
#if defined(__HAVE_CAN_BUS__)
    } else if (startsWith(str, CAN_INIT_HEADER)) {
        canInitRequest();
    } else if (startsWith(str, CAN_READ_HEADER)) {
        canReadRequest();
    } else if (startsWith(str, CAN_WRITE_ONCE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_ONCE_HEADER, str)) {
            canWriteRequest(str.substr(static_cast<std::string>(CAN_WRITE_HEADER).length()+1), true);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CAN_WRITE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_HEADER, str)) {
            canWriteRequest(str.substr(static_cast<std::string>(CAN_WRITE_HEADER).length()+1), false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }     
    } else if (startsWith(str, CAN_LIVE_UPDATE_HEADER)) {
        if (checkValidRequestString(CAN_LIVE_UPDATE_HEADER, str)) {
            canLiveUpdateRequest(str.substr(static_cast<std::string>(CAN_LIVE_UPDATE_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CLEAR_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str)) {
            clearCurrentMessageByIdRequest(str.substr(static_cast<std::string>(CLEAR_CAN_MESSAGE_BY_ID_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
    } else if (startsWith(str, CURRENT_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CURRENT_CAN_MESSAGE_BY_ID_HEADER, str)) {
            currentCachedCanMessageByIdRequest(str.substr(static_cast<std::string>(CURRENT_CAN_MESSAGE_BY_ID_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_NEGATIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_NEGATIVE_CAN_MASK_HEADER, str)) {
            removeNegativeCanMaskRequest(str.substr(static_cast<std::string>(REMOVE_NEGATIVE_CAN_MASK_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_POSITIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_POSITIVE_CAN_MASK_HEADER, str)) {
            removePositiveCanMaskRequest(str.substr(static_cast<std::string>(REMOVE_POSITIVE_CAN_MASK_HEADER).length()+1));
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CURRENT_CAN_MESSAGES_HEADER)) {
        currentCachedCanMessagesRequest();
    } else if (startsWith(str, CLEAR_CAN_MESSAGES_HEADER)) {
        clearCanMessagesRequest();
    } else if (startsWith(str, CURRENT_POSITIVE_CAN_MASKS_HEADER)) {
        currentPositiveCanMasksRequest();
    } else if (startsWith(str, CURRENT_NEGATIVE_CAN_MASKS_HEADER)) {
        currentNegativeCanMasksRequest();
    } else if (startsWith(str, CLEAR_POSITIVE_CAN_MASKS_HEADER)) {
        clearPositiveCanMasksRequest();
    } else if (startsWith(str, CLEAR_NEGATIVE_CAN_MASKS_HEADER)) {
        clearNegativeCanMasksRequest();
    } else if (startsWith(str, CLEAR_ALL_CAN_MASKS_HEADER)) {
        clearAllCanMasksRequest();
#endif
    } else {
        printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
    }
}

void addSoftwareSerialRequest(const std::string &str)
{
    size_t foundPosition{str.find(":")};
    std::string maybeRxPin{str.substr(0, foundPosition).c_str()};
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    std::string maybeTxPin{str.substr(foundPosition+1).c_str()};
    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, OPERATION_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (pinInUseBySerialPort(txPinNumber)) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, OPERATION_FAILURE, maybeTxPin, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    for (auto &it : softwareSerialPorts) {
        if ((rxPinNumber == it->rxPin()) && (txPinNumber == it->txPin())) {
            it->setEnabled(true);
            it->initialize();
            printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
            return;
        }
    }
    if (isValidSoftwareSerialAddition(rxPinNumber, txPinNumber)) {
        SWSerial *swSerialTemp{ new SWSerial{rxPinNumber, txPinNumber, SERIAL_BAUD, SERIAL_TIMEOUT, true} };
        softwareSerialPorts.push_back(swSerialTemp);
        swSerialTemp->initialize();
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
    } else {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
    }
}

void addHardwareSerialRequest(const std::string &str)
{
    size_t foundPosition{str.find(":")};
    std::string maybeRxPin{str.substr(0, foundPosition).c_str()};
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    std::string maybeTxPin{str.substr(foundPosition+1).c_str()};
    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (auto &it : hardwareSerialPorts) {
        if ((rxPinNumber == it->rxPin()) && (txPinNumber == it->txPin())) {
            it->setEnabled(true);
            it->initialize();
            printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
            return;
        }
    }
    printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

void removeSoftwareSerialRequest(const std::string &str)
{
    size_t foundPosition{str.find(":")};
    std::string maybeRxPin{str.substr(0, foundPosition).c_str()};
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    std::string maybeTxPin{str.substr(foundPosition+1).c_str()};
    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (auto &it : softwareSerialPorts) {
        if ((rxPinNumber == it->rxPin()) && (txPinNumber == it->txPin())) {
            it->setEnabled(false);
            printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
            return;
        }
    }
    printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

void removeHardwareSerialRequest(const std::string &str)
{
    size_t foundPosition{str.find(":")};
    std::string maybeRxPin{str.substr(0, foundPosition).c_str()};
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    std::string maybeTxPin{str.substr(foundPosition+1).c_str()};
    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (auto &it : hardwareSerialPorts) {
        if ((rxPinNumber == it->rxPin()) && (txPinNumber == it->txPin())) {
            it->setEnabled(false);
            printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
            return;
        }
    }
    printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

bool isValidSoftwareSerialAddition(int rxPinNumber, int txPinNumber)
{
    if (softwareSerialPorts.size() == MAXIMUM_SERIAL_PORTS) {
        return false;
    }
    if (pinInUseBySerialPort(rxPinNumber) || pinInUseBySerialPort(txPinNumber)) {
        return false;
    }
    if ((rxPinNumber <= 1) || (txPinNumber <= 1)) {        
         return false;
    }
    #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
        if (rxPinNumber == 13) {
            return false;
        } else {
            return true;
        }
    #elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
        //Used by hardware serial ports
        if ( ((rxPinNumber >= 14) && (rxPinNumber <= 19)) ||
             ((txPinNumber >= 14) && (txPinNumber <= 19)) ) {
            
            return false;
        } 
        
        int i{0};
        do {
            int tempPinNumber{pgm_read_word_near(AVAILABLE_SERIAL_RX_PINS + i++)};
            if (tempPinNumber < 0) {
                break;
            }
            if (tempPinNumber == rxPinNumber) {
                return true;
            }
        } while (true);

        return false; 
    #else
        return true;
    #endif
}

bool isValidHardwareSerialAddition(int rxPinNumber, int txPinNumber)
{
    if (pinInUseBySerialPort(rxPinNumber) || pinInUseBySerialPort(txPinNumber)) {
        return false;
    }
    #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
        return false;
    #elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
        if ( ((rxPinNumber == 19) && (txPinNumber == 18)) ||
             ((rxPinNumber == 17) && (txPinNumber == 16)) ||
             ((rxPinNumber == 15) && (txPinNumber == 14)) ) {
            
            return true;
        } else {
            return false;
        }
    #else
        return false;
    #endif
}

void changeAToDThresholdRequest(const std::string &str)
{
    if (str.length() == 0) {
        printTypeResult(CHANGE_A_TO_D_THRESHOLD_HEADER, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int maybeState{parseToAnalogState(str)};
    if (maybeState == OPERATION_FAILURE) {
        printTypeResult(CHANGE_A_TO_D_THRESHOLD_HEADER, str, OPERATION_FAILURE);
        return;
    } 
    GPIO::setAnalogToDigitalThreshold(maybeState);
    printTypeResult(CHANGE_A_TO_D_THRESHOLD_HEADER, GPIO::analogToDigitalThreshold(), OPERATION_SUCCESS);
}

void currentAToDThresholdRequest()
{
    printTypeResult(CURRENT_A_TO_D_THRESHOLD_HEADER, GPIO::analogToDigitalThreshold(), OPERATION_SUCCESS);
}

void ioReportRequest()
{
    *getCurrentValidOutputStream() << IO_REPORT_HEADER << '}';
    for (auto &it : gpioPins) {
        GPIO *gpioPin{it.second};
        int state{0};
        if ((gpioPin->ioType() == IOType::DIGITAL_INPUT) || (gpioPin->ioType() == IOType::DIGITAL_INPUT_PULLUP)) {
            state = gpioPin->g_digitalRead();
        } else if (gpioPin->ioType() == IOType::DIGITAL_OUTPUT) {
            state = gpioPin->g_softDigitalRead();
        } else if (gpioPin->ioType() == IOType::ANALOG_INPUT) {
            state = gpioPin->g_analogRead();
        } else if (gpioPin->ioType() == IOType::ANALOG_OUTPUT) {
            state = gpioPin->g_softAnalogRead();
        }
        if (isValidAnalogInputPin(gpioPin->pinNumber())) {
            *getCurrentValidOutputStream() << '{' << analogPinFromNumber(gpioPin->pinNumber()) << ':' << getIOTypeString(gpioPin->ioType()) << ':' << state << "};";
        } else {
            *getCurrentValidOutputStream() << '{' << gpioPin->pinNumber() << ':' << getIOTypeString(gpioPin->ioType()) << ':' << state << "};";
        }
    }
    *getCurrentValidOutputStream() << IO_REPORT_END_HEADER << '}';
}

void digitalReadRequest(const std::string &str, bool soft)
{
    if (str.length() == 0) {
        printResult(DIGITAL_READ_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int pinNumber{parsePin(str)};
    if (pinNumber == INVALID_PIN) {
        printResult((soft ? SOFT_DIGITAL_READ_HEADER : DIGITAL_READ_HEADER), str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult((soft ? SOFT_DIGITAL_READ_HEADER : DIGITAL_READ_HEADER), str, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!isValidDigitalInputPin(pinNumber)) {
        printResult((soft ? SOFT_DIGITAL_READ_HEADER : DIGITAL_READ_HEADER), str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    bool state{(soft ? gpioPinByPinNumber(pinNumber)->g_softDigitalRead() : gpioPinByPinNumber(pinNumber)->g_digitalRead())};
    printResult((soft ? SOFT_DIGITAL_READ_HEADER : DIGITAL_READ_HEADER), str, state, OPERATION_SUCCESS);
}

void digitalWriteRequest(const std::string &str)
{
    if (str.length() == 0) {
        printResult(DIGITAL_WRITE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    size_t foundPosition{str.find(":")};
    std::string maybePin{str.substr(0, foundPosition).c_str()};
    int pinNumber{parsePin(maybePin)};
    if (pinNumber == INVALID_PIN) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!isValidDigitalOutputPin(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    std::string maybeState{str.substr(foundPosition+1).c_str()};
    int state{parseToDigitalState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, maybeState, OPERATION_FAILURE);
        return;
    } 
    gpioPinByPinNumber(pinNumber)->g_digitalWrite(state);
    printResult(DIGITAL_WRITE_HEADER, maybePin, maybeState, OPERATION_SUCCESS);
}

void digitalWriteAllRequest(const std::string &str)
{
    *getCurrentValidOutputStream() << DIGITAL_WRITE_ALL_HEADER;
    if (str.length() == 0) {
        printResult(DIGITAL_WRITE_ALL_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    size_t foundPosition{str.find(":")};
    std::string maybeState{str.substr(0, foundPosition).c_str()};
    int state{parseToDigitalState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printTypeResult(DIGITAL_WRITE_ALL_HEADER, maybeState, OPERATION_FAILURE);
        return;
    }
    for (auto &it : gpioPins) {
        GPIO *gpioPin{it.second};
        if (gpioPin->ioType() == IOType::DIGITAL_OUTPUT) {
            gpioPin->g_digitalWrite(state);
            if (isValidAnalogInputPin(gpioPin->pinNumber())) {
                *getCurrentValidOutputStream() << ':' << analogPinFromNumber(gpioPin->pinNumber());
            } else {
                *getCurrentValidOutputStream() << ':' << gpioPin->pinNumber();
        
            }
        }
    }
    *getCurrentValidOutputStream() << ':' << state << ':' << OPERATION_SUCCESS << '}';
}

void analogReadRequest(const std::string &str)
{
    if (str.length() == 0) {
        printResult(ANALOG_READ_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int pinNumber{parsePin(str)};
    if (pinNumber == INVALID_PIN) {
        printResult(ANALOG_READ_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(ANALOG_READ_HEADER, str, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!isValidAnalogInputPin(pinNumber)) {
        printResult(ANALOG_READ_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    printResult(ANALOG_READ_HEADER, str, gpioPinByPinNumber(pinNumber)->g_analogRead(), OPERATION_SUCCESS);
}

void analogWriteRequest(const std::string &str)
{
    if (str.length() == 0) {
        printResult(ANALOG_WRITE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    size_t foundPosition{str.find(":")};
    std::string maybePin{str.substr(0, foundPosition).c_str()};
    int pinNumber{parsePin(maybePin)};

    if (pinNumber == INVALID_PIN) {
        printResult(ANALOG_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(ANALOG_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!isValidAnalogOutputPin(pinNumber)) {
        printResult(ANALOG_WRITE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    std::string maybeState{str.substr(foundPosition+1).c_str()};
    int state{parseToAnalogState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printResult(ANALOG_WRITE_HEADER, maybePin, maybeState, OPERATION_FAILURE);
    } else {
        gpioPinByPinNumber(pinNumber)->g_analogWrite(state);
        printResult(ANALOG_WRITE_HEADER, maybePin, state, OPERATION_SUCCESS);
    }
}

void pinTypeRequest(const std::string &str)
{
    if (str.length() == 0) {
        printResult(PIN_TYPE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int pinNumber = parsePin(str);
    if (pinNumber == INVALID_PIN) {
        printResult(PIN_TYPE_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(PIN_TYPE_HEADER, str, getSerialPinIOTypeString(pinNumber), OPERATION_SUCCESS);
        return;
    }
    printResult(PIN_TYPE_HEADER, str, getIOTypeString(gpioPinByPinNumber(pinNumber)->ioType()), OPERATION_SUCCESS);
}

void pinTypeChangeRequest(const std::string &str)
{   
    if (str.length() == 0) {
        printResult(PIN_TYPE_CHANGE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    size_t foundPosition{str.find(":")};

    std::string maybePin{str.substr(0, foundPosition)};
    int pinNumber{parsePin(maybePin)};

    if (pinNumber == INVALID_PIN) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }

    std::string maybeType{str.substr(foundPosition+1)};
    IOType type{parseIOType(maybeType)};
    if (type == IOType::UNSPECIFIED) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, maybeType, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, getIOTypeString(type), OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!checkValidIOChangeRequest(type, pinNumber)) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, getIOTypeString(type), OPERATION_FAILURE);
        return;
    }
    gpioPinByPinNumber(pinNumber)->setIOType(type);
    printResult(PIN_TYPE_CHANGE_HEADER, maybePin, getIOTypeString(type), OPERATION_SUCCESS);
}

void softAnalogReadRequest(const std::string &str)
{
    if (str.length() == 0) {
        printResult(SOFT_ANALOG_READ_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int pinNumber{parsePin(str)};
    if (pinNumber == INVALID_PIN) {
        printResult(SOFT_ANALOG_READ_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (!isValidAnalogOutputPin(pinNumber)) {
        printResult(SOFT_ANALOG_READ_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    int state{gpioPinByPinNumber(pinNumber)->g_softAnalogRead()};
    printResult(SOFT_ANALOG_READ_HEADER, str, state, OPERATION_SUCCESS);
}

void heartbeatRequest()
{
    printString(static_cast<std::string>(HEARTBEAT_HEADER) + "}");
}

void arduinoTypeRequest()
{
    printTypeResult(ARDUINO_TYPE_HEADER, ARDUINO_TYPE, OPERATION_SUCCESS);
}

void firmwareVersionRequest()
{
    printTypeResult(FIRMWARE_VERSION_HEADER, FIRMWARE_VERSION, OPERATION_SUCCESS);
}

void canBusEnabledRequest()
{
    #if defined(__HAVE_CAN_BUS__)
        printTypeResult(CAN_BUS_ENABLED_HEADER, 1, OPERATION_SUCCESS);
    #else
        printTypeResult(CAN_BUS_ENABLED_HEADER, 0, OPERATION_SUCCESS);
    #endif
} 

void populateGpioMap()
{
    int i{0};
    do {
        int pinNumber{pgm_read_word_near(AVAILABLE_PWM_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins.insert(std::pair<int, GPIO*>(pinNumber, new GPIO(pinNumber, IOType::DIGITAL_INPUT_PULLUP)));
    } while (true);
    i = 0;
    do {
        int pinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins.insert(std::pair<int, GPIO*>(pinNumber, new GPIO(pinNumber, IOType::ANALOG_INPUT)));
    } while (true);
    i = 0;
    do {
        int pinNumber{pgm_read_word_near(AVAILABLE_GENERAL_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins.insert(std::pair<int, GPIO*>(pinNumber, new GPIO(pinNumber, IOType::DIGITAL_INPUT_PULLUP)));
    } while (true);
}

bool checkValidRequestString(const char *header, const std::string &checkStr)
{
    return (checkStr.length() > (static_cast<std::string>(header).length() + 1));
}


bool isValidDigitalOutputPin(int pinNumber)
{
#if defined(ARDUINO_AVR_NANO)
    if ((pinNumber == A6) || (pinNumber == A7)) {
        return false;
    }
#endif
    for (auto &it : gpioPins) {
        if (pinNumber == it.first) {
            return true;
        }
    }
    return false;
}

bool isValidDigitalInputPin(int pinNumber)
{
#if defined(ARDUINO_AVR_NANO)
    if ((pinNumber == A6) || (pinNumber == A7)) {
        return false;
    }
#endif
    for (auto &it : gpioPins) {
        if (pinNumber == it.first) {
            return true;
        }
    }
    return false;
}

bool isValidAnalogOutputPin(int pinNumber)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_PWM_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == pinNumber) {
            return true;
        }
    } while (true);
}

bool isValidAnalogInputPin(int pinNumber)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == pinNumber) {
            return true;
        }
    } while (true);
}

bool checkValidIOChangeRequest(IOType ioType, int pinNumber)
{
    if ((ioType == IOType::DIGITAL_INPUT) || (ioType == IOType::DIGITAL_INPUT_PULLUP)) {
        return isValidDigitalInputPin(pinNumber);
    } else if (ioType == IOType::DIGITAL_OUTPUT) {
        return isValidDigitalOutputPin(pinNumber);
    } else if (ioType == IOType::ANALOG_INPUT) {
        return isValidAnalogInputPin(pinNumber);
    } else if (ioType == IOType::ANALOG_OUTPUT) {
        return isValidAnalogOutputPin(pinNumber);
    } else {
        return false;
    }
}

std::string getIOTypeString(IOType ioType)
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
        return UNSPECIFIED_IO_TYPE_IDENTIFIER;
    }
}

int parsePin(const std::string &str)
{
    if (str.find("A") != std::string::npos) {
        return parseAnalogPin(str);
    } else if (isValidPinIdentifier(str)) {
        return atoi(str.c_str());
    } else {
        return INVALID_PIN;
    }
}

IOType parseIOType(const std::string &str)
{
    if (str == DIGITAL_INPUT_IDENTIFIER) {
        return IOType::DIGITAL_INPUT;
    } else if (str == DIGITAL_OUTPUT_IDENTIFIER) {
        return IOType::DIGITAL_OUTPUT;
    } else if (str == ANALOG_INPUT_IDENTIFIER) {
        return IOType::ANALOG_INPUT;
    } else if (str == ANALOG_OUTPUT_IDENTIFIER) {
        return IOType::ANALOG_OUTPUT;
    } else if (str == DIGITAL_INPUT_PULLUP_IDENTIFIER) {
        return IOType::DIGITAL_INPUT_PULLUP;
    } else {
        return IOType::UNSPECIFIED;
    }
}

bool isValidAnalogStateIdentifier(const std::string &str)
{
    if (str == "0") {
        return true;
    }
    int returnVal{atoi(str.c_str())};
    return (returnVal != 0); 
}

bool isValidDigitalStateIdentifier(const std::string &str)
{   
    return (str == "1") || (str == "0");
}

int parseToDigitalState(const std::string &str)
{
    if (str == "1") {
        return HIGH; 
    } else if (str == "0") {
        return LOW;
    } else {
        return OPERATION_FAILURE;
    }
}

int parseToAnalogState(const std::string &str)
{
    if (!isValidAnalogStateIdentifier(str)) {
        return OPERATION_FAILURE;
    } else {
        int temp{atoi(str.c_str())};
        if (temp > GPIO::ANALOG_MAX) {
            temp = GPIO::ANALOG_MAX;
        }
        if (temp < 0) {
            return OPERATION_FAILURE;
        }
        return temp;
    }
}

bool isValidPinIdentifier(const std::string &str)
{
    for (auto &it : gpioPins) {
        if (atoi(str.c_str()) == it.first) {
            return true;
        }
    }
    for (unsigned int i =  0; i < ARRAY_SIZE(AVAILABLE_ANALOG_PINS)-1; i++) {
        if (str.find("A") != std::string::npos) {
            if (atoi(str.substr(1).c_str()) == i) {
                return true;
            }
        }
    }
    return false;
}

bool isValidPwmPinIdentifier(const std::string &str)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_PWM_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == atoi(str.c_str())) {
            return true;
        }
    } while (true);
}

bool isValidAnalogPinIdentifier(const std::string &str)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            break;
        }
        if (tempPinNumber == atoi(str.c_str())) {
            return true;
        }
    } while (true);

    for (unsigned int j =  0; j < ARRAY_SIZE(AVAILABLE_ANALOG_PINS)-1; j++) {
        if (str.find("A") != std::string::npos) {
            if (atoi(str.substr(1).c_str()) == j) {
                return true;
            }
        }
    }
    return false;
}

bool isValidPinTypeIdentifier(const std::string &str)
{
    std::string copyString{toLowercase(str)};
    return ((copyString == static_cast<std::string>(DIGITAL_INPUT_IDENTIFIER)) ||
            (copyString == static_cast<std::string>(DIGITAL_OUTPUT_IDENTIFIER)) ||
            (copyString == static_cast<std::string>(ANALOG_INPUT_IDENTIFIER)) ||
            (copyString == static_cast<std::string>(ANALOG_OUTPUT_IDENTIFIER)) ||
            (copyString == static_cast<std::string>(DIGITAL_INPUT_PULLUP_IDENTIFIER)));
}

void initializeSerialPorts()
{
    for (auto &it : hardwareSerialPorts) {
        if (it->isEnabled()) {
            (void)it->initialize();
        }
    }
}

void broadcastString(const std::string &str)
{
    for (auto &it : hardwareSerialPorts) {
        if (it->isEnabled()) {
            it->print(str);            
        }
    }    
}

void announceStartup()
{
    broadcastString(static_cast<std::string>(INITIALIZATION_HEADER) 
                    + ":" 
                    + static_cast<std::string>(ARDUINO_TYPE) 
                    + ":" 
                    + static_cast<std::string>(FIRMWARE_VERSION) 
                    + "}");
}

GPIO *gpioPinByPinNumber(int pinNumber)
{
    auto result = gpioPins.find(pinNumber);
    if (result == gpioPins.end()) {
        return nullptr;
    } else {
        return result->second;
    }
}
int parseAnalogPin(const std::string &pinAlias)
{
    for (unsigned int i =  0; i < ARRAY_SIZE(AVAILABLE_ANALOG_PINS)-1; i++) {
        if (pinAlias.find("A") != std::string::npos) {
            if (atoi(pinAlias.substr(1).c_str()) == i) {
                return ANALOG_PIN_OFFSET + i + 1;
            }
        }
    }
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            break;
        }
        if (toDecString(tempPinNumber) == pinAlias) {
            return tempPinNumber;
        }
    } while (true);
    return 0;
}

std::string analogPinFromNumber(int pinNumber)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            return "";
        }
        if (tempPinNumber == pinNumber) {
            return "A" + toString(i-1);
        }
    } while (true);
}

int nextAvailableSerialSlotNumber()
{
    if (softwareSerialPorts.size() == MAXIMUM_SERIAL_PORTS) {
        return NEXT_SERIAL_PORT_UNAVAILABLE;
    } else {
        return softwareSerialPorts.size();
    }
}

bool pinInUseBySerialPort(int pinNumber)
{
    for (auto &it : hardwareSerialPorts) {
        if (it->isEnabled()) {
            if ((pinNumber == it->rxPin()) || (pinNumber == it->txPin())) {
                return true;
            }
        }
    }
    for (auto &it : softwareSerialPorts) {
        if (it->isEnabled()) {
            if ((pinNumber == it->rxPin()) || (pinNumber == it->txPin())) {
                return true;
            }
        }
    }
    return false;
}

std::string getSerialPinIOTypeString(int pinNumber)
{
    for (auto &it : hardwareSerialPorts) {
        if (it->isEnabled()) {
            if (pinNumber == it->rxPin()) {
                return static_cast<std::string>(HARDWARE_SERIAL_RX_PIN_TYPE);
            } else if (pinNumber == it->txPin()) {
                return static_cast<std::string>(HARDWARE_SERIAL_TX_PIN_TYPE);
            }
        }
    }
    for (auto &it : softwareSerialPorts) {
        if (it->isEnabled()) {
            if (pinNumber == it->rxPin()) {
                return static_cast<std::string>(SOFTWARE_SERIAL_RX_PIN_TYPE);
            } else if (pinNumber == it->txPin()) {
                return static_cast<std::string>(SOFTWARE_SERIAL_TX_PIN_TYPE);
            }
        }
    }
    return "";
}

SerialPortInfo *getCurrentValidOutputStream()
{
    return (currentSerialStream != nullptr ? currentSerialStream : defaultNativePort);
}

SerialPortInfo *getHardwareCout(int coutIndex)
{
    if (coutIndex < 0) {
        return nullptr;
    }
    if (static_cast<unsigned int>(coutIndex) > hardwareSerialPorts.size()-1) {
        return nullptr;
    } else {
        if (hardwareSerialPorts.at(coutIndex)->isEnabled()) {
            return hardwareSerialPorts.at(coutIndex);
        } else {
            return nullptr;
        }
    }
}

SerialPortInfo *getSoftwareCout(int coutIndex)
{
    if (coutIndex < 0) {
        return nullptr;
    }
    if (static_cast<unsigned int>(coutIndex) > softwareSerialPorts.size()-1) {
        return nullptr;
    } else {
        if (softwareSerialPorts.at(coutIndex)->isEnabled()) {
            return softwareSerialPorts.at(coutIndex);
        } else {
            return nullptr;
        }
    }
}

bool isSoftwareCoutStream(int coutIndex)
{
    if ((coutIndex == SerialIndex::SOFTWARE_PORT_0) ||
        (coutIndex == SerialIndex::SOFTWARE_PORT_1) ||
        (coutIndex == SerialIndex::SOFTWARE_PORT_2) ||
        (coutIndex == SerialIndex::SOFTWARE_PORT_3)) {
        
        return true;
    } else { 
        return false;
    }
}

#if defined(__HAVE_CAN_BUS__)
    void printCanResult(const std::string &header, const std::string &str, int resultCode) 
    { 
        *getCurrentValidOutputStream() << header << ':' << str << ':' << resultCode << '}'; 
    }
    
    void printCanResult(const std::string &header, const CanMessage &msg, int resultCode) 
    { 
        *getCurrentValidOutputStream() << header << ':' << msg.toString() << ':' << resultCode << '}'; 
    }
    
    void printBlankCanResult(const std::string &header, int resultCode) 
    { 
        *getCurrentValidOutputStream() << header << ':' << resultCode << '}'; 
    } 

    void canInit()
    {
        using namespace ArduinoPCStrings;
        unsigned int startTime = millis();
        unsigned int endTime = millis();
        if (canController) {
            while (canController->begin(CAN_500KBPS) != CAN_OK) {
                endTime = millis();
                if ((endTime - startTime) >= CAN_CONNECTION_TIMEOUT) {
                    return;
                }
            }
            canBusInitialized = true;
        }
    }

    void canInitRequest()
    {
        using namespace ArduinoPCStrings;
        if (canBusInitialized) {
            printSingleResult(CAN_INIT_HEADER, OPERATION_SUCCESS);
            return;
        } else {
            canInit();
        }
        if (canBusInitialized) {
            printSingleResult(CAN_INIT_HEADER, OPERATION_SUCCESS);
        } else {
            printSingleResult(CAN_INIT_HEADER, OPERATION_FAILURE);
        }
        
    }

    void canReadRequest(bool autoUp)
    {
        using namespace ArduinoPCStrings;
        if (!canBusInitialized) {
            canInit();
        }
        unsigned char receivedPacketLength{0};
        unsigned char pack[8]{0, 0, 0, 0, 0, 0, 0, 0};
        uint32_t canID{0}; 
    
        if (CAN_MSGAVAIL == canController->checkReceive()) {
            canController->readMsgBuf(&receivedPacketLength, pack);
            canID = canController->getCanId();
            if ((positiveCanMasks.size() == 0) && (negativeCanMasks.size() == 0)) {
                CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS);
            } else {
                if (positiveCanMasks.size() == 0) {
                    if (negativeCanMasks.find(canID) == negativeCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS);
                    }
                }
                if (negativeCanMasks.size() == 0) {
                    if (positiveCanMasks.find(canID) != positiveCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS);
                    }
                }
                if (positiveCanMasks.find(canID) != positiveCanMasks.end()) {
                    if (negativeCanMasks.find(canID) == negativeCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS);
                    }
                }
            }
        } else if (!autoUp) {
            printBlankCanResult(CAN_READ_HEADER, OPERATION_SUCCESS);
        }
    }

    void canWriteRequest(const std::string &str, bool once)
    {
        using namespace ArduinoPCStrings;
        if (!canBusInitialized) {
            canInit();
        }
        CanMessage msg{parseCanMessage(str)};
        if (msg.toString() == "") {
            printSingleResult(CAN_WRITE_HEADER, OPERATION_FAILURE);
            return;
        }
        if (!once) {
            addLastCanMessage(msg);
        }
        sendCanMessage(msg);
        printCanResult(CAN_WRITE_HEADER, msg.toString(), OPERATION_SUCCESS);
    }

    void addPositiveCanMaskRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        if ((positiveCanMasks.insert(maybeID)).second) {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
        } else {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_KIND_OF_SUCCESS);
        }
    }

    void removePositiveCanMaskRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(REMOVE_POSITIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        positiveCanMasks.erase(maybeID);
        printTypeResult(REMOVE_POSITIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
    }

    void addNegativeCanMaskRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        if ((negativeCanMasks.insert(maybeID)).second) {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
        } else {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_KIND_OF_SUCCESS);
        }
    }

    void removeNegativeCanMaskRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(REMOVE_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        negativeCanMasks.erase(maybeID);
        printTypeResult(REMOVE_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
    }

    void canLiveUpdateRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        if (str == "") {
            printTypeResult(CAN_LIVE_UPDATE_HEADER, OPERATION_FAILURE, OPERATION_FAILURE);
            return;
        }
        std::pair<int, std::string> canState{parseToCanState(str)};
        if (canState.first == OPERATION_FAILURE) {
            printTypeResult(CAN_LIVE_UPDATE_HEADER, canState.second, OPERATION_FAILURE);
        } else {
            canLiveUpdate = canState.first;
            printTypeResult(CAN_LIVE_UPDATE_HEADER, canState.second, OPERATION_SUCCESS);
        }
    }

    void currentCachedCanMessageByIdRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_FAILURE);
        }
        if (lastCanMessages.find(maybeID) == lastCanMessages.end()) {
            printBlankCanResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, OPERATION_SUCCESS);
        } else {
            printCanResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, lastCanMessages.find(maybeID)->second, OPERATION_SUCCESS);
        }
    }

    void clearCurrentMessageByIdRequest(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        using namespace FirmwareUtilities;
        uint32_t maybeID{CanMessage::parseCanID(str)};
        if (maybeID == 0) {
            printTypeResult(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_FAILURE);
        }
        lastCanMessages.erase(maybeID);
        printTypeResult(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_SUCCESS);
    }

    void currentCachedCanMessagesRequest()
    {
        for (auto &it : lastCanMessages) {
            printCanResult(CURRENT_CAN_MESSAGES_HEADER, it.second, OPERATION_SUCCESS);
        }
    }
        
    void clearCanMessagesRequest()
    {
        lastCanMessages.clear();
        printSingleResult(CLEAR_CAN_MESSAGES_HEADER, OPERATION_SUCCESS);
    }

    void currentPositiveCanMasksRequest()
    {
        for (auto &it :positiveCanMasks) {
            printTypeResult(CURRENT_POSITIVE_CAN_MASKS_HEADER, it, OPERATION_SUCCESS);
        }
    }

    void currentNegativeCanMasksRequest()
    {
        for (auto &it : negativeCanMasks) {
            printTypeResult(CURRENT_NEGATIVE_CAN_MASKS_HEADER, it, OPERATION_SUCCESS);
        }
    }

    void allCurrentCanMasksRequest()
    {
        currentPositiveCanMasksRequest(coutIndex);
        currentNegativeCanMasksRequest(coutIndex);
    }

    void clearPositiveCanMasksRequest()
    {
        positiveCanMasks.clear();
        printSingleResult(CLEAR_POSITIVE_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    void clearNegativeCanMasksRequest()
    {
        negativeCanMasks.clear();
        printSingleResult(CLEAR_NEGATIVE_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    void clearAllCanMasksRequest()
    {
        positiveCanMasks.clear();
        negativeCanMasks.clear();
        printSingleResult(CLEAR_ALL_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    void addLastCanMessage(const CanMessage &msg)
    {
        std::map<uint32_t, CanMessage>::iterator iter{lastCanMessages.find(msg.id())};
        if (iter != lastCanMessages.end()) {
            iter->second = msg;
        } else {
            lastCanMessages.insert(std::pair<uint32_t, CanMessage>(msg.id(), msg));
        }
    }
    CanMessage parseCanMessage(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        std::vector<std::string> rawMsg{parseToVector(str, ':')};
        if (rawMsg.size() != CAN_WRITE_REQUEST_SIZE) {
            return CanMessage{};
        }
        CanMessage returnMessage;
        int i{0};
        returnMessage.setFrame(CAN_FRAME);
        returnMessage.setLength(CAN_MESSAGE_LENGTH);
        for (auto &it : rawMsg) {
            if (i++ == 0) {
                returnMessage.setID(CanMessage::parseCanID(it));
            } else {
                returnMessage.setDataPacketNthByte(i-2, CanMessage::parseCanByte(it));
            }
        }
        return returnMessage;
    }

    std::pair<int, std::string> parseToCanState(const std::string &str)
    {
        using namespace ArduinoPCStrings;
        int i{0};
        while (static_cast<std::string>(DIGITAL_STATE_HIGH_IDENTIFIERS[i]) != static_cast<std::string>(CHAR_ARRAY_TERMINATOR)) {
            if (toLowercase(str) == static_cast<std::string>(DIGITAL_STATE_HIGH_IDENTIFIERS[i++])) {
                return std::pair<int, std::string>(HIGH, DIGITAL_STATE_HIGH_STRING); 
            }
        }
        i = 0;
        while (static_cast<std::string>(DIGITAL_STATE_LOW_IDENTIFIERS[i]) != static_cast<std::string>(CHAR_ARRAY_TERMINATOR)) {
            if (toLowercase(str) == static_cast<std::string>(DIGITAL_STATE_LOW_IDENTIFIERS[i++])) {
                return std::pair<int, std::string>(LOW, DIGITAL_STATE_LOW_STRING);
            }
        }
        return std::pair<int, std::string>(OPERATION_FAILURE, str);
    }

    void sendCanMessage(const CanMessage &msg)
    {
        unsigned char send[8]{0, 0, 0, 0, 0, 0, 0, 0};
        msg.dataPacket().toBasicArray(send);
        canController->sendMsgBuf(msg.id(), msg.frame(), msg.length(), send);
    }

#endif
