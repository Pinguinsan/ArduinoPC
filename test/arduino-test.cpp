#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <serialport.h>
#include <udpduplex.h>
#include <tstream.h>
#include <generalutilities.h>
#include <arduino.h>
#include <prettyprinter.h>

static const ForegroundColor FAILURE_COLOR{ForegroundColor::FG_RED};
static const ForegroundColor SUCCESS_COLOR{ForegroundColor::FG_GREEN};
static const ForegroundColor LIST_COLOR{ForegroundColor::FG_YELLOW};
static const int COMMON_ATTRIBUTES{(FontAttribute::FA_BOLD | FontAttribute::FA_UNDERLINED)};

static std::vector<const char *> STEP_THROUGH_SWITCHES{"-s", "--s", "-step", "--step"};
static bool stepThrough{false};
template <typename T>
bool alwaysTrue(T param)
{
    (void)param;
    return true;
}

int main(int argc, char *argv[])
{
    /*
    //std::string serialPortName{SerialPort::doUserSelectSerialPortName()};
    //std::unique_ptr<Arduino> arduino{std::make_unique<Arduino>(ArduinoType::NANO, serialPortName)};

    //const std::string SERIAL_PORT_NAME{"/dev/ttyACM0"};
    //std::shared_ptr<TStream> serialPort{std::make_shared<SerialPort>(serialPortName,
                                                                        Arduino::FIRMWARE_BAUD_RATE,
                                                                        Arduino::FIRMWARE_DATA_BITS,
                                                                        Arduino::FIRMWARE_STOP_BITS,
                                                                        Arduino::FIRMWARE_PARITY)};
    
    
    */
    static const char *ANY_KEY_TO_CONTINUE_STRING{"Press any key to continue..."};  
    for (int i = 0; i < argc; i++) {
        if (GeneralUtilities::isSwitch(argv[i], STEP_THROUGH_SWITCHES)) {
            stepThrough = true;
        }
    }

    std::vector<int> pins{2, 3, 4, 5, 6};
    std::string clientHostName{UDPDuplex::doUserSelectClientHostName()};
    uint16_t clientPortNumber{UDPDuplex::doUserSelectClientPortNumber()};
    uint16_t serverPortNumber{UDPDuplex::doUserSelectServerPortNumber()};
    uint32_t delayBetween{GeneralUtilities::doUserEnterNumericParameter("On-Off Delay",
                                                                        static_cast<std::function<bool(uint32_t)>>(alwaysTrue<uint32_t>),
                                                                        std::numeric_limits<uint32_t>::min(),
                                                                        std::numeric_limits<uint32_t>::max())};
            
    
    uint32_t arduinoStreamDelay{GeneralUtilities::doUserEnterNumericParameter("Arduino Stream Delay",
                                                                        static_cast<std::function<bool(uint32_t)>>(alwaysTrue<uint32_t>),
                                                                        std::numeric_limits<uint32_t>::min()+1,
                                                                        std::numeric_limits<uint32_t>::max())};

    uint32_t arduinoIOTryCount{GeneralUtilities::doUserEnterNumericParameter("Arduino IO Try Count",
                                                                        static_cast<std::function<bool(uint32_t)>>(alwaysTrue<uint32_t>),
                                                                        std::numeric_limits<uint32_t>::min()+1,
                                                                        std::numeric_limits<uint32_t>::max())};
            

    std::unique_ptr<PrettyPrinter> prettyPrinter{std::make_unique<PrettyPrinter>()};

    std::string throwAway{""};
    prettyPrinter->setFontAttributes(COMMON_ATTRIBUTES);
    prettyPrinter->setForegroundColor(LIST_COLOR);
    std::cout << "Using ClientHostName=";
    prettyPrinter->println(clientHostName);
    
    std::cout << "Using ClientPortNumber=";
    prettyPrinter->println(clientPortNumber);
    
    std::cout << "Using ServerPortNumber=";
    prettyPrinter->println(serverPortNumber);
    
    std::cout << "Using DelayBetween=";
    prettyPrinter->println(delayBetween);
    
    std::cout << "Using ArduinoStreamDelay=";
    prettyPrinter->println(arduinoStreamDelay);
    
    std::cout << "Using ArduinoIOTryCount=";
    prettyPrinter->println(arduinoIOTryCount);

    if (stepThrough) {
        prettyPrinter->println("Using StepThroughMode");
    }
    prettyPrinter->println();
    std::shared_ptr<TStream> serialPort{std::make_shared<UDPDuplex>(clientHostName, clientPortNumber, serverPortNumber, UDPObjectType::UDP_DUPLEX)};
    std::cout << "Creating Arduino object using serial port " << std::quoted(serialPort->portName()) << "..."; 
    std::unique_ptr<Arduino> arduino{std::make_unique<Arduino>(ArduinoType::MEGA, serialPort)};
    prettyPrinter->setForegroundColor(SUCCESS_COLOR);
    prettyPrinter->println("success");;
    arduino->setIOTryCount(arduinoIOTryCount);
    arduino->setStreamSendDelay(arduinoStreamDelay);
    serialPort->openPort();
    do {
        std::cout << "Writing all pins HIGH...";
        auto result = arduino->digitalWriteAll(1);
        if (result.first == IOStatus::OPERATION_SUCCESS) {
            prettyPrinter->setForegroundColor(SUCCESS_COLOR);
            prettyPrinter->print("success (");
            for (auto it = result.second.begin(); it != result.second.end(); it++) {
                prettyPrinter->print(*it);
                if (it+1 != result.second.end()) {
                    prettyPrinter->print(",");
                }
            }
            prettyPrinter->println(")");
        } else {
            prettyPrinter->setForegroundColor(FAILURE_COLOR);
            prettyPrinter->println("failure");
        }
        if (stepThrough) {
            std::getline(std::cin, throwAway);
        } else {
            GeneralUtilities::delayMilliseconds(delayBetween);
        }
        for (auto &it : pins) {
            std::cout << "Writing pin " << it << " LOW...";
            if (arduino->digitalWrite(it, 0).first == IOStatus::OPERATION_SUCCESS) {
                prettyPrinter->setForegroundColor(SUCCESS_COLOR);
                prettyPrinter->println("success");
            } else {
                prettyPrinter->setForegroundColor(FAILURE_COLOR);
                prettyPrinter->println("failure");
            }

            if (stepThrough) {
                std::getline(std::cin, throwAway);
            } else {
                GeneralUtilities::delayMilliseconds(delayBetween);
            }
            
            std::cout << "Writing pin " << it << " HIGH...";
            if (arduino->digitalWrite(it, 1).first == IOStatus::OPERATION_SUCCESS) {
                prettyPrinter->setForegroundColor(SUCCESS_COLOR);
                prettyPrinter->println("success");
            } else {
                prettyPrinter->setForegroundColor(FAILURE_COLOR);
                prettyPrinter->println("failure");
            }            
            if (stepThrough) {
                std::getline(std::cin, throwAway);
            } else {
                GeneralUtilities::delayMilliseconds(delayBetween);
            }
        }
        for (int i = 0; i < 3; i++) {
            if (i != 0) {
                std::cout << "Writing all pins HIGH...";
                auto result = arduino->digitalWriteAll(1);
                if (result.first == IOStatus::OPERATION_SUCCESS) {
                    prettyPrinter->setForegroundColor(SUCCESS_COLOR);
                    prettyPrinter->print("success (");
                    for (auto it = result.second.begin(); it != result.second.end(); it++) {
                        prettyPrinter->print(*it);
                        if (it+1 != result.second.end()) {
                            prettyPrinter->print(",");
                        }
                    }
                    prettyPrinter->println(")");
                } else {
                    prettyPrinter->setForegroundColor(FAILURE_COLOR);
                    prettyPrinter->println("failure");
                }
                if (stepThrough) {
                    std::getline(std::cin, throwAway);
                } else {
                    GeneralUtilities::delayMilliseconds(delayBetween);
                }
            }
            std::cout << "Writing all pins LOW...";
            result = arduino->digitalWriteAll(0);
            if (result.first == IOStatus::OPERATION_SUCCESS) {
                prettyPrinter->setForegroundColor(SUCCESS_COLOR);
                prettyPrinter->print("success (");
                for (auto it = result.second.begin(); it != result.second.end(); it++) {
                    prettyPrinter->print(*it);
                    if (it+1 != result.second.end()) {
                        prettyPrinter->print(",");
                    }
                }
                prettyPrinter->println(")");
            } else {
                prettyPrinter->setForegroundColor(FAILURE_COLOR);
                prettyPrinter->println("failure");
            }
            if (stepThrough) {
                std::getline(std::cin, throwAway);
            } else {
                GeneralUtilities::delayMilliseconds(delayBetween);
            }
        }
    } while(true);
    return 0;
}
