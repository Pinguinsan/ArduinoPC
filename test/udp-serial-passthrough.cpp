#include <iostream>
#include <fstream>
#include <memory>

#include <generalutilities.h>
#include <udpduplex.h>
#include <serialport.h>
#include <arduino.h>
#include <prettyprinter.h>

static std::unique_ptr<PrettyPrinter> prettyPrinter{std::make_unique<PrettyPrinter>()};
static const BackgroundColor COMMON_BACKGROUND_COLOR{BackgroundColor::BG_DEFAULT};
static const int COMMON_FONT_ATTRIBUTE{(FontAttribute::FA_BOLD | FontAttribute::FA_UNDERLINED)};
static const ForegroundColor TX_COLOR{ForegroundColor::FG_BLUE};
static const ForegroundColor RX_COLOR{ForegroundColor::FG_RED};
static const int RX_INDENTATION_AMOUNT{4};
static const int TX_INDENTATION_AMOUNT{4};

uint16_t getReturnPortName(const std::string &str);
std::string getReturnAddress(const std::string &str);
void setResponseAddress(UDPDatagram datagram, std::shared_ptr<UDPDuplex> udpDuplex);

int main()
{
    using namespace GeneralUtilities;
    std::string serialPortName{SerialPort::doUserSelectSerialPortName()};
    std::shared_ptr<SerialPort> serialPort{std::make_shared<SerialPort>(serialPortName,
                                                                        Arduino::FIRMWARE_BAUD_RATE,
                                                                        Arduino::FIRMWARE_DATA_BITS,
                                                                        Arduino::FIRMWARE_STOP_BITS,
                                                                        Arduino::FIRMWARE_PARITY)};

    std::shared_ptr<UDPDuplex> udpDuplex{std::make_shared<UDPDuplex>("127.0.0.1", 8887, 8888)};
    prettyPrinter->setFontAttributes(COMMON_FONT_ATTRIBUTE);
    serialPort->openPort();
    udpDuplex->openPort();
    do {
        std::string str{""};
        UDPDatagram datagram;
        if (udpDuplex->available()) {
            datagram = udpDuplex->readDatagram();
            str = datagram.message();
        }
        if (!str.empty()) {
            //setResponseAddress(datagram, udpDuplex);
            //std::cout << "Address = " << udpDuplex->clientHostName() << std::endl;
            //std::cout << "Port number = " << udpDuplex->clientPortNumber() << std::endl;
            prettyPrinter->setForegroundColor(RX_COLOR);
            std::cout << tWhitespace(RX_INDENTATION_AMOUNT);
            prettyPrinter->println("Rx << " + str);
            serialPort->writeString(str);
            delayMilliseconds(25);

            std::string returnString{serialPort->readString()};
            udpDuplex->writeString(returnString);
            prettyPrinter->setForegroundColor(TX_COLOR);
            std::cout << tWhitespace(TX_INDENTATION_AMOUNT);
            prettyPrinter->println("Tx >> " + returnString); 
        }
    } while (true);
    return 0;
}
        
                                                            
uint16_t getReturnPortName(const std::string &str)
{
    try {
        int portNumber{std::stoi(str)};
        if ((portNumber < 0) || (portNumber > std::numeric_limits<uint16_t>::max())) {
            throw std::runtime_error("UDP port number must be between 1 and " + std::to_string(std::numeric_limits<uint16_t>::max()) + "(" + str + ")" );
        }
        return static_cast<uint16_t>(portNumber);
    } catch (std::exception &e) {
        throw e;
    }
}

std::string getReturnAddress(const std::string &str)
{
    return str;
}

void setResponseAddress(UDPDatagram datagram, std::shared_ptr<UDPDuplex> udpDuplex)
{
    udpDuplex->setClientPortNumber(datagram.portNumber());
    udpDuplex->setClientHostName(datagram.hostName());
}
