#include "arduino.h"

std::shared_ptr<Arduino> Arduino::makeArduino(std::shared_ptr<SerialPort> serialPort)
{
    using namespace GeneralUtilities;
    std::string arduinoType{""};
    for (unsigned int tryCount = 0; tryCount <  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT(); tryCount++) {
        try {
            std::pair<IOStatus, std::string> retrieveArduinoType{Arduino::getArduinoType(serialPort)};
            if (retrieveArduinoType.first == IOStatus::OPERATION_SUCCESS) {
                arduinoType = static_cast<std::string>(retrieveArduinoType.second);
                if (arduinoType == static_cast<std::string>(ArduinoUno::IDENTIFIER)) {
                    tryCount =  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
                    arduinoType = ArduinoUno::IDENTIFIER;
                } else if (arduinoType == static_cast<std::string>(ArduinoNano::IDENTIFIER)) {
                    tryCount =  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
                    arduinoType = ArduinoNano::IDENTIFIER;
                } else if (arduinoType == static_cast<std::string>(ArduinoMega::IDENTIFIER)) {
                    tryCount = Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
                    arduinoType = ArduinoMega::IDENTIFIER;
                } else {
                    if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                        return nullptr;
                    }
                }
            } else {
                if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                    return nullptr;
                }
            }
        } catch (std::exception &e) {
            if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                return nullptr;
            }
        }
    }
    delayMilliseconds(50);
    std::pair<IOStatus, std::string> firmwareVersion;
    for (unsigned int tryCount = 0; tryCount <  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT(); tryCount++) {
        try {
            firmwareVersion = Arduino::getFirmwareVersion(serialPort);
            if (firmwareVersion.first == IOStatus::OPERATION_SUCCESS) {
                tryCount =  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
            } else {
                if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                    return nullptr;
                }
            }
        } catch (std::exception &e) {
            if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                return nullptr;
            }
        }
    }
    std::pair<IOStatus, int> analogToDigitalThreshold;
    for (unsigned int tryCount = 0; tryCount <  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT(); tryCount++) {
        try {
            analogToDigitalThreshold = Arduino::getAnalogToDigitalThreshold(serialPort);
            if (analogToDigitalThreshold.first == IOStatus::OPERATION_SUCCESS) {
                tryCount =  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
            } else {
                if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                    return nullptr;
                }
            }
        } catch (std::exception &e) {
            (void)e;
            if (tryCount == ( Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()-1)) {
                return nullptr;
            }
        }
    }
    std::pair<IOStatus, bool> can;
    for (unsigned int tryCount = 0; tryCount <  Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT(); tryCount++) {
        try {
            can = Arduino::getCanCapability(serialPort);
            if (can.first == IOStatus::OPERATION_SUCCESS) {
                tryCount = Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT();
            }
        } catch (std::exception &e) {
            (void)e;
        }
    }
    if (firmwareVersion.first == IOStatus::OPERATION_FAILURE) {
        return nullptr;
    }
    if (can.first == IOStatus::OPERATION_FAILURE) {
        return nullptr;
    }
    if (arduinoType == "") {
        return nullptr;
    }
    if (analogToDigitalThreshold.first == IOStatus::OPERATION_FAILURE) {
        return nullptr;
    }

    if (arduinoType == ArduinoUno::IDENTIFIER) {
        return std::shared_ptr<Arduino>(new Arduino(ArduinoType::UNO, serialPort, firmwareVersion.second, can.second));
    } else if (arduinoType == ArduinoNano::IDENTIFIER) {
        return std::shared_ptr<Arduino>(new Arduino(ArduinoType::NANO, serialPort, firmwareVersion.second, can.second));
    } else if (arduinoType == ArduinoMega::IDENTIFIER) {
        return std::shared_ptr<Arduino>(new Arduino(ArduinoType::MEGA, serialPort, firmwareVersion.second, can.second));
    } else {
        return nullptr;
    }
}

Arduino::Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort) :
    m_arduinoType{arduinoType},
    m_serialPortIndex{Arduino::addSerialPort(serialPort)},
    m_firmwareVersion{""},
    m_canCapability{std::make_pair(false, false)},
    m_canPinAlias{""},
    m_analogToDigitalThreshold{-1}

{
    this->assignPinsAndIdentifiers();
}

Arduino::Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, bool canCapability) :
    m_arduinoType{arduinoType},
    m_serialPortIndex{Arduino::addSerialPort(serialPort)},
    m_firmwareVersion{""},
    m_canCapability{std::make_pair(true, canCapability)},
    m_canPinAlias{""},
    m_analogToDigitalThreshold{-1}
{
    this->assignPinsAndIdentifiers();
}

Arduino::Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, const std::string &firmwareVersion) :
    m_arduinoType{arduinoType},
    m_serialPortIndex{Arduino::addSerialPort(serialPort)},
    m_firmwareVersion{firmwareVersion},
    m_canCapability{std::make_pair(false, false)},
    m_canPinAlias{""},
    m_analogToDigitalThreshold{-1}
{
    this->assignPinsAndIdentifiers();
}

Arduino::Arduino(ArduinoType arduinoType, std::shared_ptr<SerialPort> serialPort, const std::string &firmwareVersion, bool canCapability) :
    m_arduinoType{arduinoType},
    m_serialPortIndex{Arduino::addSerialPort(serialPort)},
    m_firmwareVersion{firmwareVersion},
    m_canCapability{std::make_pair(true, canCapability)},
    m_canPinAlias{""},
    m_analogToDigitalThreshold{-1}
{
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
        this->m_gpioPins.emplace(it, std::make_shared<GPIO>(it, IOType::UNSPECIFIED));
    }

    for (auto &it : this->m_gpioPins) {
        if (it.first > this->m_numberOfDigitalPins) {
            if (it.first == CAN_BUS_PIN) {
                this->m_canPinAlias = std::to_string(it.first);
            }
            this->m_gpioPinsAlias.emplace(analogPinFromNumber(it.first), it.second);
            this->m_gpioPinIterationAliasMap.emplace(it.first, analogPinFromNumber(it.first));
        } else {
            this->m_gpioPinsAlias.emplace(std::to_string(it.first), it.second);
            this->m_gpioPinIterationAliasMap.emplace(it.first, std::to_string(it.first));
        }
    }
    if (this->m_canCapability.second) {
        this->eraseCanPin();
    }
    assignIOTypes();
}

void Arduino::assignAliasesFromMap(const std::map<int, std::string> aliasesMap)
{
    using namespace GeneralUtilities;
    for (auto &it : aliasesMap) {
        if (confirmValidAlias(it)) {
            std::cout << USING_ALIAS_STRING << tQuoted(it.second) << FOR_PIN_NUMBER_STRING << it.first;
            std::cout << " (" << this->identifier() << ", " << this->serialPortName() << ")" << std::endl;
        }
        if (this->m_gpioPinsAlias.find(std::to_string(it.first)) != this->m_gpioPinsAlias.end()) {
            auto iter = this->m_gpioPinsAlias.find(std::to_string(it.first));
            std::swap(this->m_gpioPinsAlias[std::to_string(it.first)], iter->second);
            this->m_gpioPinsAlias.erase(iter);
            this->m_gpioPinIterationAliasMap.find(it.first)->second = it.second;
        }
    }
}

void Arduino::assignIOTypesFromMap(const std::map<int, std::string> ioTypesMap)
{
    using namespace GeneralUtilities;
    for (auto &it : ioTypesMap) {
        if (confirmValidIOType(it)) {
            auto found{this->m_gpioPins.find(it.first)};
            if (found != this->m_gpioPins.end()) {
                found->second->setIOType(parseIOTypeFromString(it.second));
                std::string stringToLog{toString(SETTING_IO_TYPE_STRING) + tQuoted(it.second) + toString(FOR_PIN_NUMBER_STRING) + toString(it.first) + toString(ELIPSES_STRING)};
                std::pair<IOStatus, IOType> result{this->pinMode(found->second, found->second->ioType())};
                if (result.first == IOStatus::OPERATION_FAILURE) {
                    stringToLog.append(GENERIC_FAILED_STRING);
                } else {
                    stringToLog.append(GENERIC_SUCCESS_STRING);
                }
                std::cout << stringToLog;
                std::cout << " (" << this->identifier() << ", " << this->serialPortName() << ")" << std::endl;
            }
        }
    }
}

void Arduino::assignInitialStatesFromMap(const std::map<int, std::string> initialStatesMap)
{
    using namespace GeneralUtilities;
    for (auto &it : initialStatesMap) {
        std::string logString{toString(USING_INITIAL_STATE_STRING) + tQuoted(it.second) + toString(FOR_PIN_NUMBER_STRING) + toString(it.first) + toString(ELIPSES_STRING)};
        try {
            if (confirmValidStates(it)) {
                auto found{this->m_gpioPins.find(it.first)};
                if (found != this->m_gpioPins.end()) {
                    if (found->second->ioType() == IOType::DIGITAL_OUTPUT) {
                        std::pair<IOStatus, bool>  result{this->digitalWrite(found->second, parseToDigitalState(it.second))};
                        if (result.first == IOStatus::OPERATION_FAILURE) {
                            logString.append(GENERIC_FAILED_STRING);
                        } else {
                            logString.append(GENERIC_SUCCESS_STRING);
                        }
                    } else if (found->second->ioType() == IOType::ANALOG_OUTPUT) {
                        std::pair<IOStatus, int> result{this->analogWriteRaw(found->second, parseToAnalogStateRaw(it.second))};
                        if (result.first == IOStatus::OPERATION_FAILURE) {
                            logString.append(GENERIC_FAILED_STRING);
                        } else {
                            logString.append(GENERIC_SUCCESS_STRING);
                        }
                    }
                }
                std::cout << logString;
                std::cout << " (" << this->identifier() << ", " << this->serialPortName() << ")" << std::endl;
            } else {
                //TODO:Failure message
            }
        } catch (std::exception &e) {
            logString.append(GENERIC_FAILED_STRING);
            std::cout << logString;
            std::cout << " (" << this->identifier() << ", " << this->serialPortName() << ")" << std::endl;
            (void)e;
        }
    }
}

std::string Arduino::serialPortName() const
{
    return this->serialPort()->portName();
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

int Arduino::voltageToAnalog(double state)
{
    return (state / Arduino::ANALOG_TO_VOLTAGE_SCALE_FACTOR);
}

double Arduino::analogToVoltage(int state)
{
    return state * Arduino::ANALOG_TO_VOLTAGE_SCALE_FACTOR;
}

unsigned int Arduino::SERIAL_PORT_TRY_COUNT_HIGH_LIMIT()
{
    return Arduino::s_SERIAL_PORT_TRY_COUNT_HIGH_LIMIT;
}

int Arduino::numberOfSerialPorts() const
{
    return Arduino::s_serialPorts.size();
}

bool Arduino::isBluetooth(int serialPortIndex)
{
    return (Arduino::serialPortAtIndex(serialPortIndex)->portName().find(BLUETOOTH_SERIAL_IDENTIFIER) != std::string::npos);
}

bool Arduino::isBluetooth(const std::string &name)
{
    return (name.find(BLUETOOTH_SERIAL_IDENTIFIER) != std::string::npos);
}

bool Arduino::isBluetooth(std::shared_ptr<SerialPort> serialPort)
{
    if (!serialPort) {
        return false;
    }
    return Arduino::isBluetooth(serialPort->portName());
}

int Arduino::addSerialPort(std::shared_ptr<SerialPort> serialPort)
{
    Arduino::s_serialPorts.emplace_back(serialPort);
    return (Arduino::s_serialPorts.size()-1);
}

std::shared_ptr<SerialPort> Arduino::serialPortAtIndex(unsigned int serialPortIndex)
{
    if (serialPortIndex > Arduino::s_serialPorts.size()-1) {
        throw std::runtime_error(SERIAL_PORT_INVALID_INDEX_STRING + std::to_string(serialPortIndex));
    } else {
        return Arduino::s_serialPorts.at(serialPortIndex).serialPort();
    }
}

std::shared_ptr<std::mutex> Arduino::ioMutexAtIndex(unsigned int ioMutexIndex)
{
    if (ioMutexIndex > Arduino::s_serialPorts.size()-1) {
        throw std::runtime_error(IO_MUTEX_INVALID_INDEX_STRING + std::to_string(ioMutexIndex));
    } else {
        return (Arduino::s_serialPorts.at(ioMutexIndex).ioMutex());
    }
}


std::vector<std::string> Arduino::genericIOTask(const std::string &stringToSend, const std::string &header, int serialPortIndex, double delay)
{
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    return Arduino::genericIOTask(stringToSend, header, serialPort, delay);
}

std::vector<std::string> Arduino::genericIOTask(const std::string &stringToSend, const std::string &header, std::shared_ptr<SerialPort> serialPort, double delay)
{
    using namespace GeneralUtilities;
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    long long int tempTimeout{serialPort->timeout()};
    serialPort->setTimeout(Arduino::SERIAL_REPORT_REQUEST_TIME_LIMIT);
    serialPort->writeString(stringToSend);
#if defined(__USE_SERIAL_WRITE_DELAY__)
    delayMilliseconds(isBluetooth(serialPort->portName()) ? delay*Arduino::bluetoothSendDelayMultiplier : delay);
#else
    if (isBluetooth(serialPort->portName())) {
        delayMilliseconds(delay);
    }
#endif
    std::unique_ptr<std::string> returnString{std::make_unique<std::string>("")};
    std::future<std::string> returnHandle{serialPort->asyncReadStringUntil("}")};
    do {
        if (returnHandle.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            *returnString = returnHandle.get();
            break;
        }
    } while (true);
    serialPort->setTimeout(tempTimeout);
    if (startsWith(*returnString, header) && endsWith(*returnString, '}')) {
        *returnString = returnString->substr(static_cast<std::string>(header).length() + 1);
        *returnString = returnString->substr(0, returnString->length()-1);
    } else {
        return std::vector<std::string>{};
    }
    return parseToVector(*returnString, ':');
}

std::vector<std::string> Arduino::genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, int serialPortIndex, double delay)
{
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    return Arduino::genericIOReportTask(stringToSend, header, endHeader, serialPort, delay);
}

std::vector<std::string> Arduino::genericIOReportTask(const std::string &stringToSend, const std::string &header, const std::string &endHeader, std::shared_ptr<SerialPort> serialPort, double delay)
{
    using namespace GeneralUtilities;
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    serialPort->writeString(stringToSend);
#if defined(__USE_SERIAL_WRITE_DELAY__)
    delayMilliseconds(isBluetooth(serialPort->portName()) ? delay*Arduino::bluetoothSendDelayMultiplier : delay);
#else
    if (isBluetooth(serialPort->portName())) {
        delayMilliseconds(delay);
    }
#endif
    std::unique_ptr<std::string> returnString{std::make_unique<std::string>("")};
    std::future<std::string> returnHandle{serialPort->asyncReadStringUntil(endHeader)};
    do {
        if (returnHandle.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            *returnString = returnHandle.get();
            break;
        }
    } while (true);
    if (startsWith(*returnString, header) && endsWith(*returnString, endHeader)) {
        *returnString = returnString->substr(static_cast<std::string>(header).length() + 1);
        *returnString = returnString->substr(0, returnString->length()-1);
    } else {
        return std::vector<std::string>{};
    }
    return parseToVector(*returnString, ';');
}

std::pair<IOStatus, std::string> Arduino::getArduinoType(std::shared_ptr<SerialPort> serialPort)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::ARDUINO_TYPE_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ARDUINO_TYPE_HEADER, serialPort)};
        if (states.size() != Arduino::ARDUINO_TYPE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        if (states.at(ArduinoTypeEnum::OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, states.at(ArduinoTypeEnum::RETURN_STATE));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, "");
}

std::pair<IOStatus, std::string> Arduino::getFirmwareVersion(std::shared_ptr<SerialPort> serialPort)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::FIRMWARE_VERSION_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::FIRMWARE_VERSION_HEADER, serialPort)};
        if (states.size() != Arduino::ARDUINO_TYPE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        if (states.at(ArduinoTypeEnum::OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, "");
            } else {
                continue;
            }
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, states.at(ArduinoTypeEnum::RETURN_STATE));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, "");
}

std::pair<IOStatus, bool> Arduino::getCanCapability(std::shared_ptr<SerialPort> serialPort)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_BUS_ENABLED_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CAN_BUS_ENABLED_HEADER, serialPort)};
        if (states.size() != Arduino::CAN_BUS_ENABLED_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            }
        }
        if (states.at(CanEnabledStatus::CAN_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, (std::stoi(states.at(CanEnabledStatus::CAN_RETURN_STATE)) == 1));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, int> Arduino::getAnalogToDigitalThreshold(std::shared_ptr<SerialPort> serialPort)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CURRENT_A_TO_D_THRESHOLD_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CURRENT_A_TO_D_THRESHOLD_HEADER, serialPort)};
        if (states.size() != Arduino::A_TO_D_THRESHOLD_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(ADThresholdReq::AD_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(ADThresholdReq::AD_RETURN_STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

void Arduino::writeRawString(const std::string &str, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    Arduino::serialPortAtIndex(serialPortIndex)->writeString(str);
}

std::string Arduino::readRawString(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    return Arduino::serialPortAtIndex(serialPortIndex)->readString();
}


void Arduino::flushRX(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    Arduino::serialPortAtIndex(serialPortIndex)->flushRX();
}

void Arduino::flushTX(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    Arduino::serialPortAtIndex(serialPortIndex)->flushTX();
}

void Arduino::flushRXTX(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    if (!Arduino::serialPortAtIndex(serialPortIndex)->isOpen()) {
        Arduino::serialPortAtIndex(serialPortIndex)->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    Arduino::serialPortAtIndex(serialPortIndex)->flushRXTX();
}

void Arduino::flushTXRX(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    Arduino::serialPortAtIndex(serialPortIndex)->flushTXRX();
}

std::pair<IOStatus, bool> Arduino::canAutoUpdate(bool state, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_LIVE_UPDATE_HEADER) + ':' + std::to_string(state) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CAN_LIVE_UPDATE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::CAN_AUTO_UPDATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (std::to_string(state) != states.at(CanEnabledStatus::CAN_RETURN_STATE)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(CanEnabledStatus::CAN_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(CanEnabledStatus::CAN_OPERATION_RESULT)) == 1);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, bool> Arduino::initializeCanBus(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_INIT_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CAN_INIT_HEADER, serialPortIndex, Arduino::BLUETOOTH_SERIAL_SEND_DELAY*3)};
        if (states.size() == 0) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(0) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        } else {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, true);
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, CanMessage> Arduino::canRead(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_READ_HEADER) + '}'};
    CanMessage emptyMessage{0, 0, 0, CanDataPacket()};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CAN_READ_HEADER, serialPortIndex)};
        if ((states.size() == Arduino::CAN_READ_RETURN_SIZE) && (states.size() != Arduino::CAN_READ_BLANK_RETURN_SIZE)){
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        if (states.size() == Arduino::CAN_READ_BLANK_RETURN_SIZE) {
            if (states.at(0) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                } else {
                    continue;
                }
            } else {
                return std::make_pair(IOStatus::OPERATION_SUCCESS, emptyMessage);
            }
        }
        if (states.at(CanIOStatus::CAN_IO_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        std::string message{states.at(CanIOStatus::MESSAGE_ID)};
        for (unsigned int i = CanIOStatus::BYTE_0; i < CanIOStatus::CAN_IO_OPERATION_RESULT; i++) {
            message += ":" + states.at(i);
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, CanMessage::parseCanMessage(message));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
}

std::pair<IOStatus, CanMessage> Arduino::canWrite(const CanMessage &message, int serialPortIndex)
{
    using namespace GeneralUtilities;
    CanMessage emptyMessage{0, 0, 0, CanDataPacket()};
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_WRITE_HEADER) + ":" + message.toString() + "}" };
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CAN_WRITE_HEADER, serialPortIndex)};
        if ((states.size() == Arduino::CAN_READ_RETURN_SIZE) && (states.size() != Arduino::CAN_READ_BLANK_RETURN_SIZE)){
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        if (states.size() != Arduino::CAN_WRITE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        if (states.at(CanIOStatus::CAN_IO_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        std::string returnMessage{states.at(CanIOStatus::MESSAGE_ID) + ":"};
        for (unsigned int i = CanIOStatus::BYTE_0; i < CanIOStatus::CAN_IO_OPERATION_RESULT; i++) {
            returnMessage += ":" + states.at(i);
        }
        return std::make_pair(IOStatus::OPERATION_SUCCESS, CanMessage::parseCanMessage(returnMessage));
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
}

IOReport Arduino::ioReportRequest(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::IO_REPORT_HEADER) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> allStates{Arduino::genericIOReportTask(stringToSend, Arduino::IO_REPORT_HEADER, static_cast<std::string>(Arduino::IO_REPORT_END_HEADER) + '}', serialPortIndex, 100)};
        IOReport ioReport;
        for (auto &it : allStates) {
            it = stripAllFromString(it, '}');
            it = stripAllFromString(it, '{');
            std::vector<std::string> states{parseToVector(it, ':')};
            std::string endCheck{stripAllFromString(Arduino::IO_REPORT_END_HEADER, '{')};
            endCheck = stripAllFromString(endCheck, '}');
            if (it.find(endCheck) != std::string::npos) {
                continue;
            }
            if ((states.size() != Arduino::IO_REPORT_RETURN_SIZE) && (states.size() != 0)) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
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
                ioReport.addDigitalInputResult(std::make_pair(std::stoi(states.at(IOReportEnum::IO_PIN_NUMBER)), std::stoi(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::DIGITAL_OUTPUT) {
                ioReport.addDigitalOutputResult(std::make_pair(std::stoi(states.at(IOReportEnum::IO_PIN_NUMBER)), std::stoi(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::ANALOG_INPUT) {
                ioReport.addAnalogInputResult(std::make_pair(std::stoi(states.at(IOReportEnum::IO_PIN_NUMBER)), std::stoi(states.at(IOReportEnum::IO_STATE))));
            } else if (ioType == IOType::ANALOG_OUTPUT) {
                ioReport.addAnalogOutputResult(std::make_pair(std::stoi(states.at(IOReportEnum::IO_PIN_NUMBER)), std::stoi(states.at(IOReportEnum::IO_STATE))));
            }
        }
        return ioReport;
    }
    return IOReport{};
}

SerialReport Arduino::serialReportRequest(int serialPortIndex, const std::string &delimiter)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
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
            *readString = serialPort->readString();
            if ((*readString == "") || (isWhitespace(*readString))) {
                continue;
            } else {
                eventTimer->restart();
                returnString += *readString;
            }
        } while ((eventTimer->totalMilliseconds() <= Arduino::SERIAL_REPORT_REQUEST_TIME_LIMIT) &&
                 (overallTimeout->totalMilliseconds() <= Arduino::SERIAL_REPORT_OVERALL_TIME_LIMIT) &&
                 (!endsWith(returnString, delimiter)));
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

CanReport Arduino::canReportRequest(int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        CanReport canReport;
        std::pair<IOStatus, CanMessage> result{Arduino::canListen(serialPortIndex)};
        if (result.first == IOStatus::OPERATION_FAILURE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                throw std::runtime_error(CAN_REPORT_INVALID_DATA_STRING);
            } else {
                continue;
            }
        } else {
            canReport.addCanMessageResult(result.second);
        }
        return canReport;
    }
    return CanReport{};
}

std::pair<IOStatus, CanMessage> Arduino::canListen(int serialPortIndex, double delay)
{
    using namespace GeneralUtilities;
    std::lock_guard<std::mutex> ioLock{*Arduino::ioMutexAtIndex(serialPortIndex)};
    std::shared_ptr<SerialPort> serialPort{Arduino::serialPortAtIndex(serialPortIndex)};
    if (!serialPort->isOpen()) {
        serialPort->openPort();
        delayMilliseconds(Arduino::BOOTLOADER_BOOT_TIME);
    }
    std::string stringToSend{static_cast<std::string>(Arduino::CAN_READ_HEADER) + '}'};
    CanMessage emptyMessage{0, 0, 0, CanDataPacket()};
    serialPort->writeString(stringToSend);

#if defined(__USE_SERIAL_WRITE_DELAY__)
    delayMilliseconds(isBluetooth(serialPort->portName()) ? delay*Arduino::bluetoothSendDelayMultiplier : delay);
#else
    if (isBluetooth(serialPort)) {
        delayMilliseconds(delay);
    }
#endif
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::unique_ptr<std::string> returnString{std::make_unique<std::string>("")};
        *returnString = serialPort->readStringUntil('}');
        bool canRead{false};
        if ((returnString->find(CAN_EMPTY_READ_SUCCESS_STRING) != std::string::npos) && (returnString->length() > static_cast<std::string>(CAN_EMPTY_READ_SUCCESS_STRING).length() + 10)) {
            *returnString = returnString->substr(static_cast<std::string>(CAN_EMPTY_READ_SUCCESS_STRING).length());
        }
        if (startsWith(*returnString, Arduino::CAN_READ_HEADER) && endsWith(*returnString, '}')) {
            *returnString = returnString->substr(static_cast<std::string>(Arduino::CAN_READ_HEADER).length() + 1);
            *returnString = returnString->substr(0, returnString->length()-1);
            canRead = true;
        } else if (startsWith(*returnString, Arduino::CAN_WRITE_HEADER) && endsWith(*returnString, '}')) {
            *returnString = returnString->substr(static_cast<std::string>(Arduino::CAN_WRITE_HEADER).length() + 1);
            *returnString = returnString->substr(0, returnString->find('}')+1);
        } else {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
            } else {
                continue;
            }
        }
        if (canRead) {
            std::vector<std::string> states{parseToVector(*returnString, ':')};
            if ((states.size() != Arduino::CAN_READ_RETURN_SIZE) && (states.size() != Arduino::CAN_READ_BLANK_RETURN_SIZE)){
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                } else {
                    continue;
                }
            }
            if (states.size() == Arduino::CAN_READ_BLANK_RETURN_SIZE) {
                if (states.at(0) == OPERATION_FAILURE_STRING) {
                    if (i+1 == Arduino::IO_TRY_COUNT) {
                        return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                    } else {
                        continue;
                    }
                } else {
                    return std::make_pair(IOStatus::OPERATION_SUCCESS, emptyMessage);
                }
            }
            if (states.at(CanIOStatus::CAN_IO_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                } else {
                    continue;
                }
            }
            std::string message{states.at(CanIOStatus::MESSAGE_ID)};
            for (unsigned int i = CanIOStatus::BYTE_0; i < CanIOStatus::CAN_IO_OPERATION_RESULT; i++) {
                message += ":" + states.at(i);
            }
            return std::make_pair(IOStatus::OPERATION_SUCCESS, CanMessage::parseCanMessage(message));
        } else {
            std::vector<std::string> states{parseToVector(*returnString, ':')};
            if (states.size() != Arduino::CAN_WRITE_RETURN_SIZE) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                } else {
                    continue;
                }
            }
            if (states.at(CanIOStatus::CAN_IO_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
                } else {
                    continue;
                }
            }
            std::string message{states.at(CanIOStatus::MESSAGE_ID)};
            for (unsigned int i = CanIOStatus::BYTE_0; i < CanIOStatus::CAN_IO_OPERATION_RESULT; i++) {
                message += ":" + states.at(i);
            }
            return std::make_pair(IOStatus::OPERATION_SUCCESS, CanMessage::parseCanMessage(message));
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, emptyMessage);
}

std::pair<IOStatus, uint32_t> Arduino::addCanMask(CanMaskType canMaskType, const std::string &mask, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{""};
    if (canMaskType == CanMaskType::POSITIVE) {
        stringToSend = static_cast<std::string>(Arduino::ADD_POSITIVE_CAN_MASK_HEADER) + ':' + mask + '}';
    } else if (canMaskType == CanMaskType::NEGATIVE) {
        stringToSend = static_cast<std::string>(Arduino::ADD_NEGATIVE_CAN_MASK_HEADER) + ':' + mask + '}';
    } else {
        return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
    }
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        if (canMaskType == CanMaskType::POSITIVE) {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ADD_POSITIVE_CAN_MASK_HEADER, serialPortIndex)};
            if (states.size() != Arduino::ADD_CAN_MASK_RETURN_SIZE) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (mask != states.at(CanMask::CAN_MASK_RETURN_STATE)) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (states.at(CanMask::CAN_MASK_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            try {
                return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(CanMask::CAN_MASK_RETURN_STATE)));
            } catch (std::exception &e) {
                (void)e;
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
        } else if (canMaskType == CanMaskType::NEGATIVE) {
            std::string stringToSend{static_cast<std::string>(Arduino::ADD_NEGATIVE_CAN_MASK_HEADER) + ':' + mask + '}'};
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ADD_NEGATIVE_CAN_MASK_HEADER, serialPortIndex)};
            if (states.size() != Arduino::ADD_CAN_MASK_RETURN_SIZE) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (mask != states.at(CanMask::CAN_MASK_RETURN_STATE)) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (states.at(CanMask::CAN_MASK_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            try {
                return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(CanMask::CAN_MASK_RETURN_STATE)));
            } catch (std::exception &e) {
                (void)e;
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
        } else {
            return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}


std::pair<IOStatus, uint32_t> Arduino::removeCanMask(CanMaskType canMaskType, const std::string &mask, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{""};
    if (canMaskType == CanMaskType::POSITIVE) {
        stringToSend = static_cast<std::string>(Arduino::REMOVE_POSITIVE_CAN_MASK_HEADER) + ':' + mask + '}';
    } else if (canMaskType == CanMaskType::NEGATIVE) {
        stringToSend = static_cast<std::string>(Arduino::REMOVE_NEGATIVE_CAN_MASK_HEADER) + ':' + mask + '}';
    } else {
        return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
    }
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        if (canMaskType == CanMaskType::POSITIVE) {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::REMOVE_POSITIVE_CAN_MASK_HEADER, serialPortIndex)};
            if (states.size() != Arduino::REMOVE_CAN_MASK_RETURN_SIZE) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (mask != states.at(CanMask::CAN_MASK_RETURN_STATE)) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (states.at(CanMask::CAN_MASK_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            try {
                return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(CanMask::CAN_MASK_RETURN_STATE)));
            } catch (std::exception &e) {
                (void)e;
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
        } else if (canMaskType == CanMaskType::NEGATIVE) {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::REMOVE_NEGATIVE_CAN_MASK_HEADER, serialPortIndex)};
            if (states.size() != Arduino::REMOVE_CAN_MASK_RETURN_SIZE) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (mask != states.at(CanMask::CAN_MASK_RETURN_STATE)) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            if (states.at(CanMask::CAN_MASK_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
            try {
                return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(CanMask::CAN_MASK_RETURN_STATE)));
            } catch (std::exception &e) {
                (void)e;
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
                } else {
                    continue;
                }
            }
        } else {
            return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, bool> Arduino::removeAllCanMasks(CanMaskType canMaskType, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{""};
    if (canMaskType == CanMaskType::POSITIVE) {
        stringToSend = static_cast<std::string>(Arduino::REMOVE_ALL_POSITIVE_CAN_MASKS_HEADER) + '}';
    } else if (canMaskType == CanMaskType::NEGATIVE) {
        stringToSend = static_cast<std::string>(Arduino::REMOVE_ALL_NEGATIVE_CAN_MASKS_HEADER) + '}';
    } else {
        stringToSend = static_cast<std::string>(Arduino::REMOVE_ALL_CAN_MASKS_HEADER) + '}';
    }

    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        if (canMaskType == CanMaskType::POSITIVE) {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::REMOVE_ALL_POSITIVE_CAN_MASKS_HEADER, serialPortIndex)};
            if (states.size() != 1) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            if (states.at(0) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            return std::make_pair(IOStatus::OPERATION_SUCCESS, true);
        } else if (canMaskType == CanMaskType::NEGATIVE) {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::REMOVE_ALL_NEGATIVE_CAN_MASKS_HEADER, serialPortIndex)};
            if (states.size() != 1) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            if (states.at(0) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            return std::make_pair(IOStatus::OPERATION_SUCCESS, true);
        } else {
            std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::REMOVE_ALL_CAN_MASKS_HEADER, serialPortIndex)};
            if (states.size() != 1) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            if (states.at(0) == OPERATION_FAILURE_STRING) {
                if (i+1 == Arduino::IO_TRY_COUNT) {
                    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
                } else {
                    continue;
                }
            }
            return std::make_pair(IOStatus::OPERATION_SUCCESS, true);
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, int> Arduino::changeAnalogToDigitalThreshold(int threshold, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::CHANGE_A_TO_D_THRESHOLD_HEADER) + ':' + std::to_string(threshold) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::CHANGE_A_TO_D_THRESHOLD_HEADER, serialPortIndex)};
        if (states.size() != Arduino::A_TO_D_THRESHOLD_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(threshold) != states.at(ADThresholdReq::AD_RETURN_STATE)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(ADThresholdReq::AD_OPERATION_RESULT) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(ADThresholdReq::AD_RETURN_STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}


std::pair<IOStatus, IOType> Arduino::pinMode(int pinNumber, IOType ioType, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::PIN_TYPE_CHANGE_HEADER) + ':' + std::to_string(pinNumber) + ':' + parseIOType(ioType) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::PIN_TYPE_CHANGE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, parseIOTypeFromString(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
}


std::pair<IOStatus, IOType> Arduino::currentPinMode(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::PIN_TYPE_HEADER) + ':' + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::PIN_TYPE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::PIN_TYPE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, parseIOTypeFromString(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, IOType::UNSPECIFIED);
}

std::pair<IOStatus, bool> Arduino::digitalRead(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::DIGITAL_READ_HEADER) + ":" + std::to_string(pinNumber) + '}' };
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::DIGITAL_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)) == 1);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, bool> Arduino::digitalWrite(int pinNumber, bool state, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::DIGITAL_WRITE_HEADER) + ":" + std::to_string(pinNumber) + ":" + std::to_string(state) + '}' };
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::DIGITAL_WRITE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)) == 1);
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, false);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, false);
}

std::pair<IOStatus, bool> Arduino::softDigitalRead(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::SOFT_DIGITAL_READ_HEADER) + ":" + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::SOFT_DIGITAL_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::analogRead(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ANALOG_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, Arduino::analogToVoltage(std::stoi(states.at(IOState::STATE))));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::analogReadRaw(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ANALOG_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::softAnalogRead(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::SOFT_ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, SOFT_ANALOG_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, Arduino::analogToVoltage(std::stoi(states.at(IOState::STATE))));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::softAnalogReadRaw(int pinNumber, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::SOFT_ANALOG_READ_HEADER) + ":" + std::to_string(pinNumber) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::SOFT_ANALOG_READ_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, double> Arduino::analogWrite(int pinNumber, double state, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::ANALOG_WRITE_HEADER) + ":" + std::to_string(Arduino::voltageToAnalog(pinNumber)) + ":" + std::to_string(state) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ANALOG_WRITE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stod(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0.00);
}

std::pair<IOStatus, int> Arduino::analogWriteRaw(int pinNumber, int state, int serialPortIndex)
{
    using namespace GeneralUtilities;
    std::string stringToSend{static_cast<std::string>(Arduino::ANALOG_WRITE_HEADER) + ":" + std::to_string(pinNumber) + ":" + std::to_string(state) + '}'};
    for (int i = 0; i < Arduino::IO_TRY_COUNT; i++) {
        std::vector<std::string> states{Arduino::genericIOTask(stringToSend, Arduino::ANALOG_WRITE_HEADER, serialPortIndex)};
        if (states.size() != Arduino::IO_STATE_RETURN_SIZE) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (std::to_string(pinNumber) != states.at(IOState::PIN_NUMBER)) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        if (states.at(IOState::RETURN_CODE) == OPERATION_FAILURE_STRING) {
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
        try {
            return std::make_pair(IOStatus::OPERATION_SUCCESS, std::stoi(states.at(IOState::STATE)));
        } catch (std::exception &e) {
            (void)e;
            if (i+1 == Arduino::IO_TRY_COUNT) {
                return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
            } else {
                continue;
            }
        }
    }
    return std::make_pair(IOStatus::OPERATION_FAILURE, 0);
}

std::pair<IOStatus, std::string> Arduino::getArduinoType(int serialPortIndex)
{
    return Arduino::getArduinoType(Arduino::serialPortAtIndex(serialPortIndex));
}

bool Arduino::parseToDigitalState(const std::string &state) const
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

double Arduino::parseToAnalogState(const std::string &state) const
{

    try {
        double temp{std::stod(state.c_str())};
        if (temp > Arduino::VOLTAGE_MAX) {
            temp = Arduino::VOLTAGE_MAX;
        }
        return temp;
    } catch (std::exception &e) {
        throw std::runtime_error(INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_STRING + state);
    }
}

int Arduino::parseToAnalogStateRaw(const std::string &state) const
{
    try {
        int temp{std::stoi(state.c_str())};
        if (temp > Arduino::ANALOG_MAX) {
            temp = Arduino::ANALOG_MAX;
        }
        return temp;
    } catch (std::exception &e) {
        throw std::runtime_error(INVALID_STATE_TO_PARSE_TO_ANALOG_STATE_RAW_STRING + state);
    }
}

bool Arduino::isValidDigitalStateIdentifier(const std::string &state) const
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

bool Arduino::isValidAnalogStateIdentifier(const std::string &state) const
{
    for (auto &it : VALID_ANALOG_STATE_IDENTIFIERS) {
        if (state == std::string(1, it)) {
            return true;
        }
    }
    return false;
}

bool Arduino::isValidAnalogRawStateIdentifier(const std::string &state) const
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

std::string Arduino::parseIOType(IOType ioType)
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

IOType Arduino::parseIOTypeFromString(const std::string &ioType)
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

bool Arduino::setPinAlias(int pinNumber, const std::string &newAlias)
{
    if (this->m_gpioPinIterationAliasMap.find(pinNumber) == this->m_gpioPinIterationAliasMap.end()) {
        return false;
    } else {
        this->m_gpioPinIterationAliasMap.find(pinNumber)->second = newAlias;
        return true;
    }
}

bool Arduino::setPinAlias(std::shared_ptr<GPIO> gpio, const std::string &newAlias)
{
    if (!gpio) {
        return false;
    }
    if (this->m_gpioPinIterationAliasMap.find(gpio->pinNumber()) == this->m_gpioPinIterationAliasMap.end()) {
        return false;
    } else {
        this->m_gpioPinIterationAliasMap.find(gpio->pinNumber())->second = newAlias;
        return true;
    }
}


bool Arduino::setPinAlias(const GPIO &gpio, const std::string &newAlias)
{
    if (this->m_gpioPinIterationAliasMap.find(gpio.pinNumber()) == this->m_gpioPinIterationAliasMap.end()) {
        return false;
    } else {
        this->m_gpioPinIterationAliasMap.find(gpio.pinNumber())->second = newAlias;
        return true;
    }
}


bool Arduino::isBluetooth() const
{
    return Arduino::isBluetooth(this->m_serialPortIndex);
}

void Arduino::initializeIO()
{
    using namespace GeneralUtilities;
    this->m_gpioPins.clear();
    this->m_gpioPinIterationAliasMap.clear();
    this->m_gpioPinsAlias.clear();
    for (auto &it : this->m_availablePins) {
        this->m_gpioPins.emplace(it, std::make_shared<GPIO>(it, IOType::UNSPECIFIED));
    }
    for (auto &it : this->m_gpioPins) {
        if (it.first > this->m_numberOfDigitalPins) {
            if (it.first == CAN_BUS_PIN) {
                m_canPinAlias = std::to_string(it.first);
            }
            this->m_gpioPinsAlias.emplace(analogPinFromNumber(it.first), it.second);
            this->m_gpioPinIterationAliasMap.emplace(it.first, analogPinFromNumber(it.first));
        } else {
            this->m_gpioPinsAlias.emplace(std::to_string(it.first), it.second);
            this->m_gpioPinIterationAliasMap.emplace(it.first, std::to_string(it.first));
        }
    }
    if (this->m_canCapability.second) {
        this->eraseCanPin();
    }

    /*
    ConfigurationFileReader cfr;
    for (auto &it : cfr.nanoAliasesMap()) {
        if (this->m_gpioPinsAlias.find(std::to_string(it.first)) != this->m_gpioPinsAlias.end()) {
            auto iter = this->m_gpioPinsAlias.find(std::to_string(it.first));
            std::swap(this->m_gpioPinsAlias[std::to_string(it.first)], iter->second);
            this->m_gpioPinsAlias.erase(iter);
            this->m_gpioPinIterationAliasMap.find(it.first)->second = it.second;
        }
    }
    for (auto &it : cfr.nanoIOTypesMap()) {
        auto found{this->m_gpioPins.find(it.first)};
        if (found != this->m_gpioPins.end()) {
            if (found->second->ioType() == IOType::UNSPECIFIED) {
                found->second->setIOType(parseIOTypeFromString(it.second));
            }
        }
    }
    assignIOTypes();
    for (auto &it : cfr.nanoInitialStatesMap()) {
        try {
            if (confirmValidStates(it)) {
                auto found{this->m_gpioPins.find(it.first)};
                if (found != this->m_gpioPins.end()) {
                    if (found->second->ioType() == IOType::DIGITAL_OUTPUT) {
                        std::pair<IOStatus, bool>  result{this->digitalWrite(found->second, parseToDigitalState(it.second))};
                        (void)result;
                    } else if (found->second->ioType() == IOType::ANALOG_OUTPUT) {
                        std::pair<IOStatus, int> result{this->analogWriteRaw(found->second, parseToAnalogStateRaw(it.second))};
                        (void)result;
                    }
                }
            } else {
                //TODO:Failure message
            }
        } catch (std::exception &e) {
            (void)e;
        }
    }
    if (cfr.bluetoothMultiplier() != -1) {
        Arduino::bluetoothSendDelayMultiplier = cfr.bluetoothMultiplier();
    }
    */
}



void Arduino::writeRawString(const std::string &str)
{
    return Arduino::writeRawString(str, this->m_serialPortIndex);
}

std::string Arduino::readRawString()
{
    return Arduino::readRawString(this->m_serialPortIndex);
}

void Arduino::flushRX()
{
    return Arduino::flushRX(this->m_serialPortIndex);
}

void Arduino::flushTX()
{
    return Arduino::flushTX(this->m_serialPortIndex);
}

void Arduino::flushRXTX()
{
    return Arduino::flushRXTX(this->m_serialPortIndex);
}

void Arduino::flushTXRX()
{
    return Arduino::flushTXRX(this->m_serialPortIndex);
}

void Arduino::setCanCapability(const std::pair<bool, bool> &capability)
{
    this->m_canCapability = capability;
    if ((this->m_canCapability.first) && (this->m_canCapability.second)) {
        eraseCanPin();
    }
}

void Arduino::setFirmwareVersion(const std::string &firmwareVersion)
{
    this->m_firmwareVersion = firmwareVersion;
}

std::string Arduino::firmwareVersion()
{

    if (this->m_firmwareVersion == "") {
        std::pair<IOStatus, std::string> firmware{Arduino::getFirmwareVersion(Arduino::serialPortAtIndex(this->m_serialPortIndex))};
        if (firmware.first == IOStatus::OPERATION_FAILURE) {
            this->m_firmwareVersion = FIRMWARE_VERSION_UNKNOWN_STRING;
            return this->m_firmwareVersion;
        } else {
            return (this->m_firmwareVersion = firmware.second);
        }
    } else {
        return this->m_firmwareVersion;
    }
}

void Arduino::setAnalogToDigitalThreshold(int state)
{
    this->m_analogToDigitalThreshold = state;
}

int Arduino::analogToDigitalThreshold()
{

    if (this->m_analogToDigitalThreshold == -1) {
        std::pair<IOStatus, int> threshold{Arduino::getAnalogToDigitalThreshold(Arduino::serialPortAtIndex(this->m_serialPortIndex))};
        if (threshold.first == IOStatus::OPERATION_FAILURE) {
            return this->m_analogToDigitalThreshold;
        } else {
            return (this->m_analogToDigitalThreshold = threshold.second);
        }
    } else {
        return this->m_analogToDigitalThreshold;
    }
}


void Arduino::eraseCanPin()
{
    this->m_availablePins.erase(this->m_availablePins.find(CAN_BUS_PIN));
    this->m_availablePwmPins.erase(this->m_availablePwmPins.find(CAN_BUS_PIN));
    this->m_gpioPins.erase(this->m_gpioPins.find(CAN_BUS_PIN));
    this->m_gpioPinsAlias.erase(this->m_gpioPinsAlias.find(std::to_string(CAN_BUS_PIN)));
    this->m_numberOfDigitalPins--;

}

std::pair<bool, bool> Arduino::canCapability()
{

    if (this->m_canCapability.first == false) {
        std::pair<IOStatus, bool> capability{Arduino::getCanCapability(Arduino::serialPortAtIndex(this->m_serialPortIndex))};
        if (capability.first == IOStatus::OPERATION_FAILURE) {
            return std::make_pair(false, this->m_canCapability.second);
        } else {
            this->m_canCapability = std::make_pair(true, capability.second);
            if (capability.second) {
                eraseCanPin();
            }
            return std::make_pair(true, this->m_canCapability.second);
        }
    } else {
        return std::make_pair(true, this->m_canCapability.second);
    }
}

std::shared_ptr<SerialPort> Arduino::serialPort() const
{
    return Arduino::serialPortAtIndex(this->m_serialPortIndex);
}

bool Arduino::confirmValidAlias(const std::pair<int, std::string> &alias) const
{
    (void)alias;
    return true;
}

bool Arduino::confirmValidIOType(const std::pair<int, std::string> &ioType) const
{
    IOType io{IOType::UNSPECIFIED};
    (void)io;
    try {
        io = parseIOTypeFromString(ioType.second);
        return true;
    } catch (std::exception &e) {
        return false;
    }
}

bool Arduino::confirmValidStates(const std::pair<int, std::string> &state) const
{
    (void)state;
    return true;
}

void Arduino::assignIOTypes()
{
    using namespace GeneralUtilities;
    for (auto &it : this->m_gpioPins) {
        if (it.second->ioType() == IOType::UNSPECIFIED) {
            it.second->setIOType(this->currentPinMode(it.second).second);
        } else {
            std::pair<IOStatus, IOType> result{this->pinMode(it.second, it.second->ioType())};
            (void)result;
        }
    }
}

std::shared_ptr<GPIO> Arduino::gpioPinByPinAlias(const std::string &pinAlias) const
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return (this->m_gpioPinsAlias.find(pinAlias)->second);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias);
    }
}

std::shared_ptr<GPIO> Arduino::gpioPinByPinNumber(int pinNumber) const
{
    if (this->m_gpioPins.find(pinNumber) != this->m_gpioPins.end()) {
        return (this->m_gpioPins.find(pinNumber)->second);
    } else {
        throw std::runtime_error(INVALID_PIN_NUMBER_STRING + pinNumber);
    }
}

std::string Arduino::gpioAliasByPinNumber(int number) const
{
    if (this->m_gpioPinIterationAliasMap.find(number) != this->m_gpioPinIterationAliasMap.end()) {
        return this->m_gpioPinIterationAliasMap.find(number)->second;
    } else {
        throw std::runtime_error("");
    }
}

std::pair<IOStatus, bool> Arduino::digitalRead(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return digitalRead(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_DIGITAL_READ_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::digitalWrite(const std::string &pinAlias, bool state)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return digitalWrite(this->m_gpioPinsAlias.find(pinAlias)->first, state);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_DIGITAL_WRITE_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogRead(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return analogRead(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_ANALOG_READ_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogReadRaw(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return analogReadRaw(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_ANALOG_READ_RAW_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogWrite(const std::string &pinAlias, double state)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return analogWrite(this->m_gpioPinsAlias.find(pinAlias)->first, state);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_ANALOG_WRITE_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogWriteRaw(const std::string &pinAlias, int state)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return analogWriteRaw(this->m_gpioPinsAlias.find(pinAlias)->first, state);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_ANALOG_WRITE_RAW_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::softDigitalRead(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return softDigitalRead(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_SOFT_DIGITAL_READ_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::softAnalogRead(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return softAnalogRead(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_SOFT_ANALOG_READ_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::softAnalogReadRaw(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return softAnalogReadRaw(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_SOFT_ANALOG_READ_RAW_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::pinMode(const std::string &pinAlias, IOType ioType)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return pinMode(this->m_gpioPinsAlias.find(pinAlias)->first, ioType);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_PIN_MODE_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::currentPinMode(const std::string &pinAlias)
{
    if (this->m_gpioPinsAlias.find(pinAlias) != this->m_gpioPinsAlias.end()) {
        return currentPinMode(this->m_gpioPinsAlias.find(pinAlias)->first);
    } else {
        throw std::runtime_error(INVALID_PIN_ALIAS_STRING + pinAlias + PASSED_TO_CURRENT_PIN_MODE_STRING_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::digitalRead(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::digitalRead(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_DIGITAL_READ_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::digitalWrite(const GPIO &gpio, bool state)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::digitalWrite(gpio.pinNumber(), state, this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_DIGITAL_WRITE_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogRead(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::analogRead(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_ANALOG_READ_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogReadRaw(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::analogReadRaw(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_ANALOG_READ_RAW_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogWrite(const GPIO &gpio, double state)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::analogWrite(gpio.pinNumber(), state, this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_ANALOG_WRITE_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogWriteRaw(const GPIO &gpio, int state)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::analogWriteRaw(gpio.pinNumber(), state, this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_ANALOG_WRITE_RAW_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::softDigitalRead(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::softDigitalRead(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_SOFT_DIGITAL_READ_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, double> Arduino::softAnalogRead(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::softAnalogRead(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_SOFT_ANALOG_READ_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, int> Arduino::softAnalogReadRaw(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::softAnalogReadRaw(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_SOFT_ANALOG_READ_RAW_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::pinMode(GPIO &gpio, IOType ioType)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        std::pair<IOStatus, IOType> result{Arduino::pinMode(gpio.pinNumber(), ioType, this->m_serialPortIndex)};
        if (result.first == IOStatus::OPERATION_SUCCESS) {
            gpio.setIOType(ioType);
        }
        return result;
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_PIN_MODE_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::currentPinMode(const GPIO &gpio)
{
    if (this->m_gpioPins.find(gpio.pinNumber()) != this->m_gpioPins.end()) {
        return Arduino::currentPinMode(gpio.pinNumber(), this->m_serialPortIndex);
    } else {
        throw std::runtime_error(INVALID_GPIO_PIN_STRING + std::to_string(gpio.pinNumber()) + PASSED_TO_CURRENT_PIN_MODE_GPIO_TAIL_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::digitalRead(std::shared_ptr<GPIO> gpioPtr)
{
    if (gpioPtr) {
        return digitalRead(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_DIGITAL_READ_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::digitalWrite(std::shared_ptr<GPIO> gpioPtr, bool state)
{
    if (gpioPtr) {
        return digitalWrite(*gpioPtr, state);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_DIGITAL_WRITE_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogRead(std::shared_ptr<GPIO> gpioPtr)
{
    if (gpioPtr) {
        return analogRead(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_ANALOG_READ_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogReadRaw(std::shared_ptr<GPIO> gpioPtr)
{
    if (gpioPtr) {
        return analogReadRaw(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_ANALOG_READ_RAW_STRING);
    }
}

std::pair<IOStatus, double> Arduino::analogWrite(std::shared_ptr<GPIO> gpioPtr, double state)
{
    if (gpioPtr) {
        return analogWrite(*gpioPtr, state);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_ANALOG_WRITE_STRING);
    }
}

std::pair<IOStatus, int> Arduino::analogWriteRaw(std::shared_ptr<GPIO> gpioPtr, int state)
{

    if (gpioPtr) {
        return analogWriteRaw(*gpioPtr, state);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_ANALOG_WRITE_RAW_STRING);
    }
}

std::pair<IOStatus, bool> Arduino::softDigitalRead(std::shared_ptr<GPIO> gpioPtr)
{

    if (gpioPtr) {
        return softDigitalRead(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_SOFT_DIGITAL_READ_STRING);
    }
}

std::pair<IOStatus, double> Arduino::softAnalogRead(std::shared_ptr<GPIO> gpioPtr)
{

    if (gpioPtr) {
        return softAnalogRead(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_SOFT_ANALOG_READ_STRING);
    }
}

std::pair<IOStatus, int> Arduino::softAnalogReadRaw(std::shared_ptr<GPIO> gpioPtr)
{
    if (gpioPtr) {
        return softAnalogReadRaw(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_SOFT_ANALOG_READ_RAW_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::pinMode(std::shared_ptr<GPIO> gpioPtr, IOType ioType)
{
    if (gpioPtr) {
        return pinMode(*gpioPtr, ioType);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_PIN_MODE_STRING);
    }
}

std::pair<IOStatus, IOType> Arduino::currentPinMode(std::shared_ptr<GPIO> gpioPtr)
{
    if (gpioPtr) {
        return currentPinMode(*gpioPtr);
    } else {
        throw std::runtime_error(NULL_GPIO_PTR_TO_CURRENT_PIN_MODE_STRING);
    }
}

bool Arduino::isValidAnalogPinIdentifier(const std::string &state) const
{
    for (auto &it : this->m_availableAnalogPins) {
        if (state == analogPinFromNumber(it)) {
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

int Arduino::parseAnalogPin(const std::string &pinAlias) const
{
    return Arduino::staticParseAnalogPin(this->m_arduinoType, pinAlias);
}

std::string Arduino::analogPinFromNumber(int pinNumber) const
{
    return Arduino::staticAnalogPinFromNumber(this->m_arduinoType, pinNumber);
}

int Arduino::staticParseAnalogPin(ArduinoType arduinoType, const std::string &pinAlias)
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
            throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + UNO_INVALID_ANALOG_STRING_TAIL_STRING);
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
           throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + NANO_INVALID_ANALOG_STRING_TAIL_STRING);
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
            throw std::logic_error(INVALID_ANALOG_PIN_BASE_STRING + pinAlias + MEGA_INVALID_ANALOG_STRING_TAIL_STRING);
        }
    } else {
        throw std::runtime_error("");
    }
}

std::string Arduino::staticAnalogPinFromNumber(ArduinoType arduinoType, int pinNumber)
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
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + UNO_INVALID_ANALOG_INT_TAIL_STRING);
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
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + NANO_INVALID_ANALOG_INT_TAIL_STRING);
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
            throw std::runtime_error(INVALID_ANALOG_PIN_BASE_STRING + std::to_string(pinNumber) + MEGA_INVALID_ANALOG_INT_TAIL_STRING);
        }
    } else {
        throw std::runtime_error("");
    }
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
