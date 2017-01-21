#include "arduino.h"

const BaudRate Arduino::FIRMWARE_BAUD_RATE{BaudRate::BAUD115200};
const DataBits Arduino::FIRMWARE_DATA_BITS{DataBits::EIGHT};
const StopBits Arduino::FIRMWARE_STOP_BITS{StopBits::ONE};
const Parity Arduino::FIRMWARE_PARITY{Parity::NONE};
const int Arduino::ANALOG_MAX{1023};
const double Arduino::VOLTAGE_MAX{5.0};
const unsigned int Arduino::DEFAULT_IO_TRY_COUNT{3};

Arduino::Arduino(ArduinoType arduinoType, std::shared_ptr<TStream> tStream) :
    m_arduinoType{arduinoType},
    m_ioStream{tStream},
    m_streamSendDelay{DEFAULT_IO_STREAM_SEND_DELAY},
    m_ioTryCount{DEFAULT_IO_TRY_COUNT}
{
    try {
        if (!this->m_ioStream->isOpen()) {
            this->m_ioStream->openPort();
            GeneralUtilities::delayMilliseconds(BOOTLOADER_BOOT_TIME);
        }
    } catch (std::exception &e) {
        throw e;
    }
    //TODO: Validate type of Arduino
    this->assignPinsAndIdentifiers();
}

void Arduino::assignPinsAndIdentifiers()
{
   if (this->m_arduinoType == ArduinoType::UNO) {
        this->m_availablePins = ArduinoUno::s_availablePins;
        this->m_availablePwmPins = ArduinoUno::s_availablePwmPins;
        this->m_availableAnalogPins = ArduinoUno::s_availableAnalogPins;
        this->m_numberOfDigitalPins = ArduinoUno::s_numberOfDigitalPins;
        this->m_identifier = ArduinoUno::IDENTIFIER;
        this->m_longName = ArduinoUno::LONG_NAME;
    } else if (this->m_arduinoType == ArduinoType::NANO) {
        this->m_availablePins = ArduinoNano::s_availablePins;
        this->m_availablePwmPins = ArduinoNano::s_availablePwmPins;
        this->m_availableAnalogPins = ArduinoNano::s_availableAnalogPins;
        this->m_numberOfDigitalPins = ArduinoNano::s_numberOfDigitalPins;
        this->m_identifier = ArduinoNano::IDENTIFIER;
        this->m_longName = ArduinoNano::LONG_NAME;
    } else if (this->m_arduinoType == ArduinoType::MEGA) {
        this->m_availablePins = ArduinoMega::s_availablePins;
        this->m_availablePwmPins = ArduinoMega::s_availablePwmPins;
        this->m_availableAnalogPins = ArduinoMega::s_availableAnalogPins;
        this->m_numberOfDigitalPins = ArduinoMega::s_numberOfDigitalPins;
        this->m_identifier = ArduinoMega::IDENTIFIER;
        this->m_longName = ArduinoMega::LONG_NAME;
    }
    for (auto &it : this->m_availablePins) {
        if (isValidAnalogInputPin(it)) {
            this->m_gpioPins.emplace(it, std::make_shared<GPIO>(it, IOType::ANALOG_INPUT));
        } else {
            this->m_gpioPins.emplace(it, std::make_shared<GPIO>(it, IOType::DIGITAL_INPUT_PULLUP));
        }
    }
}

std::string Arduino::serialPortName() const
{
    return this->m_ioStream->portName();
}

ArduinoType Arduino::arduinoType() const
{
    return this->m_arduinoType;
}

std::string Arduino::identifier() const
{
    return this->m_identifier;
}

std::string Arduino::longName() const
{
    return this->m_longName;
}

unsigned int Arduino::streamSendDelay() const
{
    return this->m_streamSendDelay;
}

unsigned int Arduino::ioTryCount() const
{
    return this->m_ioTryCount;
}

void Arduino::setStreamSendDelay(unsigned int streamSendDelay)
{
    this->m_streamSendDelay = streamSendDelay;
}


void Arduino::setIOTryCount(unsigned int ioTryCount)
{
    if (ioTryCount < 1) {
        throw std::runtime_error(IO_TRY_COUNT_TOO_LOW_STRING + std::to_string(ioTryCount) + " < 1) ");
    }
    this->m_ioTryCount = ioTryCount;
}

std::vector<std::string> Arduino::genericIOTask(const std::string &stringToSend, const std::string &header, double delay)
{
    std::lock_guard<std::mutex> ioLock{this->m_ioMutex};
    if (!this->m_ioStream->isOpen()) {
        this->m_ioStream->openPort();
        GeneralUtilities::delayMilliseconds(BOOTLOADER_BOOT_TIME);
    }
    unsigned long int tempTimeout{this->m_ioStream->timeout()};
    this->m_ioStream->setTimeout(SERIAL_REPORT_REQUEST_TIME_LIMIT);
    this->m_ioStream->writeString(stringToSend);
    GeneralUtilities::delayMilliseconds(delay);
    std::unique_ptr<std::string> returnString{std::make_unique<std::string>("")};
    EventTimer eventTimer;
    eventTimer.start();
    do {
        std::string str{this->m_ioStream->readStringUntil("}")};
        if (str != "") {
            *returnString = str;
            break;
        }
        eventTimer.update();
    } while (eventTimer.totalMilliseconds() < this->m_ioStream->timeout());
    this->m_ioStream->setTimeout(tempTimeout);
    if (GeneralUtilities::startsWith(*returnString, header) && GeneralUtilities::endsWith(*returnString, '}')) {
        *returnString = returnString->substr(static_cast<std::string>(header).length() + 1);
        *returnString = returnString->substr(0, returnString->length()-1);
    } else {
        return std::vector<std::string>{};
    }
    return GeneralUtilities::parseToVector(*returnString, ':');
}

std::vector<std::string> Arduino::genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, double delay)
{
    std::lock_guard<std::mutex> ioLock{this->m_ioMutex};
    if (!this->m_ioStream->isOpen()) {
        this->m_ioStream->openPort();
        GeneralUtilities::delayMilliseconds(BOOTLOADER_BOOT_TIME);
    }
    this->m_ioStream->writeString(stringToSend);
    GeneralUtilities::delayMilliseconds(delay);
    std::unique_ptr<std::string> returnString{std::make_unique<std::string>("")};
    EventTimer eventTimer;
    eventTimer.start();
    do {
        std::string str{this->m_ioStream->readStringUntil("}")};
        if (str != "") {
            *returnString = str;
            break;
        }
        eventTimer.update();
    } while (eventTimer.totalMilliseconds() < this->m_ioStream->timeout());
    if (GeneralUtilities::startsWith(*returnString, header) && GeneralUtilities::endsWith(*returnString, endHeader)) {
        *returnString = returnString->substr(static_cast<std::string>(header).length() + 1);
        *returnString = returnString->substr(0, returnString->length()-1);
    } else {
        return std::vector<std::string>{};
    }
    return GeneralUtilities::parseToVector(*returnString, ';');
}

std::pair<IOStatus, std::string> Arduino::arduinoTypeString()
{
    std::string stringToSend{static_cast<std::string>(ARDUINO_TYPE_HEADER) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(ARDUINO_TYPE_HEADER), this->m_streamSendDelay)};
        if (states.size() != ARDUINO_TYPE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        if (states.at(ArduinoTypeEnum::OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, states.at(ArduinoTypeEnum::RETURN_STATE));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, "");
}

std::pair<IOStatus, std::string> Arduino::firmwareVersion()
{
    std::string stringToSend{static_cast<std::string>(FIRMWARE_VERSION_HEADER) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(FIRMWARE_VERSION_HEADER), this->m_streamSendDelay)};
        if (states.size() != ARDUINO_TYPE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        if (states.at(ArduinoTypeEnum::OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, states.at(ArduinoTypeEnum::RETURN_STATE));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, "");
}

std::pair<IOStatus, bool> Arduino::canCapability()
{
    std::string stringToSend{static_cast<std::string>(CAN_BUS_ENABLED_HEADER) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(CAN_BUS_ENABLED_HEADER), this->m_streamSendDelay)};
        if (states.size() != CAN_BUS_ENABLED_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            }
        }
        if (states.at(CanEnabledStatus::CAN_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, (GeneralUtilities::decStringToInt(states.at(CanEnabledStatus::CAN_RETURN_STATE)) == 1));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, int> Arduino::analogToDigitalThreshold()
{
    std::string stringToSend{static_cast<std::string>(CURRENT_A_TO_D_THRESHOLD_HEADER) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(CURRENT_A_TO_D_THRESHOLD_HEADER), this->m_streamSendDelay)};
        if (states.size() != A_TO_D_THRESHOLD_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(ADThresholdReq::AD_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(ADThresholdReq::AD_RETURN_STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

IOReport Arduino::ioReportRequest()
{
    std::string stringToSend{static_cast<std::string>(IO_REPORT_HEADER) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> allStates{genericIOReportTask(stringToSend, 
                                                               static_cast<std::string>(IO_REPORT_HEADER), 
                                                               static_cast<std::string>(IO_REPORT_END_HEADER) + "}", 
                                                               100)};
        IOReport ioReport;
        for (auto &it : allStates) {
            it = GeneralUtilities::stripAllFromString(it, '}');
            it = GeneralUtilities::stripAllFromString(it, '{');
            std::vector<std::string> states{GeneralUtilities::parseToVector(it, ':')};
            std::string endCheck{GeneralUtilities::stripAllFromString(IO_REPORT_END_HEADER, '{')};
            endCheck = GeneralUtilities::stripAllFromString(endCheck, '}');
            if (it.find(endCheck) != std::string::npos) {
                continue;
            }
            if ((states.size() != IO_REPORT_RETURN_SIZE) && (states.size() != 0)) {
                if (i+1 == this->m_ioTryCount) {
                    throw std::runtime_error(IO_REPORT_INVALID_DATA_STRING);
                } else {
                    break;
                }
            }
            if (states.size() == 0) {
                continue;
            }
            IOType ioType{parseIOTypeFromString(states.at(IOReportEnum::IO_TYPE))};
            if ((ioType == IOType::DIGITAL_INPUT) || (ioType == IOType::DIGITAL_INPUT_PULLUP)) {
                ioReport.addDigitalInputResult(std::make_pair(GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_PIN_NUMBER)), GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::DIGITAL_OUTPUT) {
                ioReport.addDigitalOutputResult(std::make_pair(GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_PIN_NUMBER)), GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::ANALOG_INPUT) {
                ioReport.addAnalogInputResult(std::make_pair(GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_PIN_NUMBER)), GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::ANALOG_OUTPUT) {
                ioReport.addAnalogOutputResult(std::make_pair(GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_PIN_NUMBER)), GeneralUtilities::decStringToInt(states.at(IOReportEnum::IO_STATE))));
            }
        }
        return ioReport;
    }
    return IOReport{};
}

SerialReport Arduino::serialReportRequest(const std::string &delimiter)
{
    std::lock_guard<std::mutex> ioLock{this->m_ioMutex};
    if (!this->m_ioStream->isOpen()) {
        this->m_ioStream->openPort();
        GeneralUtilities::delayMilliseconds(BOOTLOADER_BOOT_TIME);
    }
    SerialReport serialReport;
    std::unique_ptr<std::string> readString{std::make_unique<std::string>("")};
    std::unique_ptr<EventTimer> eventTimer{std::make_unique<EventTimer>()};
    std::unique_ptr<EventTimer> overallTimeout{std::make_unique<EventTimer>()};
    std::string returnString{""};
    eventTimer->start();
    overallTimeout->start();
    if (delimiter == "") {
        std::vector<std::string> stringsToPrint;
        if (returnString.find("}{") != std::string::npos) {
            while (returnString.find("}{") != std::string::npos) {
                stringsToPrint.emplace_back(returnString.substr(0, returnString.find("}{")+1));
                returnString = returnString.substr(returnString.find("}{") + static_cast<std::string>("}{").length()-1);
            }
        } else {
            stringsToPrint.emplace_back(returnString);
        }
        for (auto &it : stringsToPrint) {
            serialReport.addSerialResult(it);
        }
       return serialReport;
    } else {
        do {
            eventTimer->update();
            overallTimeout->update();
            *readString = this->m_ioStream->readString();
            if ((*readString == "") || (GeneralUtilities::isWhitespace(*readString))) {
                continue;
            } else {
                eventTimer->restart();
                returnString += *readString;
            }
        } while ((eventTimer->totalMilliseconds() <= SERIAL_REPORT_REQUEST_TIME_LIMIT) &&
                 (overallTimeout->totalMilliseconds() <= SERIAL_REPORT_OVERALL_TIME_LIMIT) &&
                 (!GeneralUtilities::endsWith(returnString, delimiter)));
    }
    std::vector<std::string> stringsToPrint;
    if (returnString.find("}{") != std::string::npos) {
        while (returnString.find("}{") != std::string::npos) {
            stringsToPrint.emplace_back(returnString.substr(0, returnString.find("}{")+1));
            returnString = returnString.substr(returnString.find("}{") + static_cast<std::string>("}{").length()-1);
        }
    } else {
        stringsToPrint.emplace_back(returnString);
    }
    for (auto &it : stringsToPrint) {
        serialReport.addSerialResult(it);
    }
    return serialReport;
}

std::pair<IOStatus, int> Arduino::setAnalogToDigitalThreshold(int threshold)
{
    std::string stringToSend{static_cast<std::string>(CHANGE_A_TO_D_THRESHOLD_HEADER) + ":" + std::to_string(threshold) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(CHANGE_A_TO_D_THRESHOLD_HEADER), this->m_streamSendDelay)};
        if (states.size() != A_TO_D_THRESHOLD_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(threshold) != states.at(ADThresholdReq::AD_RETURN_STATE)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(ADThresholdReq::AD_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(ADThresholdReq::AD_RETURN_STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}


std::pair<IOStatus, IOType> Arduino::pinMode(int pinNumber, IOType ioType)
{
    std::string stringToSend{static_cast<std::string>(PIN_TYPE_CHANGE_HEADER) + ":" + std::to_string(pinNumber) + ":" + parseIOType(ioType) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(PIN_TYPE_CHANGE_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, parseIOTypeFromString(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
}


std::pair<IOStatus, IOType> Arduino::currentPinMode(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(PIN_TYPE_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(PIN_TYPE_HEADER), this->m_streamSendDelay)};
        if (states.size() != PIN_TYPE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, parseIOTypeFromString(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
}

std::pair<IOStatus, bool> Arduino::digitalRead(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(DIGITAL_READ_HEADER) + ":" + std::to_string(pinNumber) + "}" };
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(DIGITAL_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)) == 1);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, bool> Arduino::digitalWrite(int pinNumber, bool state)
{
    std::string stringToSend{static_cast<std::string>(DIGITAL_WRITE_HEADER) + ":" + std::to_string(pinNumber) + ":" + std::to_string(state) + "}" };
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(DIGITAL_WRITE_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)) == 1);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, std::vector<int>> Arduino::digitalWriteAll(bool state)
{
    std::vector<int> writtenPins;
    std::string stringToSend{static_cast<std::string>(DIGITAL_WRITE_ALL_HEADER) + ":" + std::to_string(state) + "}" };
    for (int i = 0; i < this->m_ioTryCount; i++) {
        writtenPins = std::vector<int>{};
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(DIGITAL_WRITE_ALL_HEADER), this->m_streamSendDelay)};
        if (states.size() == 0) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
            } else {
                continue;
            }
        }
        if (states.size() < DIGITAL_WRITE_ALL_MINIMIM_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
            } else {
                continue;
            }
        }
        if (*(states.end()-1) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
            } else {
                continue;
            }
        }
        states.pop_back();
        if (*(states.end()-1) != (state ? "1" : "0")) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
            } else {
                continue;
            }
        }
        states.pop_back();
        try {
            for (auto &it : states) {
                writtenPins.push_back(GeneralUtilities::decStringToInt(it));
            }
            std::sort(writtenPins.begin(), writtenPins.end());
            return std::make_pair(IOStatus::OPERATION_SUCCESS, writtenPins);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, std::vector<int>{});
}

std::pair<IOStatus, bool> Arduino::softDigitalRead(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(SOFT_DIGITAL_READ_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(SOFT_DIGITAL_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::analogRead(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(ANALOG_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, analogToVoltage(GeneralUtilities::decStringToInt(states.at(IOState::STATE))));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::analogReadRaw(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(ANALOG_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::softAnalogRead(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(SOFT_ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(SOFT_ANALOG_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, analogToVoltage(GeneralUtilities::decStringToInt(states.at(IOState::STATE))));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::softAnalogReadRaw(int pinNumber)
{
    std::string stringToSend{static_cast<std::string>(SOFT_ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(SOFT_ANALOG_READ_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::analogWrite(int pinNumber, double state)
{
    std::string stringToSend{static_cast<std::string>(ANALOG_WRITE_HEADER) + ":" + std::to_string(voltageToAnalog(pinNumber)) + ":" + std::to_string(state) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(ANALOG_WRITE_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToDouble(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::analogWriteRaw(int pinNumber, int state)
{
    std::string stringToSend{static_cast<std::string>(ANALOG_WRITE_HEADER) + ":" + std::to_string(pinNumber) + ":" + std::to_string(state) + "}"};
    for (int i = 0; i < this->m_ioTryCount; i++) {
        std::vector<std::string> states{genericIOTask(stringToSend, static_cast<std::string>(ANALOG_WRITE_HEADER), this->m_streamSendDelay)};
        if (states.size() != IO_STATE_RETURN_SIZE) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, GeneralUtilities::decStringToInt(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == this->m_ioTryCount) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

bool Arduino::isValidAnalogPinIdentifier(const std::string &state) const
{
    for (auto &it : this->m_availableAnalogPins) {
        if (state == analogPinFromNumber(this->m_arduinoType, it)) {
            return true;
        }
    }
    return false;
}

bool Arduino::isValidDigitalOutputPin(int pinNumber) const
{
    for (auto &it : this->m_availableAnalogPins) {
        if (pinNumber == it) {
            return true;
        }
    }
    for (auto &it : this->m_availablePins) {
        if (pinNumber == it) {
            return true;
        }
    }
    return false;
}

bool Arduino::isValidDigitalInputPin(int pinNumber) const
{
    for (auto &it : this->m_availableAnalogPins) {
        if (pinNumber == it) {
            return true;
        }
    }
    for (auto &it : this->m_availablePins) {
        if (pinNumber == it) {
            return true;
        }
    }
    return false;
}

bool Arduino::isValidAnalogOutputPin(int pinNumber) const
{
    for (auto &it : this->m_availablePwmPins) {
        if (pinNumber == it) {
            return true;
        }
    }
    return false;
}

bool Arduino::isValidAnalogInputPin(int pinNumber) const
{
    for (auto &it : this->m_availableAnalogPins) {
        if (pinNumber == it) {
            return true;
        }
    }
    return false;
}

std::set<int> Arduino::AVAILABLE_ANALOG_PINS() const
{
    return this->m_availableAnalogPins;
}

std::set<int> Arduino::AVAILABLE_PWM_PINS() const
{
    return this->m_availablePwmPins;
}

std::set<int> Arduino::AVAILABLE_PINS() const
{
    return this->m_availablePins;
}

int Arduino::NUMBER_OF_DIGITAL_PINS() const
{
    return this->m_numberOfDigitalPins;
}

std::set<int> ArduinoUno::s_availableAnalogPins{14, 15, 16, 17, 18, 19};
std::set<int> ArduinoUno::s_availablePwmPins{3, 5, 6, 9, 10, 11};
std::set<int> ArduinoUno::s_availablePins{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
const char *ArduinoUno::IDENTIFIER{"arduino_uno"};
const char *ArduinoUno::LONG_NAME{"Arduino Uno"};
int ArduinoUno::s_numberOfDigitalPins{13};

std::set<int> ArduinoNano::s_availableAnalogPins{14, 15, 16, 17, 18, 19, 20, 21};
std::set<int> ArduinoNano::s_availablePwmPins{3, 5, 6, 9, 10, 11};
std::set<int> ArduinoNano::s_availablePins{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
const char *ArduinoNano::IDENTIFIER{"arduino_nano"};
const char *ArduinoNano::LONG_NAME{"Arduino Nano"};
int ArduinoNano::s_numberOfDigitalPins{13};

std::set<int> ArduinoMega::s_availableAnalogPins{54, 55, 56, 57, 58, 59, 60, 61,
                                                 62, 63, 64, 65, 66, 67, 68, 69};
std::set<int> ArduinoMega::s_availablePwmPins{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, 46};
std::set<int> ArduinoMega::s_availablePins{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                           13, 14, 15, 16, 17, 18, 19, 20, 21,
                                           22, 23, 24, 25, 26, 27, 28, 29, 30,
                                           31, 32, 33, 34, 35, 36, 37, 38, 39,
                                           40, 41, 42, 43, 44, 45, 46, 47, 48,
                                           49, 50, 51, 52, 53, 54, 55, 56, 57,
                                           58, 59, 60, 61, 62, 63, 64, 65, 66,
                                           67, 68, 69};
const char *ArduinoMega::IDENTIFIER{"arduino_mega"};
const char *ArduinoMega::LONG_NAME{"Arduino Mega"};
int ArduinoMega::s_numberOfDigitalPins{53};
