#include "include/gpio.h"
#include "utilities.h"
#include "include/arduinopcstrings.h"
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <bytestream.h>
#include <hardwareserialport.h>
#include <softwareserialport.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#if defined(__HAVE_CAN_BUS__)
    #include "include/mcp_can.h"
    #include "include/canmessage.h"
#endif //__HAVE_CAN_BUS__

#if defined(__HAVE_LIN_BUS__)
    #include "include/lin.h"
    #include "include/linmessage.h"
#endif //__HAVE_LIN_BUS__

using namespace ArduinoPCStrings;
using namespace Utilities;

#define ID_WIDTH 3
#define MESSAGE_WIDTH 2
#define MAXIMUM_SERIAL_READ_SIZE 175
#define PIN_OFFSET 2
#define NEXT_SERIAL_PORT_UNAVAILABLE -1
#define SERIAL_BAUD 115200L
#define SERIAL_TIMEOUT 0

#define ANALOG_WRITE_PARAMETER_COUNT 2
#define DIGITAL_WRITE_PARAMETER_COUNT 2
#define SET_IO_THRESHOLD_PARAMETER_COUNT 2
#define PIN_TYPE_CHANGE_PARAMETER_COUNT 2

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

static const char *LINE_ENDING{"\n"};
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

#if defined(__HAVE_LIN_BUS__)
    void printLinResult(const char *header, const char *str, int resultCode, bool broadcast = false);
    void printLinResult(const char *header, const LinMessage &msg, int resultCode, bool broadcast = false);
    void printBlankLinResult(const char *header, int resultCode); 
#endif

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

ByteStream *getCurrentValidOutputStream();

bool isValidAnalogPinIdentifier(const char *str);
bool isValidPinIdentifier(const char *str);
bool isValidDigitalStateIdentifier(const char *str);
bool isValidAnalogStateIdentifier(const char *str);
bool isValidPwmPinIdentifier(const char *str);
bool isValidPinTypeIdentifier(const char *str);

bool checkValidIOChangeRequest(IOType ioType, int8_t pinNumber);
bool checkValidRequestString(const char *header, const char *checkStr); 

bool isValidDigitalOutputPin(int8_t pinNumber);
bool isValidDigitalInputPin(int8_t pinNumber);
bool isValidAnalogOutputPin(int8_t pinNumber);
bool isValidAnalogInputPin(int8_t pinNumber);
int8_t parseAnalogPin(const char *str);
int parseToState(const char *str);

int parseToAnalogState(const char *str);
IOType parseIOType(const char *str);
int parseToDigitalState(const char *str);
int8_t parsePin(const char *str);
void populateGpioMap();

int8_t getIOTypeString(IOType type, char *out, size_t maximumSize);
int8_t getSerialPinIOTypeString(int8_t pinNumber, char *out, size_t maximumSize);
int8_t analogPinFromNumber(int8_t number, char *out, size_t maximumSize);
GPIO *gpioPinByPinNumber(int8_t pinNumber);
bool pinInUseBySerialPort(int8_t pinNumber);
size_t makeRequestString(const char *str, const char *header, char *out, size_t maximumSize);
uint8_t analogPinArraySize();
uint8_t generalPinArraySize();
uint8_t pwmPinArraySize();

#if defined(ARDUINO_AVR_UNO)
    static const PROGMEM int8_t AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, -1};
    static const PROGMEM int8_t AVAILABLE_GENERAL_PINS[]{2, 4, 7, 8, 12, 13, -1};
    #define NUMBER_OF_ANALOG_PINS 6
    #define ANALOG_PIN_OFFSET 13
    #define NUMBER_OF_PINS 21
    static const PROGMEM int8_t AVAILABLE_PWM_PINS[]{3, 5, 6, 9, 10, 11, -1};
#elif defined(ARDUINO_AVR_NANO)
    static const PROGMEM int8_t AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, A6, A7, -1};                                                
    static const PROGMEM int8_t AVAILABLE_GENERAL_PINS[]{2, 4, 7, 8, 12, 13, -1};
    #define NUMBER_OF_ANALOG_PINS 8
    #define ANALOG_PIN_OFFSET 13
    #define NUMBER_OF_PINS 23
    static const PROGMEM int8_t AVAILABLE_PWM_PINS[]{3, 5, 6, 9, 10, 11, -1};
#elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    static const PROGMEM int8_t AVAILABLE_ANALOG_PINS[]{A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, -1};
    static const PROGMEM int8_t AVAILABLE_GENERAL_PINS[]{14, 15, 16, 17, 18, 19, 20,21, 22, 23, 24, 
                                                      25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
                                                      38, 39, 40, 41, 42, 43, 47, 48, 49, 50, 51, 52, 53,-1};            

    static const PROGMEM int8_t AVAILABLE_SERIAL_RX_PINS[]{10, 11, 12, 13, 14, 15, 50, 51, 52, 53,
                                                           A8, A9, A10, A11, A12, A13, A14, A15};                                    
    #define NUMBER_OF_ANALOG_PINS 16
    #define ANALOG_PIN_OFFSET 53
    #define NUMBER_OF_PINS 71
    static const PROGMEM int8_t AVAILABLE_PWM_PINS[]{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, 46, -1};
#endif

#if defined(__HAVE_CAN_BUS__)
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

    #define EMPTY_CAN_MASK_SLOT 0x0
    #define MAX_LAST_CAN_MESSAGES 10
    #define MAX_POSITIVE_CAN_MASKS 10
    #define MAX_NEGATIVE_CAN_MASKS 10
    static CanMessage lastCanMessages[MAX_LAST_CAN_MESSAGES];
    static uint32_t positiveCanMasks[MAX_POSITIVE_CAN_MASKS];
    static uint32_t negativeCanMasks[MAX_NEGATIVE_CAN_MASKS];
    bool positiveCanMaskExists(uint32_t targetMask);
    bool negativeCanMaskExists(uint32_t targetMask);
    uint8_t numberOfPositiveCanMasks();
    uint8_t numberOfNegativeCanMasks();
    void initializeCanMasks();
    
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

    static ByteStream *hardwareSerialPorts[NUMBER_OF_HARDWARE_SERIAL_PORTS] {
        new HardwareSerialPort{&Serial,   0,   1,  SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial1,  19,  18, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial2,  17,  16, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING},
        new HardwareSerialPort{&Serial3,  15,  14, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING}
    };
    
    static ByteStream *softwareSerialPorts[MAXIMUM_SOFTWARE_SERIAL_PORTS] {
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

#else
    #define SOFTWARE_SERIAL_ENUM_OFFSET 1
    #define NUMBER_OF_HARDWARE_SERIAL_PORTS 1
    #define MAXIMUM_SOFTWARE_SERIAL_PORTS 1

    static ByteStream *hardwareSerialPorts[NUMBER_OF_HARDWARE_SERIAL_PORTS] {
        new HardwareSerialPort{&Serial, 0,  1, SERIAL_BAUD, SERIAL_TIMEOUT, true, LINE_ENDING}
    };
    
    static ByteStream *softwareSerialPorts[MAXIMUM_SOFTWARE_SERIAL_PORTS] {
        nullptr
    };

#endif
static uint8_t softwareSerialPortIndex{0};
static GPIO *gpioPins[NUMBER_OF_PINS];

void initializeSerialPorts();
void announceStartup();
void doImAliveBlink();
ByteStream *getHardwareCout(int coutIndex);
ByteStream *getSoftwareCout(int coutIndex);
static ByteStream *currentSerialStream{hardwareSerialPorts[0]};
ByteStream *defaultNativePort{hardwareSerialPorts[0]};

template <typename Header, typename PinNumber, typename State, typename ResultCode> inline void printResult(const Header &header, PinNumber pinNumber, State state, ResultCode resultCode)
{    
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << pinNumber << ITEM_SEPARATOR << state << ITEM_SEPARATOR << resultCode << LINE_ENDING;
}

template <typename Header, typename ResultCode> inline void printSingleResult(const Header &header, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << resultCode << LINE_ENDING;
}

template <typename Header, typename Type, typename ResultCode> inline void printTypeResult(const Header &header, Type type, ResultCode resultCode)
{
    *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << type << ITEM_SEPARATOR << resultCode << LINE_ENDING; 
}

template <typename Parameter> inline void printString(const Parameter &parameter)
{
    *getCurrentValidOutputStream() << parameter << LINE_ENDING;
}

void setup() {
    initializeSerialPorts();
    announceStartup();
    populateGpioMap();
    #if defined(__HAVE_CAN_BUS__)
        initializeCanMasks();
    #endif //__HAVE_CAN_BUS__
}

void loop() {
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        ByteStream *it{nullptr};
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
                currentSerialStream = it;
                handleSerialString(buffer);
            }
        }
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(softwareSerialPorts); i++) {
        ByteStream *it{nullptr};
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
                currentSerialStream = it;
                handleSerialString(buffer);
            }
        }
    }
    
    #if defined(__HAVE_CAN_BUS__)
        if (canLiveUpdate) {
            canReadRequest(canLiveUpdate);
        }
        bool canSend{false};
        for (int i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            if (lastCanMessages[i].id() != 0) {
                canSend = true;
            }
        }
        if (canSend) {
            if (canCommunicationStartTime == 0) {
                canCommunicationStartTime = millis();
            }
            canCommunicationEndTime = millis();
            if (static_cast<unsigned int>((canCommunicationEndTime - canCommunicationStartTime)) >= CAN_COMMUNICATION_DOWN_TIME) {    
                for (int i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
                    if (lastCanMessages[i].id() != 0) {
                        sendCanMessage(lastCanMessages[i]);
                    }
                }
                canCommunicationStartTime = millis();
            }
        }
    #endif
    doImAliveBlink();
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

void handleSerialString(const char *str)
{
    if (!str) {
        return;
    } else if (strlen(str) == 0) {
        return;
    }
    char requestString[SMALL_BUFFER_SIZE];
    int substringResult{0};
    (void)substringResult;
    if (startsWith(str, ANALOG_READ_HEADER)) {
        if (checkValidRequestString(ANALOG_READ_HEADER, str)) {
            substringResult = makeRequestString(str, ANALOG_READ_HEADER, requestString, SMALL_BUFFER_SIZE);
            analogReadRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CHANGE_A_TO_D_THRESHOLD_HEADER)) {
        if (checkValidRequestString(CHANGE_A_TO_D_THRESHOLD_HEADER, str)) {
            substringResult = makeRequestString(str, CHANGE_A_TO_D_THRESHOLD_HEADER, requestString, SMALL_BUFFER_SIZE);
            changeAToDThresholdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ANALOG_WRITE_HEADER)) {
        if (checkValidRequestString(ANALOG_WRITE_HEADER, str)) {
            substringResult = makeRequestString(str, ANALOG_WRITE_HEADER, requestString, SMALL_BUFFER_SIZE);
            analogWriteRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_READ_HEADER)) {
        if (checkValidRequestString(DIGITAL_READ_HEADER, str)) {
            substringResult = makeRequestString(str, DIGITAL_READ_HEADER, requestString, SMALL_BUFFER_SIZE);
            digitalReadRequest(requestString, false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_ALL_HEADER)) {
        if (checkValidRequestString(DIGITAL_WRITE_ALL_HEADER, str)) {
            substringResult = makeRequestString(str, DIGITAL_WRITE_ALL_HEADER, requestString, SMALL_BUFFER_SIZE);
            digitalWriteAllRequest(requestString);
        } else {
            printTypeResult(DIGITAL_WRITE_ALL_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, DIGITAL_WRITE_HEADER)) {
        if (checkValidRequestString(DIGITAL_WRITE_HEADER, str)) {
            substringResult = makeRequestString(str, DIGITAL_WRITE_HEADER, requestString, SMALL_BUFFER_SIZE);
            digitalWriteRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_HEADER)) {
        if (checkValidRequestString(PIN_TYPE_HEADER, str)) {
            substringResult = substring(str, strlen(PIN_TYPE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            pinTypeRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, PIN_TYPE_CHANGE_HEADER)) {
        if (checkValidRequestString(PIN_TYPE_CHANGE_HEADER, str)) {
            substringResult = substring(str, strlen(PIN_TYPE_CHANGE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            pinTypeChangeRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, SOFT_DIGITAL_READ_HEADER)) {
        if (checkValidRequestString(SOFT_DIGITAL_READ_HEADER, str)) {
            substringResult = makeRequestString(str, SOFT_DIGITAL_READ_HEADER, requestString, SMALL_BUFFER_SIZE);
            digitalReadRequest(requestString, true);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, SOFT_ANALOG_READ_HEADER)) {
        if (checkValidRequestString(SOFT_ANALOG_READ_HEADER, str)) {
            substringResult = makeRequestString(str, SOFT_ANALOG_READ_HEADER, requestString, SMALL_BUFFER_SIZE);
            softAnalogReadRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, HEARTBEAT_HEADER)) {
        heartbeatRequest();
    } else if (startsWith(str, FIRMWARE_VERSION_HEADER)) {
        firmwareVersionRequest();
    } else if (startsWith(str, IO_REPORT_HEADER)) {
        ioReportRequest();
    } else if (startsWith(str, CURRENT_A_TO_D_THRESHOLD_HEADER)) {
        currentAToDThresholdRequest();
    } else if (startsWith(str, ARDUINO_TYPE_HEADER)) {
        arduinoTypeRequest();
    } else if (startsWith(str, CAN_BUS_ENABLED_HEADER)) {
        canBusEnabledRequest();
#if (defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA1280))
    } else if (startsWith(str, ADD_HARDWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(ADD_HARDWARE_SERIAL_HEADER, str)) {
            substringResult = substring(str, strlen(ADD_HARDWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addHardwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }  
    } else if (startsWith(str, REMOVE_HARDWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(REMOVE_HARDWARE_SERIAL_HEADER, str)) {
            substringResult = substring(str, strlen(REMOVE_HARDWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeHardwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
#endif
#if defined(__HAVE_CAN_BUS__)
    } else if (startsWith(str, CAN_INIT_HEADER)) {
        canInitRequest();
    } else if (startsWith(str, CAN_READ_HEADER)) {
        canReadRequest(false);
    } else if (startsWith(str, CAN_WRITE_ONCE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_ONCE_HEADER, str)) {
            substringResult = substring(str, strlen(CAN_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canWriteRequest(requestString, true);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CAN_WRITE_HEADER)) {
        if (checkValidRequestString(CAN_WRITE_HEADER, str)) {
            substringResult = substring(str, strlen(CAN_WRITE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canWriteRequest(requestString, false);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }     
    } else if (startsWith(str, CAN_LIVE_UPDATE_HEADER)) {
        if (checkValidRequestString(CAN_LIVE_UPDATE_HEADER, str)) {
            substringResult = substring(str, strlen(CAN_LIVE_UPDATE_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            canLiveUpdateRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, CLEAR_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str)) {
            substringResult = substring(str, strlen(CLEAR_CAN_MESSAGE_BY_ID_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            clearCurrentMessageByIdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }   
    } else if (startsWith(str, CURRENT_CAN_MESSAGE_BY_ID_HEADER)) {
        if (checkValidRequestString(CURRENT_CAN_MESSAGE_BY_ID_HEADER, str)) {
            substringResult = substring(str, strlen(CURRENT_CAN_MESSAGE_BY_ID_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            currentCachedCanMessageByIdRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_NEGATIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_NEGATIVE_CAN_MASK_HEADER, str)) {
            substringResult = substring(str, strlen(REMOVE_NEGATIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeNegativeCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_POSITIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(REMOVE_POSITIVE_CAN_MASK_HEADER, str)) {
            substringResult = substring(str, strlen(REMOVE_POSITIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removePositiveCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_POSITIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(ADD_POSITIVE_CAN_MASK_HEADER, str)) {
            substringResult = substring(str, strlen(ADD_POSITIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addPositiveCanMaskRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, ADD_NEGATIVE_CAN_MASK_HEADER)) {
        if (checkValidRequestString(ADD_NEGATIVE_CAN_MASK_HEADER, str)) {
            substringResult = substring(str, strlen(ADD_NEGATIVE_CAN_MASK_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
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
    } else if (startsWith(str, ADD_SOFTWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(ADD_SOFTWARE_SERIAL_HEADER, str)) {
            substringResult = substring(str, strlen(ADD_SOFTWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            addSoftwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else if (startsWith(str, REMOVE_SOFTWARE_SERIAL_HEADER)) {
        if (checkValidRequestString(REMOVE_SOFTWARE_SERIAL_HEADER, str)) {
            substringResult = substring(str, strlen(REMOVE_SOFTWARE_SERIAL_HEADER)+1, requestString, SMALL_BUFFER_SIZE);
            removeSoftwareSerialRequest(requestString);
        } else {
            printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
        }
    } else {
        printTypeResult(INVALID_HEADER, str, OPERATION_FAILURE);
    }

}

size_t makeRequestString(const char *str, const char *header, char *out, size_t maximumSize)
{
    size_t returnLength = substring(str, strlen(header) + 1, out, maximumSize);
    if ((out[strlen(out)-1] == '\n') || (out[strlen(out)-1] == '\r')) {
        out[strlen(out)-1] = '\0';
        returnLength--;
    }
    return returnLength;
}

void addSoftwareSerialRequest(const char *str)
{
    int foundPosition{positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int8_t rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }

    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);
    
    int8_t txPinNumber{parsePin(maybeTxPin)};
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
                    printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
                }
            }
        }
    }
    if (isValidSoftwareSerialAddition(rxPinNumber, txPinNumber)) {
        softwareSerialPorts[softwareSerialPortIndex++] = new SoftwareSerialPort{new SoftwareSerial{static_cast<uint8_t>(rxPinNumber), static_cast<uint8_t>(txPinNumber)},
                                                                                static_cast<uint8_t>(rxPinNumber), 
                                                                                static_cast<uint8_t>(txPinNumber), 
                                                                                SERIAL_BAUD, 
                                                                                SERIAL_TIMEOUT, 
                                                                                true, 
                                                                                LINE_ENDING};
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_SUCCESS);
    } else {
        printResult(ADD_SOFTWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
    }
}

void addHardwareSerialRequest(const char *str)
{
    int foundPosition{positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
    
    int8_t rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int8_t txPinNumber{parsePin(maybeTxPin)};
    if (txPinNumber == INVALID_PIN) {
        printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
        return;
    }
    for (unsigned int i = 0; i < ARRAY_SIZE(hardwareSerialPorts); i++) {
        if (hardwareSerialPorts + i) {
            if (hardwareSerialPorts[i]) {
                if ((rxPinNumber == hardwareSerialPorts[i]->rxPin()) && (txPinNumber == hardwareSerialPorts[i]->txPin())) {
                    hardwareSerialPorts[i]->setEnabled(true);
                    printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_KIND_OF_SUCCESS);
                }
            }
        }
    }
    printResult(ADD_HARDWARE_SERIAL_HEADER, maybeRxPin, maybeTxPin, OPERATION_FAILURE);
}

void removeSoftwareSerialRequest(const char *str)
{
    int foundPosition{positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int8_t rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_SOFTWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int8_t txPinNumber{parsePin(maybeTxPin)};
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
    int foundPosition{positionOfSubstring(str, ITEM_SEPARATOR)};
    char maybeRxPin[SMALL_BUFFER_SIZE];
    int result{substring(str, 0, foundPosition, maybeRxPin, SMALL_BUFFER_SIZE)};
    (void)result;
 
    int8_t rxPinNumber{parsePin(maybeRxPin)};
    if (rxPinNumber == INVALID_PIN) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    if (pinInUseBySerialPort(rxPinNumber)) {
        printResult(REMOVE_HARDWARE_SERIAL_HEADER, maybeRxPin, STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        return;
    }
    char maybeTxPin[SMALL_BUFFER_SIZE];
    result = substring(str, foundPosition+1, maybeTxPin, SMALL_BUFFER_SIZE);

    int8_t txPinNumber{parsePin(maybeTxPin)};
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
            int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_SERIAL_RX_PINS + i++)};
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
    *getCurrentValidOutputStream() << IO_REPORT_HEADER << ITEM_SEPARATOR;
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        GPIO *gpioPin{gpioPinByPinNumber(i)};
        if (!gpioPin) {
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
            int8_t result{analogPinFromNumber(gpioPin->pinNumber(), analogPinString, SMALL_BUFFER_SIZE)};
            int8_t secondResult{getIOTypeString(gpioPin->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
            (void)result;
            (void)secondResult;
            *getCurrentValidOutputStream() << ITEM_SEPARATOR << analogPinString << ITEM_SEPARATOR << ioTypeString << ITEM_SEPARATOR << state;
        } else {
            char ioTypeString[SMALL_BUFFER_SIZE];
            int result{getIOTypeString(gpioPin->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
            (void)result;
            *getCurrentValidOutputStream() << ITEM_SEPARATOR << gpioPin->pinNumber() << ITEM_SEPARATOR << ioTypeString << ITEM_SEPARATOR << state;
        }
    }
    *getCurrentValidOutputStream() << ITEM_SEPARATOR << IO_REPORT_END_HEADER << LINE_ENDING;
}

void digitalReadRequest(const char *str, bool soft)
{
    int8_t pinNumber{parsePin(str)};
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
    bool state{false};
    GPIO *gpioHandle{gpioPinByPinNumber(pinNumber)};
    if (gpioHandle->ioType() == IOType::DIGITAL_OUTPUT) {
        state = gpioHandle->g_softDigitalRead();
    } else {
        state = gpioHandle->g_digitalRead();
    }
    printResult((soft ? SOFT_DIGITAL_READ_HEADER : DIGITAL_READ_HEADER), str, state, OPERATION_SUCCESS);
}

void digitalWriteRequest(const char *str)
{
    char **splitString{calloc2D<char>(DIGITAL_WRITE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)};
    int splitStringSize{split(str, splitString, ITEM_SEPARATOR, DIGITAL_WRITE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)};
    if (splitStringSize != DIGITAL_WRITE_PARAMETER_COUNT) {
        printResult(DIGITAL_WRITE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, DIGITAL_WRITE_PARAMETER_COUNT);
        return;
    }
    int8_t pinNumber{parsePin(splitString[0])};
    if (pinNumber == INVALID_PIN) {
        printResult(DIGITAL_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, DIGITAL_WRITE_PARAMETER_COUNT);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        free2D(splitString, DIGITAL_WRITE_PARAMETER_COUNT);
        return;
    }
    if (!isValidDigitalOutputPin(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, DIGITAL_WRITE_PARAMETER_COUNT);
        return;
    }
    
    int state{parseToDigitalState(splitString[1])};
    if (state == OPERATION_FAILURE) {
        printResult(DIGITAL_WRITE_HEADER, splitString[0], splitString[1], OPERATION_FAILURE);
    } else {
        gpioPinByPinNumber(pinNumber)->g_digitalWrite(state);
        printResult(DIGITAL_WRITE_HEADER, splitString[0], state, OPERATION_SUCCESS);
    }
    free2D(splitString, DIGITAL_WRITE_PARAMETER_COUNT);
}

void digitalWriteAllRequest(const char *str)
{
    *getCurrentValidOutputStream() << DIGITAL_WRITE_ALL_HEADER << ITEM_SEPARATOR;
    
    int state{parseToDigitalState(str)};
    if (state == OPERATION_FAILURE) {
        printTypeResult(DIGITAL_WRITE_ALL_HEADER, str, OPERATION_FAILURE);
        return;
    }
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        GPIO *gpioPin{gpioPinByPinNumber(i)};
        if (gpioPin) {
            if (gpioPin->ioType() == IOType::DIGITAL_OUTPUT) {
                gpioPin->g_digitalWrite(state);
                if (isValidAnalogInputPin(gpioPin->pinNumber())) {
                    char analogPinString[SMALL_BUFFER_SIZE];
                    int8_t result{analogPinFromNumber(gpioPin->pinNumber(), analogPinString, SMALL_BUFFER_SIZE)};
                    (void)result;
                    *getCurrentValidOutputStream() << ITEM_SEPARATOR << analogPinString;
                } else {
                    *getCurrentValidOutputStream() << ITEM_SEPARATOR << gpioPin->pinNumber();
                }
            }
        }
    }
    *getCurrentValidOutputStream() << ITEM_SEPARATOR << state << ITEM_SEPARATOR << OPERATION_SUCCESS << LINE_ENDING;
}

void analogReadRequest(const char *str)
{
    int8_t pinNumber{parsePin(str)};
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
    char **splitString{calloc2D<char>(ANALOG_WRITE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)};
    int splitStringSize{split(str, splitString, ITEM_SEPARATOR, ANALOG_WRITE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)};
    if (splitStringSize != ANALOG_WRITE_PARAMETER_COUNT) {
        printResult(ANALOG_WRITE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, ANALOG_WRITE_PARAMETER_COUNT);
        return;
    }
    int8_t pinNumber{parsePin(splitString[0])};
    if (pinNumber == INVALID_PIN) {
        printResult(ANALOG_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, ANALOG_WRITE_PARAMETER_COUNT);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(ANALOG_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        free2D(splitString, ANALOG_WRITE_PARAMETER_COUNT);
        return;
    }
    if (!isValidAnalogOutputPin(pinNumber)) {
        printResult(ANALOG_WRITE_HEADER, splitString[0], STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, ANALOG_WRITE_PARAMETER_COUNT);
        return;
    }
    
    int state{parseToAnalogState(splitString[1])};
    if (state == OPERATION_FAILURE) {
        printResult(ANALOG_WRITE_HEADER, splitString[0], splitString[1], OPERATION_FAILURE);
    } else {
        gpioPinByPinNumber(pinNumber)->g_analogWrite(state);
        printResult(ANALOG_WRITE_HEADER, splitString[0], state, OPERATION_SUCCESS);
    }
    free2D(splitString, ANALOG_WRITE_PARAMETER_COUNT);
}

void pinTypeRequest(const char *str)
{
    int8_t pinNumber{parsePin(str)};
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
    GPIO *tempGpio{gpioPinByPinNumber(pinNumber)};
    if (!tempGpio) {
        printResult(PIN_TYPE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        return;
    }
    char ioTypeString[SMALL_BUFFER_SIZE];
    int8_t result{getIOTypeString(tempGpio->ioType(), ioTypeString, SMALL_BUFFER_SIZE)};
    (void)result;
    printResult(PIN_TYPE_HEADER, str, ioTypeString, OPERATION_SUCCESS);
}

void pinTypeChangeRequest(const char *str)
{   
    char **splitString{calloc2D<char>(PIN_TYPE_CHANGE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)}; 
    int splitStringSize{split(str, splitString, ITEM_SEPARATOR, PIN_TYPE_CHANGE_PARAMETER_COUNT, SMALL_BUFFER_SIZE)};
    if (splitStringSize != PIN_TYPE_CHANGE_PARAMETER_COUNT) {
        printResult(PIN_TYPE_CHANGE_HEADER, INVALID_PIN, STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }
    int8_t pinNumber{parsePin(splitString[0])};
    if (pinNumber == INVALID_PIN) {
        printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], STATE_FAILURE, OPERATION_FAILURE);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], STATE_FAILURE, OPERATION_PIN_USED_BY_SERIAL_PORT);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }

    IOType type{parseIOType(splitString[1])};
    if (type == IOType::UNSPECIFIED) {
        printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], splitString[1], OPERATION_FAILURE);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }

    char ioTypeString[SMALL_BUFFER_SIZE];
    (void)getIOTypeString(type, ioTypeString, SMALL_BUFFER_SIZE);
    if (pinInUseBySerialPort(pinNumber)) {
        printResult(DIGITAL_WRITE_HEADER, splitString[0], ioTypeString, OPERATION_PIN_USED_BY_SERIAL_PORT);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }
    if (!checkValidIOChangeRequest(type, pinNumber)) {
        printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], ioTypeString, OPERATION_FAILURE);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }
    GPIO *tempGpio{gpioPinByPinNumber(pinNumber)};
    if (!tempGpio) {
        printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], ioTypeString, OPERATION_FAILURE);
        free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
        return;
    }
    tempGpio->setIOType(type);
    printResult(PIN_TYPE_CHANGE_HEADER, splitString[0], ioTypeString, OPERATION_SUCCESS);
    free2D(splitString, PIN_TYPE_CHANGE_PARAMETER_COUNT);
}

void softAnalogReadRequest(const char *str)
{
    int8_t pinNumber{parsePin(str)};
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
    char *stringToPrint = (char *)calloc(strlen(HEARTBEAT_HEADER) + 1, sizeof(char));
    strncpy(stringToPrint, HEARTBEAT_HEADER, strlen(HEARTBEAT_HEADER) + 1);

    stringToPrint[strlen(stringToPrint)] = '\0';

    printString(stringToPrint);
    free(stringToPrint);
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
    for (uint8_t i = 0; i < NUMBER_OF_PINS; i++) {
        gpioPins[i] = nullptr;
    }
    uint8_t i{0};
    do {
        int8_t pinNumber{pgm_read_byte_near(AVAILABLE_PWM_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins[pinNumber] = new GPIO(pinNumber, IOType::DIGITAL_INPUT_PULLUP);
    } while (true);
    i = 0;
    do {
        int8_t pinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + i++)};
        if (pinNumber < 0) {
            break;
        }
        gpioPins[pinNumber] = new GPIO(pinNumber, IOType::ANALOG_INPUT);
    } while (true);
    i = 0;
    do {
        int8_t pinNumber{pgm_read_byte_near(AVAILABLE_GENERAL_PINS + i++)};
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


bool isValidDigitalOutputPin(int8_t pinNumber)
{
#if defined(ARDUINO_AVR_NANO)
    if ((pinNumber == A6) || (pinNumber == A7)) {
        return false;
    }
#endif
    char temp[SMALL_BUFFER_SIZE];
    snprintf(temp, SMALL_BUFFER_SIZE, "%i", pinNumber);
    return isValidPinIdentifier(temp);
}

bool isValidDigitalInputPin(int8_t pinNumber)
{
    return isValidDigitalOutputPin(pinNumber);
}

bool isValidAnalogOutputPin(int8_t pinNumber)
{
    uint8_t i{0};
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_PWM_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == pinNumber) {
            return true;
        }
    } while (true);
}

bool isValidAnalogInputPin(int8_t pinNumber)
{
    uint8_t i{0};
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            return false;
        }
        if (tempPinNumber == pinNumber) {
            return true;
        }
    } while (true);
}

bool checkValidIOChangeRequest(IOType ioType, int8_t pinNumber)
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

int8_t getIOTypeString(IOType ioType, char *out, size_t maximumSize)
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

int8_t parsePin(const char *str)
{
    if (startsWith(str, ANALOG_IDENTIFIER_CHAR)) { 
        return parseAnalogPin(str);
    } else if (isValidPinIdentifier(str)) {
        return atoi(str);
    } else {
        return INVALID_PIN;
    }
}

IOType parseIOType(const char *str)
{
    if (strncmp(str, DIGITAL_INPUT_IDENTIFIER, SMALL_BUFFER_SIZE) == 0) {
        return IOType::DIGITAL_INPUT;
    } else if (strncmp(str, DIGITAL_OUTPUT_IDENTIFIER, SMALL_BUFFER_SIZE) == 0) {
        return IOType::DIGITAL_OUTPUT;
    } else if (strncmp(str, ANALOG_INPUT_IDENTIFIER, SMALL_BUFFER_SIZE) == 0) {
        return IOType::ANALOG_INPUT;
    } else if (strncmp(str, ANALOG_OUTPUT_IDENTIFIER, SMALL_BUFFER_SIZE) == 0) {
        return IOType::ANALOG_OUTPUT;
    } else if (strncmp(str, DIGITAL_INPUT_PULLUP_IDENTIFIER, SMALL_BUFFER_SIZE) == 0) {
        return IOType::DIGITAL_INPUT_PULLUP;
    } else {
        return IOType::UNSPECIFIED;
    }
}

bool isValidAnalogStateIdentifier(const char *str)
{
    if (!str) {
        return false;
    } else if (strlen(str) == 1) {
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
    } else if (strlen(str) == 1) {
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
    for (int i = 0; i < NUMBER_OF_PINS; i++) {
        GPIO *tempGpio{gpioPinByPinNumber(i)};
        if (tempGpio) {
            if (atoi(str) == static_cast<int>(tempGpio->pinNumber())) {
                return true;
            }
        }
    }

    char *buffer = (char *)calloc(strlen(str) + 1, sizeof(char));
    strncpy(buffer, str, strlen(str) + 1);

    for (unsigned int i =  0; i < analogPinArraySize(); i++) {
        if (startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(i)) {
                free(buffer);
                return ANALOG_PIN_OFFSET + i + 1;
            }
        }
    }
    free(buffer);
    return false;
}

bool isValidPwmPinIdentifier(const char *str)
{
    uint8_t i{0};
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_PWM_PINS + i++)};
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
    uint8_t i{0};
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + i++)};
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

    for (unsigned int j = 0; j < analogPinArraySize(); j++) {
        if (startsWith(str, ANALOG_IDENTIFIER_CHAR)) {
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
    strncpy (str, INITIALIZATION_HEADER, SMALL_BUFFER_SIZE);

    size_t stringLength{strlen(str)};
    str[stringLength] = ITEM_SEPARATOR;
    str[stringLength + 1] = '\0';

    strncat (str, ARDUINO_TYPE, SMALL_BUFFER_SIZE);

    stringLength = strlen(str);
    str[stringLength] = ITEM_SEPARATOR;
    str[stringLength + 1] = '\0';

    strncat (str, FIRMWARE_VERSION, SMALL_BUFFER_SIZE);

    stringLength = strlen(str);
    str[strlen(str)] = '\0';

    broadcastString(str);
}


GPIO *gpioPinByPinNumber(int8_t pinNumber)
{
    return ((gpioPins + pinNumber) ? gpioPins[pinNumber] : nullptr);
}

uint8_t analogPinArraySize()
{
    uint8_t arraySize{0};
    while (true) {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + arraySize++)};
        if (tempPinNumber < 0) {
            arraySize--;
            break;
        }
    }
    return arraySize;
}

uint8_t generalPinArraySize()
{
    uint8_t arraySize{0};
    while (true) {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_GENERAL_PINS + arraySize++)};
        if (tempPinNumber < 0) {
            arraySize--;
            break;
        }
    }
    return arraySize;
}

uint8_t pwmPinArraySize()
{
    size_t arraySize{0};
    while (true) {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_PWM_PINS + arraySize++)};
        if (tempPinNumber < 0) {
            arraySize--;
            break;
        } 
    }
    return arraySize;
}

int8_t parseAnalogPin(const char *pinAlias)
{
    if (!pinAlias) {
        return 0;
    }
    char *buffer = (char *)calloc(strlen(pinAlias) + 1, sizeof(char));
    strncpy(buffer, pinAlias, strlen(pinAlias) + 1);
    for (unsigned int i =  0; i < analogPinArraySize(); i++) {
        if (startsWith(pinAlias, ANALOG_IDENTIFIER_CHAR)) {
            buffer[0] = DIGITAL_STATE_LOW_IDENTIFIER;
            if (atoi(buffer) == static_cast<int>(i)) {
                free(buffer);
                return ANALOG_PIN_OFFSET + i + 1;
            }
        }
    }
    free(buffer);
    uint8_t i{0};
    int8_t maybePinNumber{parsePin(pinAlias)};
    if (maybePinNumber == INVALID_PIN) {
        return INVALID_PIN;
    }
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            break;
        }
        if (tempPinNumber == maybePinNumber) {
            return tempPinNumber;
        }
    } while (true);
    return INVALID_PIN;
}

int8_t analogPinFromNumber(int8_t pinNumber, char *out, size_t maximumSize)
{
    if (!out) {
        return 0;
    }
    uint8_t i{0};
    do {
        int8_t tempPinNumber{pgm_read_byte_near(AVAILABLE_ANALOG_PINS + i++)};
        if (tempPinNumber < 0) {
            return 0;
        }
        if (tempPinNumber == pinNumber) {
            char tempNumber[SMALL_BUFFER_SIZE];
            if (!toDecString(i-1, tempNumber, maximumSize)) {
                return 0;
            }
            out[0] = ANALOG_IDENTIFIER_CHAR;
            out[1] = '\0';
            strcat(out, tempNumber);
            return strlen(out);
        }
    } while (true);
}

bool pinInUseBySerialPort(int8_t pinNumber)
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

int8_t getSerialPinIOTypeString(int8_t pinNumber, char *out, size_t maximumSize)
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

ByteStream *getCurrentValidOutputStream()
{
    return (currentSerialStream != nullptr ? currentSerialStream : defaultNativePort);
}

ByteStream *getHardwareCout(int coutIndex)
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

ByteStream *getSoftwareCout(int coutIndex)
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
                *it << header << ITEM_SEPARATOR << str << ITEM_SEPARATOR << resultCode << LINE_ENDING;
            }            
        } else {
            *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << str << ITEM_SEPARATOR << resultCode << LINE_ENDING;
        }
    }
    
    void printCanResult(const char *header, const CanMessage &msg, int resultCode, bool broadcast) 
    { 
        char temp[SMALL_BUFFER_SIZE];
        msg.toString(temp, SMALL_BUFFER_SIZE);
        if (broadcast) {
            for (int i = 0; i < NUMBER_OF_HARDWARE_SERIAL_PORTS; i++) {
                if (hardwareSerialPorts + i) {
                    if (hardwareSerialPorts[i]) {
                        *(hardwareSerialPorts[i]) << header << ITEM_SEPARATOR << temp << ITEM_SEPARATOR << resultCode << LINE_ENDING;
                    }
                }
            }
        } else {
            *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << temp << ITEM_SEPARATOR << resultCode << LINE_ENDING;
        }
    }
    
    void printBlankCanResult(const char *header, int resultCode) 
    { 
        *getCurrentValidOutputStream() << header << ITEM_SEPARATOR << resultCode << LINE_ENDING;
    } 

    void canInit()
    {
        using namespace ArduinoPCStrings;
        unsigned int startTime = millis();
        unsigned int endTime = millis();
        if (canBusInitialized) {
            return;
        }
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
        canInit();
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
        //TODO: Extended frame
        unsigned char receivedPacketLength{0};
        unsigned char pack[8]{0, 0, 0, 0, 0, 0, 0, 0};
        uint32_t canID{0}; 
        if (CAN_MSGAVAIL == canController->checkReceive()) {
            canController->readMsgBuf(&receivedPacketLength, pack);
            canID = canController->getCanId();
            if ((numberOfPositiveCanMasks() == 0) && (numberOfNegativeCanMasks() == 0)) {
                CanMessage readMessage{canID, CAN_FRAME, receivedPacketLength, pack};
                printCanResult(CAN_READ_HEADER, readMessage, OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
            } else {
                if (numberOfPositiveCanMasks() == 0) {
                    if (!negativeCanMaskExists(canID)) {
                        CanMessage readMessage{canID, CAN_FRAME, receivedPacketLength, pack};
                        printCanResult(CAN_READ_HEADER, readMessage, OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
                if (numberOfNegativeCanMasks() == 0) {
                    if (positiveCanMaskExists(canID)) {
                        CanMessage readMessage{canID, CAN_FRAME, receivedPacketLength, pack};
                        printCanResult(CAN_READ_HEADER, readMessage, OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
                if (positiveCanMaskExists(canID)) {
                    if (!negativeCanMaskExists(canID)) {
                        CanMessage readMessage{canID, CAN_FRAME, receivedPacketLength, pack};
                        printCanResult(CAN_READ_HEADER, readMessage, OPERATION_SUCCESS, (autoUp ? BROADCAST : NO_BROADCAST));
                    }
                }
            }
        } else if (!autoUp) {
            printBlankCanResult(CAN_READ_HEADER, OPERATION_SUCCESS);
        }
    }

    void canWriteRequest(const char *str, bool once)
    {
        if (!canBusInitialized) {
            canInit();
        }
        CanMessage readMessage{CanMessage::parse(str, ITEM_SEPARATOR, SMALL_BUFFER_SIZE)};
        char tempMessage[SMALL_BUFFER_SIZE];
        int resultLength{readMessage.toString(tempMessage, SMALL_BUFFER_SIZE)};
        if (!resultLength) {
            printSingleResult((once ? CAN_WRITE_ONCE_HEADER : CAN_WRITE_HEADER), OPERATION_FAILURE);
            return;
        }
        if (!once) {
            addLastCanMessage(readMessage);
        }
        sendCanMessage(readMessage);
        printCanResult((once ? CAN_WRITE_ONCE_HEADER : CAN_WRITE_HEADER), tempMessage, OPERATION_SUCCESS, NO_BROADCAST);
    }

    void addPositiveCanMaskRequest(const char *str)
    {
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        int8_t result{addPositiveCanMask(maybeID)};
        if (result == -1) {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        } else if (result) {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
        } else {
            printTypeResult(ADD_POSITIVE_CAN_MASK_HEADER, str, OPERATION_KIND_OF_SUCCESS);
        }
    }

    void removePositiveCanMaskRequest(const char *str)
    {
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(REMOVE_POSITIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            if (positiveCanMasks[i] == maybeID) {
                positiveCanMasks[i] = EMPTY_CAN_MASK_SLOT;
            }
        }
        printTypeResult(REMOVE_POSITIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
    }

    void addNegativeCanMaskRequest(const char *str)
    {
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        int8_t result{addNegativeCanMask(maybeID)};
        if (result == -1) {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        } else if (result) {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
        } else {
            printTypeResult(ADD_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_KIND_OF_SUCCESS);
        }
    }

    void removeNegativeCanMaskRequest(const char *str)
    {
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(REMOVE_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_FAILURE);
        }
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            if (negativeCanMasks[i] == maybeID) {
                negativeCanMasks[i] = EMPTY_CAN_MASK_SLOT;
            }
        }
        printTypeResult(REMOVE_NEGATIVE_CAN_MASK_HEADER, str, OPERATION_SUCCESS);
    }

    void canLiveUpdateRequest(const char *str)
    {
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
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_FAILURE);
        }
        int8_t foundPosition{-1};
        for (uint8_t i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            if (lastCanMessages[i].id() == maybeID) {
                foundPosition = i;
            }
        }
        if (foundPosition != -1) {
            printCanResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, lastCanMessages[foundPosition], OPERATION_SUCCESS, NO_BROADCAST);
        } else {
            printBlankCanResult(CURRENT_CAN_MESSAGE_BY_ID_HEADER, OPERATION_SUCCESS);
        }
    }

    void clearCurrentMessageByIdRequest(const char *str)
    {
        uint32_t maybeID{stringToUInt(str)};
        if (maybeID == 0) {
            printTypeResult(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_FAILURE);
        }
        for (uint8_t i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            if (lastCanMessages[i].id() == maybeID) {
                lastCanMessages[i] = CanMessage{};
            }
        }
        printTypeResult(CLEAR_CAN_MESSAGE_BY_ID_HEADER, str, OPERATION_SUCCESS);
    }

    void currentCachedCanMessagesRequest()
    {
        for (uint8_t i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            printCanResult(CURRENT_CAN_MESSAGES_HEADER, lastCanMessages[i], OPERATION_SUCCESS, NO_BROADCAST);
        }
    }
        
    void clearCanMessagesRequest()
    {
        for (uint8_t i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            lastCanMessages[i] = CanMessage{};
        }
        printSingleResult(CLEAR_CAN_MESSAGES_HEADER, OPERATION_SUCCESS);
    }

    void currentPositiveCanMasksRequest()
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            printTypeResult(CURRENT_POSITIVE_CAN_MASKS_HEADER, positiveCanMasks[i], OPERATION_SUCCESS);
        }
    }

    void currentNegativeCanMasksRequest()
    {
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            printTypeResult(CURRENT_POSITIVE_CAN_MASKS_HEADER, negativeCanMasks[i], OPERATION_SUCCESS);
        }
    }

    void allCurrentCanMasksRequest()
    {
        currentPositiveCanMasksRequest();
        currentNegativeCanMasksRequest();
    }

    void clearPositiveCanMasksRequest()
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            positiveCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
        printSingleResult(CLEAR_POSITIVE_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    void clearNegativeCanMasksRequest()
    {
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            negativeCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
        printSingleResult(CLEAR_NEGATIVE_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    void clearAllCanMasksRequest()
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            positiveCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            negativeCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
        printSingleResult(CLEAR_ALL_CAN_MASKS_HEADER, OPERATION_SUCCESS);
    }

    int8_t addPositiveCanMask(uint32_t id)
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            if (positiveCanMasks[i] == id) {
                return 0;
            }
        }
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            if (positiveCanMasks[i] == EMPTY_CAN_MASK_SLOT) {
                positiveCanMasks[i] = id;
                return i;
            }
        }
        return -1;
    }

    int8_t addNegativeCanMask(uint32_t id)
    {
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            if (negativeCanMasks[i] == id) {
                return 0;
            }
        }
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            if (negativeCanMasks[i] == EMPTY_CAN_MASK_SLOT) {
                negativeCanMasks[i] = id;
                return i;
            }
        }
        return -1;
    }

    void addLastCanMessage(const CanMessage &msg)
    {
        for (uint8_t i = 0; i < MAX_LAST_CAN_MESSAGES; i++) {
            if ((lastCanMessages[i].id() == msg.id()) || (lastCanMessages[i].id() == 0)) {
                lastCanMessages[i] = msg;
            }
        }
    }

    void sendCanMessage(const CanMessage &msg)
    {
        canController->sendMsgBuf(msg.id(), msg.frameType(), msg.length(), msg.message());
    }

    void initializeCanMasks()
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            positiveCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            negativeCanMasks[i] = EMPTY_CAN_MASK_SLOT;
        }
    }

    uint8_t numberOfPositiveCanMasks()
    {
        uint8_t returnLength{0};
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            if (positiveCanMasks[i] != EMPTY_CAN_MASK_SLOT) {
                returnLength++;
            }
        }
        return returnLength;
    }

    uint8_t numberOfNegativeCanMasks()
    {
        uint8_t returnLength{0};
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            if (negativeCanMasks[i] != EMPTY_CAN_MASK_SLOT) {
                returnLength++;
            }
        }
        return returnLength;
    }

    bool positiveCanMaskExists(uint32_t targetMask)
    {
        for (uint8_t i = 0; i < MAX_POSITIVE_CAN_MASKS; i++) {
            if (positiveCanMasks[i] != EMPTY_CAN_MASK_SLOT) {
                if (targetMask == positiveCanMasks[i]) {
                    return true;
                }
            }
        }
        return false;
    }

    bool negativeCanMaskExists(uint32_t targetMask)
    {
        for (uint8_t i = 0; i < MAX_NEGATIVE_CAN_MASKS; i++) {
            if (negativeCanMasks[i] != EMPTY_CAN_MASK_SLOT) {
                if (targetMask == negativeCanMasks[i]) {
                    return true;
                }
            }
        }
        return false;
    }


#endif
