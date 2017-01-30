#include "include/debug.h"
#include "include/arduinoinit.h"
#include "include/gpio.h"
#include "include/serialportbase.h"
#include "include/firmwareutilities.h"
#include "include/arduinopcstrings.h"
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#if defined(__HAVE_CAN_BUS__)
    #include "include/mcp_can.h"
    #include "include/candatapacket.h"
    #include "include/canmessage.h"
#endif

#define __DEBUG__ true

static Debug debug{__DEBUG__};

using namespace ArduinoPCStrings;
using namespace FirmwareUtilities;

#define ID_WIDTH 3
#define MESSAGE_WIDTH 2
#define MAXIMUM_SERIAL_READ_SIZE 175
#define PIN_OFFSET 2
#define NEXT_SERIAL_PORT_UNAVAILABLE -1
#define SERIAL_BAUD 115200L
#define SERIAL_TIMEOUT 0

#define OPERATION_FAILURE -1
#define INVALID_PIN -1
#define STATE_FAILURE -1
#define OPERATION_SUCCESS 1
#define OPERATION_KIND_OF_SUCCESS 2
#define OPERATION_PIN_USED_BY_SERIAL_PORT 3
#define PIN_PLACEHOLDER 1
#define SOFT 1

static const bool NO_BROADCAST{false};
static const bool BROADCAST{true};

static const char OPENING_CHARACTER{'{'};
static const char CLOSING_CHARACTER{'}'};
static const char *LINE_ENDING{"\r"};
static const char ITEM_SEPARATOR{':'}; 
static const char DIGITAL_STATE_LOW_IDENTIFIER{'0'};
static const char DIGITAL_STATE_HIGH_IDENTIFIER{'1'};
static const char ANALOG_IDENTIFIER_CHAR{'A'};

void broadcastString(const char *str);
#if defined(__HAVE_CAN_BUS__)
    void printCanResult(const char *header, const char *str, int resultCode, bool broadcast = false);
    void printCanResult(const char *header, const CanMessage &msg, int resultCode, bool broadcast = false);
    void printBlankCanResult(const char *header, int resultCode); 
#endif

#define SMALL_BUFFER_SIZE 255

void handleSerialString(const char *str);

void digitalReadRequest(const char *str, bool soft);
void digitalWriteRequest(const char *str);
void digitalWriteAllRequest(const char *str);
void analogReadRequest(const char *str);
void analogWriteRequest(const char *str);
void softAnalogReadRequest(const char *str);
void addSoftwareSerialRequest(const char *str);
void addHardwareSerialRequest(const char *str);
void removeSoftwareSerialRequest(const char *str);
void removeHardwareSerialRequest(const char *str);

void pinTypeRequest(const char *str);
void pinTypeChangeRequest(const char *str);

void changeAToDThresholdRequest(const char *str);
void currentAToDThresholdRequest();

void arduinoTypeRequest();
void firmwareVersionRequest();
void canBusEnabledRequest();
void ioReportRequest();

SerialPortBase *getCurrentValidOutputStream();

bool isValidAnalogPinIdentifier(const char *str);
bool isValidPinIdentifier(const char *str);
bool isValidDigitalStateIdentifier(const char *str);
bool isValidAnalogStateIdentifier(const char *str);
bool isValidPwmPinIdentifier(const char *str);
bool isValidPinTypeIdentifier(const char *str);

bool checkValidIOChangeRequest(IOType ioType, int pinNumber);
bool checkValidRequestString(const char *header, const char *checkStr); 

bool isValidDigitalOutputPin(int pinNumber);
bool isValidDigitalInputPin(int pinNumber);
bool isValidAnalogOutputPin(int pinNumber);
bool isValidAnalogInputPin(int pinNumber);
int parseAnalogPin(const char *str);
int parseToState(const char *str);

int parseToAnalogState(const char *str);
IOType parseIOType(const char *str);
int parseToDigitalState(const char *str);
int parsePin(const char *str);
void populateGpioMap();

int getIOTypeString(IOType type, char *out, size_t maximumSize);
int getSerialPinIOTypeString(int pinNumber, char *out, size_t maximumSize);
int analogPinFromNumber(int number, char *out, size_t maximumSize);
GPIO *gpioPinByPinNumber(int pinNumber);
bool pinInUseBySerialPort(int pinNumber);

#if defined(ARDUINO_AVR_UNO)
    static const PROGMEM int AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, -1};
    static const PROGMEM int AVAILABLE_GENERAL_PINS[]{2, 4, 7, 8, 12, 13, -1};
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
    void canWriteRequest(const char *canMessage, bool once);
    void addNegativeCanMaskRequest(const char *canMask);
    void removePositiveCanMaskRequest(const char *canMask);
    void canLiveUpdateRequest(const char *str);
    void clearCurrentMessageByIdRequest(const char *str);
    void currentCachedCanMessageByIdRequest(const char *str);
    void clearCanMessagesRequest();
    void currentPositiveCanMasksRequest();
    void currentNegativeCanMasksRequest();
    void allCurrentCanMasksRequest();
    void clearPositiveCanMasksRequest();
    void clearNegativeCanMasksRequest();
    void clearCanMasksRequest();
    void currentCachedCanMessagesRequest();
    void clearAllCanMasksRequest();
    void sendCanMessage(const CanMessage &msg);
    #define SPI_CS_PIN 9 
    MCP_CAN *canController{new MCP_CAN(SPI_CS_PIN)};
    #define CAN_CONNECTION_TIMEOUT 1000
    #define CAN_WRITE_REQUEST_SIZE 9
    static bool canBusInitialized{false};
    static bool canLiveUpdate{false};
    CanMessage parseCanMessage(const char *str);
    static std::map<uint32_t, CanMessage> lastCanMessages;
    static std::set<uint32_t> positiveCanMasks;
    static std::set<uint32_t> negativeCanMasks;
    void addLastCanMessage(const CanMessage &msg);
    #define CAN_MESSAGE_LENGTH 8
    #define CAN_FRAME 0
    #define CAN_COMMUNICATION_DOWN_TIME 100
    static int canCommunicationStartTime{0};
    static int canCommunicationEndTime{0};
#endif

#if defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    #define SOFTWARE_SERIAL_ENUM_OFFSET 4
    #define NUMBER_OF_HARDWARE_SERIAL_PORTS 4
    #define MAXIMUM_SOFTWARE_SERIAL_PORTS 4

    static SerialPortBase *hardwareSerialPorts[NUMBER_OF_HARDWARE_SERIAL_PORTS] {
        new HardwareSerialPort{&Serial,   0,   1,  SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial1,  19,  18, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial2,  17,  16, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial3,  15,  14, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING}
    };
    
    static SerialPortBase *softwareSerialPorts[MAXIMUM_SOFTWARE_SERIAL_PORTS] {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

#else
    #define SOFTWARE_SERIAL_ENUM_OFFSET 1
    #define NUMBER_OF_HARDWARE_SERIAL_PORTS 1
    #define MAXIMUM_SOFTWARE_SERIAL_PORTS 1

    static SerialPortBase *hardwareSerialPorts[NUMBER_OF_HARDWARE_SERIAL_PORTS] {
        new HardwareSerialPort{&Serial, 0,  1, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING}
    };
    
    static SerialPortBase *softwareSerialPorts[MAXIMUM_SOFTWARE_SERIAL_PORTS] {
        nullptr
    };

#endif
static int softwareSerialPortIndex{0};
static GPIO *gpioPins[NUMBER_OF_PINS-1];
/*
template <typename T>
int findInArray(T itemToFind, T *arrayToLook, size_t sizeOfArray)
{
    for (size_t i = 0; i < sizeOfArray; i++) {
        if (arrayToLook + i) {
            if (itemToFind == arrayToLook[i]) {
                return i;
            }
        }
    }
    return -1; 
} 
*/

void initializeSerialPorts();
void announceStartup();
SerialPortBase *getHardwareCout(int coutIndex);
SerialPortBase *getSoftwareCout(int coutIndex);
static SerialPortBase *currentSerialStream{hardwareSerialPorts[0]};
SerialPortBase *defaultNativePort{hardwareSerialPorts[0]};

template <typename Header, typename PinNumber, typename State, typename ResultCode> inline void printResult(const Header &header, PinNumber pinNumber, State state, ResultCode resultCode)
{    
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << pinNumber << ITEM_SEPARATOR << state << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
}

template <typename Header, typename ResultCode> inline void printSingleResult(const Header &header, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
}

template <typename Header, typename Type, typename ResultCode> inline void printTypeResult(const Header &header, Type type, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << type << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING; 
}

template <typename Parameter> inline void printString(const Parameter &parameter)
{
    *getCurrentValidOutputStream() << parameter << LINE_ENDING;
}

int main()
{
    arduinoInit();
    initializeSerialPorts();
    announceStartup();
    populateGpioMap();

    bool ledState{true};
    pinMode(13, OUTPUT);
    digitalWrite(13, ledState);
    long long blinkStartTime{millis()};
    long long blinkEndTime{millis()};
    long long blinkElapsedTime{blinkEndTime - blinkStartTime};
    int blinkCountThreshold{4};
    int blinkCounter{0};
    int blinkTimeout{500};
    while (true) {
        for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
            SerialPortBase *it{nullptr};
            if (hardwareSerialPorts + i) {
                if (hardwareSerialPorts[i]) {
                    it = hardwareSerialPorts[i];
                }
            }
            if (!it) {
                continue;
            }
            if ((it->isEnabled()) && (it->available() != 0)) {
                char buffer[MAXIMUM_SERIAL_READ_SIZE];
                int serialRead{it->readLine(buffer, MAXIMUM_SERIAL_READ_SIZE)};
                if (serialRead > 0) {
                    debug.print("hardware serial port at i = ");
                    debug.print(i);
                    debug.print(" read ");
                    debug.print(serialRead);
                    debug.print(" bytes (read '");
                    debug.print(buffer);
                    debug.println("')");
                    //currentSerialStream = it;
                    handleSerialString(buffer);
                }
            }
        }
        for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
            SerialPortBase *it{nullptr};
            if (softwareSerialPorts + i) {
                if (softwareSerialPorts[i]) {
                    it = softwareSerialPorts[i];
                }
            }
            if (!it) {
                continue;
            }
            if ((it->isEnabled()) && (it->available() != 0)) {
                char buffer[MAXIMUM_SERIAL_READ_SIZE];
                int serialRead{it->readLine(buffer, MAXIMUM_SERIAL_READ_SIZE)};
                if (serialRead > 0) {
                    debug.print("hardware serial port at i = ");
                    debug.print(i);
                    debug.print(" read ");
                    debug.print(serialRead);
                    debug.print(" bytes (read '");
                    debug.print(buffer);
                    debug.println("')");
                    currentSerialStream = it;
                    //handleSerialString(buffer);
                }
            }
        }
        #if defined(__HAVE_CAN_BUS__)
            if (canLiveUpdate) {
                canReadRequest(canLiveUpdate);
            }
            bool canSend{false};
            for (auto &it : lastCanMessages) {
                if (it.second.toString() != "") {
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

        blinkEndTime = millis();
        blinkElapsedTime = (blinkEndTime - blinkStartTime);
        if (blinkElapsedTime >= blinkTimeout) {
            blinkCounter++;
            blinkElapsedTime = 0;
            blinkStartTime = millis();
            if (blinkCounter >= blinkCountThreshold) {
                ledState = !ledState;
                digitalWrite(13, ledState);
            }
        }

        if (serialEventRun) serialEventRun();
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(gpioPins); i++) {
        if (gpioPins + i) {
            if (gpioPins[i]) {
                delete gpioPins[i];
            }
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                delete hardwareSerialPorts[i];
            }
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                delete softwareSerialPorts[i];
            }
        }
    }
    #if defined(__HAVE_CAN_BUS__)
        delete canController;
    #endif
    return 0;
}

void handleSerialString(const char *str)
{
    debug.print("str = ");
    debug.println(str);
    return;
    /*
    if (!str) {
        return;
    } else if (strlen(str) == 0) {
        return;
    }
    */
    char requestString[SMALL_BUFFER_SIZE];
    int substringResult{0};
    (void)substringResult;
    if (startsWith(str, ANALOG_READ_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(ANALOG_READ_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(ANALOG_READ_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ANALOG_READ_HEADER)+1, requestString, SMALL_BUFFER_SIZE); 
            analogReadRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ANALOG_WRITE_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(ANALOG_WRITE_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(ANALOG_WRITE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ANALOG_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            analogWriteRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CHANGE_A_TO_D_THRESHOLD_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(CHANGE_A_TO_D_THRESHOLD_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(CHANGE_A_TO_D_THRESHOLD_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CHANGE_A_TO_D_THRESHOLD_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            changeAToDThresholdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_SOFTWARE_SERIAL_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(ADD_SOFTWARE_SERIAL_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(ADD_SOFTWARE_SERIAL_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ADD_SOFTWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addSoftwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_HARDWARE_SERIAL_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(ADD_HARDWARE_SERIAL_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(ADD_HARDWARE_SERIAL_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ADD_HARDWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addHardwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }  
    } else if (startsWith(str, DIGITAL_READ_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(DIGITAL_READ_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(DIGITAL_READ_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(DIGITAL_READ_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            digitalReadRequest(requestString, false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_ALL_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(DIGITAL_WRITE_ALL_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(DIGITAL_WRITE_ALL_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(DIGITAL_WRITE_ALL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            digitalWriteAllRequest(requestString);
        } else {
            printTypeResult(DIGITAL_WRITE_ALL_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(DIGITAL_WRITE_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(DIGITAL_WRITE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(DIGITAL_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            digitalWriteRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(PIN_TYPE_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(PIN_TYPE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(PIN_TYPE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            pinTypeRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_CHANGE_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(PIN_TYPE_CHANGE_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(PIN_TYPE_CHANGE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(PIN_TYPE_CHANGE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            pinTypeChangeRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_SOFTWARE_SERIAL_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(REMOVE_SOFTWARE_SERIAL_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(REMOVE_SOFTWARE_SERIAL_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(REMOVE_SOFTWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeSoftwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_HARDWARE_SERIAL_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(REMOVE_HARDWARE_SERIAL_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(REMOVE_HARDWARE_SERIAL_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(REMOVE_HARDWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeHardwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
    } else if (startsWith(str, SOFT_DIGITAL_READ_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(SOFT_DIGITAL_READ_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(SOFT_DIGITAL_READ_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(SOFT_DIGITAL_READ_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            digitalReadRequest(requestString, true);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, SOFT_ANALOG_READ_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(SOFT_ANALOG_READ_HEADER);
        debug.println("'");
        delay(1000);
        if (checkValidRequestString(SOFT_ANALOG_READ_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(SOFT_ANALOG_READ_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            softAnalogReadRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, FIRMWARE_VERSION_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(FIRMWARE_VERSION_HEADER);
        debug.println("'");
        delay(1000);
        firmwareVersionRequest();
    } else if (startsWith(str, IO_REPORT_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(IO_REPORT_HEADER);
        debug.println("'");
        delay(1000);
        ioReportRequest();
    } else if (startsWith(str, CURRENT_A_TO_D_THRESHOLD_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(CURRENT_A_TO_D_THRESHOLD_HEADER);
        debug.println("'");
        delay(1000);
        currentAToDThresholdRequest();
    } else if (startsWith(str, ARDUINO_TYPE_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(ARDUINO_TYPE_HEADER);
        debug.println("'");
        delay(1000);
        arduinoTypeRequest();
    } else if (startsWith(str, CAN_BUS_ENABLED_HEADER)) {
        debug.print("Serial string starts with '");
        debug.print(CAN_BUS_ENABLED_HEADER);
        debug.println("'");
        delay(1000);
        canBusEnabledRequest();
#if defined(__HAVE_CAN_BUS__)
    } else if (startsWith(str, CAN_INIT_HEADER)) {
        canInitRequest();
    } else if (startsWith(str, CAN_READ_HEADER)) {
        canReadRequest(false);
    } else if (startsWith(str, CAN_WRITE_ONCE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_ONCE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CAN_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canWriteRequest(requestString, true);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CAN_WRITE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CAN_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canWriteRequest(requestString, false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }     
    } else if (startsWith(str, CAN_LIVE_UPDATE_HEADER)) {
        if (checkValidRequestString(CAN_LIVE_UPDATE_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CAN_LIVE_UPDATE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canLiveUpdateRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CLEAR_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CLEAR_CAN_MESSAGE_BY_ID_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            clearCurrentMessageByIdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
    } else if (startsWith(str, CURRENT_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CURRENT_CAN_MESSAGE_BY_ID_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(CURRENT_CAN_MESSAGE_BY_ID_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            currentCachedCanMessageByIdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_NEGATIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_NEGATIVE_CAN_MASK_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(REMOVE_NEGATIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeNegativeCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_POSITIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_POSITIVE_CAN_MASK_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(REMOVE_POSITIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removePositiveCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_POSITIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(ADD_POSITIVE_CAN_MASK_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ADD_POSITIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addPositiveCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_NEGATIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(ADD_NEGATIVE_CAN_MASK_HEADER, str)) {
            substringResult = FirmwareUtilities::substring(str, strlen(ADD_NEGATIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addNegativeCanMaskRequest(requestString);
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
        debug.println("Serial string is invalid");
        delay(1000);
        printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
    }
}

void addSoftwareSerialRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }

    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);
    
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
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                if ((rxPinNumber == softwareSerialPorts[i]->rxPin()) && (txPinNumber == softwareSerialPorts[i]->txPin())) {
                    softwareSerialPorts[i]->setEnabled(true);
                    softwareSerialPorts[i]->initialize();
                    printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
                }
            }
        }
    }
    if (isValidSoftwareSerialAddition(rxPinNumber, txPinNumber)) {
        softwareSerialPorts[softwareSerialPortIndex++] = new SoftwareSerialPort{rxPinNumber, txPinNumber, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING};
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
    } else {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
    }
}

void addHardwareSerialRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
    
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if ((rxPinNumber == hardwareSerialPorts[i]->rxPin()) && (txPinNumber == hardwareSerialPorts[i]->txPin())) {
                    hardwareSerialPorts[i]->setEnabled(true);
                    hardwareSerialPorts[i]->initialize();
                    printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
                }
            }
        }
    }
    printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

void removeSoftwareSerialRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                if ((rxPinNumber == softwareSerialPorts[i]->rxPin()) && (txPinNumber == softwareSerialPorts[i]->txPin())) {
                    softwareSerialPorts[i]->setEnabled(false);
                    printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
                }
            }
        }
    }
    printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

void removeHardwareSerialRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if ((rxPinNumber == hardwareSerialPorts[i]->rxPin()) && (txPinNumber == hardwareSerialPorts[i]->txPin())) {
                    hardwareSerialPorts[i]->setEnabled(false);
                    printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
                }
            }
        }
    }
    printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

bool isValidSoftwareSerialAddition(int rxPinNumber, int txPinNumber)
{
    if (ARRAY_SIZE(softwareSerialPorts) >= MAXIMUM_SOFTWARE_SERIAL_PORTS) {
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

void changeAToDThresholdRequest(const char *str)
{
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
    *getCurrentValidOutputStream() << IO_REPORT_HEADER << CLOSING_CHARACTER;
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        GPIO *gpioPin{nullptr};
        if (gpioPins + i) {
            if (gpioPins[i]) {
                gpioPin = gpioPins[i];
            } else {
                continue;
            }
        } else {
            continue;
        }

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
            char analogPinString[SMALL_BUFFER_SIZE];
            char ioTypeString[SMALL_BUFFER_SIZE];
            int result{analogPinFromNumber(gpioPin->pinNumber(), analogPinString, SMALL_BUFFER_SIZE)};
            int secondResult{getIOTypeString(gpioPin->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
            (void)result;
            (void)secondResult;
            *getCurrentValidOutputStream() << OPENING_CHARACTER << analogPinString << ITEM_SEPARATOR << ioTypeString << ITEM_SEPARATOR << state << CLOSING_CHARACTER;
        } else {
            char ioTypeString[SMALL_BUFFER_SIZE];
            int result{getIOTypeString(gpioPin->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
            (void)result;
            *getCurrentValidOutputStream() << OPENING_CHARACTER << gpioPin->pinNumber() << ITEM_SEPARATOR << ioTypeString << ITEM_SEPARATOR << state << CLOSING_CHARACTER;
        }
    }
    *getCurrentValidOutputStream() << IO_REPORT_END_HEADER << CLOSING_CHARACTER << LINE_ENDING;
}

void digitalReadRequest(const char *str, bool soft)
{
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

void digitalWriteRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybePin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybePin, SMALL_BUFFER_SIZE)};
    (void)result;
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
    char maybeState[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeState, SMALL_BUFFER_SIZE);
    int state{parseToDigitalState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, maybeState, OPERATION_FAILURE);
        return;
    } 
    gpioPinByPinNumber(pinNumber)->g_digitalWrite(state);
    printResult(DIGITAL_WRITE_HEADER, maybePin, maybeState, OPERATION_SUCCESS);
}

void digitalWriteAllRequest(const char *str)
{
    *getCurrentValidOutputStream() << DIGITAL_WRITE_ALL_HEADER;
    
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeState[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybeState, SMALL_BUFFER_SIZE)};
    (void)result;

    int state{parseToDigitalState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printTypeResult(DIGITAL_WRITE_ALL_HEADER, maybeState, OPERATION_FAILURE);
        return;
    }
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        if (gpioPins + i) {
            if (gpioPins[i]) {
                GPIO *gpioPin{gpioPins[i]};
                if (gpioPin->ioType() == IOType::DIGITAL_OUTPUT) {
                    gpioPin->g_digitalWrite(state);
                    if (isValidAnalogInputPin(gpioPin->pinNumber())) {
                        char analogPinString[SMALL_BUFFER_SIZE];
                        int result{analogPinFromNumber(gpioPin->pinNumber(), analogPinString, SMALL_BUFFER_SIZE)};
                        (void)result;
                        *getCurrentValidOutputStream() << ITEM_SEPARATOR << analogPinString;
                    } else {
                        *getCurrentValidOutputStream() << ITEM_SEPARATOR << gpioPin->pinNumber();
                    }
                }
            }
        }
    }
    *getCurrentValidOutputStream() << ITEM_SEPARATOR << state << ITEM_SEPARATOR << OPERATION_SUCCESS << CLOSING_CHARACTER << LINE_ENDING;
}

void analogReadRequest(const char *str)
{
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

void analogWriteRequest(const char *str)
{
    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybePin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybePin, SMALL_BUFFER_SIZE)};
    (void)result;
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
    
    char maybeState[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeState, SMALL_BUFFER_SIZE);
    int state{parseToAnalogState(maybeState)};
    if (state == OPERATION_FAILURE) {
        printResult(ANALOG_WRITE_HEADER, maybePin, maybeState, OPERATION_FAILURE);
    } else {
        gpioPinByPinNumber(pinNumber)->g_analogWrite(state);
        printResult(ANALOG_WRITE_HEADER, maybePin, state, OPERATION_SUCCESS);
    }
}

void pinTypeRequest(const char *str)
{
    int pinNumber = parsePin(str);
    if (pinNumber == INVALID_PIN) {
        printResult(PIN_TYPE_HEADER, str, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        char temp[SMALL_BUFFER_SIZE];
        if (!getSerialPinIOTypeString(pinNumber, temp, SMALL_BUFFER_SIZE)) {
            printResult(PIN_TYPE_HEADER, str, pinNumber, OPERATION_SUCCESS);
        } else {
            printResult(PIN_TYPE_HEADER, str, temp, OPERATION_SUCCESS);
        }
        return;
    }
    char ioTypeString[SMALL_BUFFER_SIZE];
    int result{getIOTypeString(gpioPinByPinNumber(pinNumber)->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
    (void)result;
    printResult(PIN_TYPE_HEADER, str, ioTypeString, OPERATION_SUCCESS);
}

void pinTypeChangeRequest(const char *str)
{   

    int foundPosition{FirmwareUtilities::positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybePin[SMALL_BUFFER_SIZE];
    int result{FirmwareUtilities::substring(str, 0, foundPosition, maybePin, SMALL_BUFFER_SIZE)};
    (void)result;
    int pinNumber{parsePin(maybePin)};

    if (pinNumber == INVALID_PIN) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }

    char maybeType[SMALL_BUFFER_SIZE];
    result = FirmwareUtilities::substring(str, foundPosition+1, maybeType, SMALL_BUFFER_SIZE);
    IOType type{parseIOType(maybeType)};
    if (type == IOType::UNSPECIFIED) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, maybeType, OPERATION_FAILURE);
        return;
    }

    char ioTypeString[SMALL_BUFFER_SIZE];
    result = getIOTypeString(type, ioTypeString, SMALL_BUFFER_SIZE);
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, maybePin, ioTypeString, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    if (!checkValidIOChangeRequest(type, pinNumber)) {
        printResult(PIN_TYPE_CHANGE_HEADER, maybePin, ioTypeString, OPERATION_FAILURE);
        return;
    }
    gpioPinByPinNumber(pinNumber)->setIOType(type);
    printResult(PIN_TYPE_CHANGE_HEADER, maybePin, ioTypeString, OPERATION_SUCCESS);
}

void softAnalogReadRequest(const char *str)
{
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
    char stringToPrint[SMALL_BUFFER_SIZE];
    strcpy(stringToPrint, HEARTBEAT_HEADER);

    size_t stringLength{strlen(stringToPrint)};
    stringToPrint[stringLength] = CLOSING_CHARACTER;
    stringToPrint[stringLength + 1] = '\0';

    printString(stringToPrint);
    
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
        gpioPins[pinNumber] = new GPIO(pinNumber, IOType::DIGITAL_INPUT_PULLUP);
    } while (true);
    i = 0;
    do {
        int pinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins[pinNumber] = new GPIO(pinNumber, IOType::ANALOG_INPUT);
    } while (true);
    i = 0;
    do {
        int pinNumber{pgm_read_word_near(AVAILABLE_GENERAL_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins[pinNumber] = new GPIO(pinNumber, IOType::DIGITAL_INPUT_PULLUP);
    } while (true);
}

bool checkValidRequestString(const char *header, const char *checkStr)
{
    if ((!header) || (!checkStr)) {
        return false;
    }
    return (strlen(checkStr) > (strlen(header) + 1));
}


bool isValidDigitalOutputPin(int pinNumber)
{
#if defined(ARDUINO_AVR_NANO)
    if ((pinNumber == A6) || (pinNumber == A7)) {
        return false;
    }
#endif
    for (unsigned int i = 0; i < ARRAY_SIZE(gpioPins); i++) {
        if (gpioPins + i) {
            if (gpioPins[i]) {
                if (pinNumber == gpioPins[i]->pinNumber()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isValidDigitalInputPin(int pinNumber)
{
    return isValidDigitalOutputPin(pinNumber);
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

int getIOTypeString(IOType ioType, char *out, size_t maximumSize)
{
    if (ioType == IOType::DIGITAL_INPUT) {
        strncpy(out, DIGITAL_INPUT_IDENTIFIER, maximumSize);
        return strlen(out);
    } else if (ioType == IOType::DIGITAL_OUTPUT) {
        strncpy(out, DIGITAL_OUTPUT_IDENTIFIER, maximumSize);
        return strlen(out);
    } else if (ioType == IOType::ANALOG_INPUT) {
        strncpy(out, ANALOG_INPUT_IDENTIFIER, maximumSize);
        return strlen(out);
    } else if (ioType == IOType::ANALOG_OUTPUT) {
        strncpy(out, ANALOG_OUTPUT_IDENTIFIER, maximumSize);
        return strlen(out);
    } else if (ioType == IOType::DIGITAL_INPUT_PULLUP) {
        strncpy(out, DIGITAL_INPUT_PULLUP_IDENTIFIER, maximumSize);
        return strlen(out);
    } else {
        strncpy(out, UNSPECIFIED_IO_TYPE_IDENTIFIER, maximumSize);
        return OPERATION_FAILURE;
    }
}

int parsePin(const char *str)
{
    if (FirmwareUtilities::startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
        return parseAnalogPin(str);
    } else if (isValidPinIdentifier(str)) {
        return atoi(str);
    } else {
        return INVALID_PIN;
    }
}

IOType parseIOType(const char *str)
{
    if (strcmp(str, DIGITAL_INPUT_IDENTIFIER) == 0) {
        return IOType::DIGITAL_INPUT;
    } else if (strcmp(str, DIGITAL_OUTPUT_IDENTIFIER) == 0) {
        return IOType::DIGITAL_OUTPUT;
    } else if (strcmp(str, ANALOG_INPUT_IDENTIFIER) == 0) {
        return IOType::ANALOG_INPUT;
    } else if (strcmp(str, ANALOG_OUTPUT_IDENTIFIER) == 0) {
        return IOType::ANALOG_OUTPUT;
    } else if (strcmp(str, DIGITAL_INPUT_PULLUP_IDENTIFIER) == 0) {
        return IOType::DIGITAL_INPUT_PULLUP;
    } else {
        return IOType::UNSPECIFIED;
    }
}

bool isValidAnalogStateIdentifier(const char *str)
{
    if (!str) {
        return false;
    }
    if (strlen(str) == 1) {
        if (str[0] == DIGITAL_STATE_HIGH_IDENTIFIER) {
            return true;
        }
    }
    int returnVal{atoi(str)};
    return (returnVal != 0); 
}

bool isValidDigitalStateIdentifier(const char *str)
{   
    if (!str) {
        return false;
    }
    if (strlen(str) == 1) {
        return ((str[0] == DIGITAL_STATE_LOW_IDENTIFIER) || (str[0] == DIGITAL_STATE_HIGH_IDENTIFIER)); 
    } else {
        return false;
    }
}

int parseToDigitalState(const char *str)
{
    if (!str) {
        return false;
    } else if (!isValidDigitalStateIdentifier(str)) {
        return OPERATION_FAILURE;
    } else if (str[0] == DIGITAL_STATE_HIGH_IDENTIFIER) {
        return HIGH;
    } else if (str[0] == DIGITAL_STATE_LOW_IDENTIFIER) {
        return LOW;
    } else {
        return OPERATION_FAILURE;
    }
    return false;
}

int parseToAnalogState(const char *str)
{
    if (!isValidAnalogStateIdentifier(str)) {
        return OPERATION_FAILURE;
    } else {
        int temp{atoi(str)};
        if (temp > GPIO::ANALOG_MAX) {
            temp = GPIO::ANALOG_MAX;
        } else if (temp < 0) {
            return OPERATION_FAILURE;
        }
        return temp;
    }
}

bool isValidPinIdentifier(const char *str)
{
    for (int i = 0; i < (NUMBER_OF_PINS - 1); i++) {
        if (gpioPins + i) {
            if (gpioPins[i]) {
                if (atoi(str) == static_cast<int>(gpioPins[i]->pinNumber())) {
                    return true;
                }
            }
        }
    }

    char buffer[strlen(str) + 1];
    memset(buffer, 0, sizeof buffer);
    strncpy(buffer, str, sizeof(buffer) - 1);

    for (unsigned int i =  0; i < ARRAY_SIZE(AVAILABLE_ANALOG_PINS); i++) {
        if (FirmwareUtilities::startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(i)) {
                return ANALOG_PIN_OFFSET + i + 1;
            }
        }
    }
    memset(buffer, 0, strlen(buffer) + 1);
    strncpy(buffer, str, sizeof(buffer) - 1);
    
    for (unsigned int i =  0; i < ARRAY_SIZE(AVAILABLE_ANALOG_PINS); i++) {
        if (FirmwareUtilities::startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(i)) {
                return true;
            }
        }
    }
    return false;
}

bool isValidPwmPinIdentifier(const char *str)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_PWM_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == atoi(str)) {
            return true;
        }
    } while (true);
}

bool isValidAnalogPinIdentifier(const char *str)
{
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            break;
        }
        if (tempPinNumber == atoi(str)) {
            return true;
        }
    } while (true);

    char buffer[strlen(str) + 1];
    memset(buffer, 0, sizeof buffer);
    strncpy(buffer, str, sizeof(buffer) - 1);

    for (unsigned int j = 0; j < ARRAY_SIZE(AVAILABLE_ANALOG_PINS); j++) {
        if (FirmwareUtilities::startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(j)) {
                return true;
            }
        }
    }
    return false;
}

bool isValidPinTypeIdentifier(const char *str)
{
    return ((strcmp(str, DIGITAL_INPUT_IDENTIFIER) == 0) ||
            (strcmp(str, DIGITAL_OUTPUT_IDENTIFIER) == 0) ||
            (strcmp(str, ANALOG_INPUT_IDENTIFIER) == 0) ||
            (strcmp(str, ANALOG_OUTPUT_IDENTIFIER) == 0) ||
            (strcmp(str, DIGITAL_INPUT_PULLUP_IDENTIFIER) == 0));
}

void initializeSerialPorts()
{
    Serial.begin(115200L);
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if (hardwareSerialPorts[i]->isEnabled()) {
                    (void)hardwareSerialPorts[i]->initialize();
                }
            }
        }
    }
}

void broadcastString(const char *str)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if (hardwareSerialPorts[i]->isEnabled()) {
                    *(hardwareSerialPorts[i]) << str << LINE_ENDING;
                }
            }
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                if (softwareSerialPorts[i]->isEnabled()) {
                    *(softwareSerialPorts[i]) << str << LINE_ENDING;
                }
            }
        }
    }
}

void announceStartup()
{
    char str[SMALL_BUFFER_SIZE];
    strcpy (str, INITIALIZATION_HEADER);

    size_t stringLength{strlen(str)};
    str[stringLength] = ITEM_SEPARATOR;
    str[stringLength + 1] = '\0';

    strcat (str, ARDUINO_TYPE);

    stringLength = strlen(str);
    str[stringLength] = ITEM_SEPARATOR;
    str[stringLength + 1] = '\0';

    strcat (str, FIRMWARE_VERSION);

    stringLength = strlen(str);
    str[stringLength] = CLOSING_CHARACTER;
    str[stringLength + 1] = '\0';

    strcat (str, LINE_ENDING);

    broadcastString(str);
}


GPIO *gpioPinByPinNumber(int pinNumber)
{
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        if (gpioPins + i) {
            if (gpioPins[i]) {
                if (gpioPins[i]->pinNumber() == pinNumber) {
                    return gpioPins[i];
                }
            }
        }
    }
    return nullptr;
}
int parseAnalogPin(const char *pinAlias)
{
    if (!pinAlias) {
        return 0;
    }
    char buffer[strlen(pinAlias + 1)];
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, pinAlias, sizeof(buffer) - 1);
    for (unsigned int i =  0; i < ARRAY_SIZE(AVAILABLE_ANALOG_PINS); i++) {
        if (FirmwareUtilities::startsWith(pinAlias, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(i)) {
                return ANALOG_PIN_OFFSET + i + 1;
            }
        }
    }
    int i{0};
    int maybePinNumber{parsePin(pinAlias)};
    if (maybePinNumber == INVALID_PIN) {
        return 0;
    }
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            break;
        }
        if (tempPinNumber == maybePinNumber) {
            return tempPinNumber;
        }
    } while (true);
    return 0;
}

int analogPinFromNumber(int pinNumber, char *out, size_t maximumSize)
{
    if (!out) {
        return 0;
    }
    int i{0};
    do {
        int tempPinNumber{pgm_read_word_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            return 0;
        }
        if (tempPinNumber == pinNumber) {
            char tempNumber[255];
            if (!FirmwareUtilities::toDecString(i-1, tempNumber, maximumSize)) {
                return 0;
            }
            char temp[SMALL_BUFFER_SIZE];
            temp[0] = ANALOG_IDENTIFIER_CHAR;
            temp[1] = '\0';
            strncpy(out, temp, maximumSize);
            strcat(out, tempNumber);
            return strlen(out);
        }
    } while (true);
}

bool pinInUseBySerialPort(int pinNumber)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if (hardwareSerialPorts[i]->isEnabled()) {
                    if (pinNumber == hardwareSerialPorts[i]->rxPin()) {
                        return true;
                    } else if (pinNumber == hardwareSerialPorts[i]->txPin()) {
                        return true;
                    }
                }
            }
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                if (softwareSerialPorts[i]->isEnabled()) {
                    if (pinNumber == softwareSerialPorts[i]->rxPin()) {
                        return true;
                    } else if (pinNumber == softwareSerialPorts[i]->txPin()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int getSerialPinIOTypeString(int pinNumber, char *out, size_t maximumSize)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if (hardwareSerialPorts[i]->isEnabled()) {
                    if (pinNumber == hardwareSerialPorts[i]->rxPin()) {
                        strncpy(out, HARDWARE_SERIAL_RX_PIN_TYPE, maximumSize);
                        return strlen(out);
                    } else if (pinNumber == hardwareSerialPorts[i]->txPin()) {
                        strncpy(out, HARDWARE_SERIAL_TX_PIN_TYPE, maximumSize);
                        return strlen(out);
                    }
                }
            }
        }
    }
    for (int i = 0; i < MAXIMUM_SOFTWARE_SERIAL_PORTS - 1; i++) {
        if (softwareSerialPorts + i) {
            if (softwareSerialPorts[i]) {
                if (softwareSerialPorts[i]->isEnabled()) {
                    if (pinNumber == softwareSerialPorts[i]->rxPin()) {
                        strncpy(out, SOFTWARE_SERIAL_RX_PIN_TYPE, maximumSize);
                        return strlen(out);
                    } else if (pinNumber == softwareSerialPorts[i]->txPin()) {
                        strncpy(out, SOFTWARE_SERIAL_TX_PIN_TYPE, maximumSize);
                        return strlen(out);
                    }
                }
            }
        }
    }
    return -1;
}

SerialPortBase *getCurrentValidOutputStream()
{
    return (currentSerialStream != nullptr ? currentSerialStream : defaultNativePort);
}

SerialPortBase *getHardwareCout(int coutIndex)
{
    if (coutIndex < 0) {
        return nullptr;
    }
    if (static_cast<unsigned int>(coutIndex) > ARRAY_SIZE(hardwareSerialPorts)) {
        return nullptr;
    } else {
        if (hardwareSerialPorts[coutIndex]->isEnabled()) {
            return hardwareSerialPorts[coutIndex];
        } else {
            return nullptr;
        }
    }
}

SerialPortBase *getSoftwareCout(int coutIndex)
{
    if (coutIndex < 0) {
        return nullptr;
    }
    if (static_cast<unsigned int>(coutIndex) > ARRAY_SIZE(softwareSerialPorts)) {
        return nullptr;
    } else {
        if (softwareSerialPorts[coutIndex]->isEnabled()) {
            return softwareSerialPorts[coutIndex];
        } else {
            return nullptr;
        }
    }
    return nullptr;
}


#if defined(__HAVE_CAN_BUS__)
    void printCanResult(const char *header, const char *str, int resultCode, bool broadcast)
    {
        if (broadcast) {
            for (auto &it : hardwareSerialPorts) {
                *it << header << ITEM_SEPARATOR << str << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
            }            
        } else {
            *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << str << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
        }
    }
    
    void printCanResult(const char *header, const CanMessage &msg, int resultCode, bool broadcast) 
    { 
        if (broadcast) {
            for (auto &it : hardwareSerialPorts) {
                *it << header << ITEM_SEPARATOR << msg.toString() << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
            }
        } else {
            *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << msg.toString() << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
        }
    }
    
    void printBlankCanResult(const char *header, int resultCode) 
    { 
        *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << resultCode << CLOSING_CHARACTER << LINE_ENDING;
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
                printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
            } else {
                if (positiveCanMasks.size() == 0) {
                    if (negativeCanMasks.find(canID) == negativeCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
                if (negativeCanMasks.size() == 0) {
                    if (positiveCanMasks.find(canID) != positiveCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
                if (positiveCanMasks.find(canID) != positiveCanMasks.end()) {
                    if (negativeCanMasks.find(canID) == negativeCanMasks.end()) {
                        CanMessage msg{canID, CAN_FRAME, receivedPacketLength, CanDataPacket{pack[0], pack[1], pack[2], pack[3], pack[4], pack[5], pack[6], pack[7]}};
                        printCanResult(CAN_READ_HEADER, msg.toString(), OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
            }
        } else if (!autoUp) {
            printBlankCanResult(CAN_READ_HEADER, OPERATION_SUCCESS);
        }
    }

    void canWriteRequest(const char *str, bool once)
    {
        using namespace ArduinoPCStrings;
        if (!canBusInitialized) {
            canInit();
        }
        CanMessage msg{parseCanMessage(str)};
        if (msg.toString() == "") {
            printSingleResult((once ? CAN_WRITE_ONCE_HEADER : CAN_WRITE_HEADER), OPERATION_FAILURE);
            return;
        }
        if (!once) {
            addLastCanMessage(msg);
        }
        sendCanMessage(msg);
        printCanResult((once ? CAN_WRITE_ONCE_HEADER : CAN_WRITE_HEADER), msg.toString(), OPERATION_SUCCESS, NO_BROADCAST);
    }

    void addPositiveCanMaskRequest(const char *str)
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

    void removePositiveCanMaskRequest(const char *str)
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

    void addNegativeCanMaskRequest(const char *str)
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

    void removeNegativeCanMaskRequest(const char *str)
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

    void canLiveUpdateRequest(const char *str)
    {
        using namespace ArduinoPCStrings;
        if (str == "") {
            printTypeResult(CAN_LIVE_UPDATE_HEADER, OPERATION_FAILURE, OPERATION_FAILURE);
            return;
        }
        int canState{parseToDigitalState(str)};
        if (canState == OPERATION_FAILURE) {
            printTypeResult(CAN_LIVE_UPDATE_HEADER, str, OPERATION_FAILURE);
        } else {
            canLiveUpdate = canState;
            printTypeResult(CAN_LIVE_UPDATE_HEADER, str, OPERATION_SUCCESS);
        }
    }

    void currentCachedCanMessageByIdRequest(const char *str)
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
            printCanResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, lastCanMessages.find(maybeID)->second, OPERATION_SUCCESS, NO_BROADCAST);
        }
    }

    void clearCurrentMessageByIdRequest(const char *str)
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
            printCanResult(CURRENT_CAN_MESSAGES_HEADER, it.second, OPERATION_SUCCESS, NO_BROADCAST);
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
        currentPositiveCanMasksRequest();
        currentNegativeCanMasksRequest();
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
    CanMessage parseCanMessage(const char *str)
    {
        using namespace ArduinoPCStrings;
        std::vector<std::string> rawMsg{FirmwareUtilities::parseToContainer<std::vector<std::string>>(str.begin(), str.end(), ITEM_SEPARATOR)};
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

    void sendCanMessage(const CanMessage &msg)
    {
        unsigned char send[8]{0, 0, 0, 0, 0, 0, 0, 0};
        msg.dataPacket().toBasicArray(send);
        canController->sendMsgBuf(msg.id(), msg.frame(), msg.length(), send);
    }

#endif
