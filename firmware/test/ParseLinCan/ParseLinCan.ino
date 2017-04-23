#include <Arduino.h>

#include "linmessage.h"
#include "canmessage.h"

Stream *globalInputStream{nullptr};
Stream *globalOutputStream{nullptr};

enum SerialCommandType {
	CommandTypeUnknown = -1,
	NoSerialCommand = 0,
	ReadLinMessage = 1,
	WriteLinMessage = 2,
	WriteCanMessage = 3
};

#define READ_MESSAGE_QUERY 'r'
#define WRITE_MESSAGE_QUERY 'w'
#define LIN_MESSAGE_IDENTIFIER 'l'
#define CAN_MESSAGE_IDENTIFIER 'c'
#define LINE_ENDING '\n'
#define DATA_DELIMITER ':'

#define FIND_READ_MESSAGE_TYPE_TIMEOUT 200
#define FIND_WRITE_MESSAGE_TYPE_TIMEOUT 200
#define PARSE_CAN_MESSAGE_TIMEOUT 250
#define PARSE_LIN_MESSAGE_TIMEOUT 250

#define CAN_MESSAGE_BUFFER 100
#define LIN_MESSAGE_BUFFER 100

CanMessage parseCanMessage();
LinMessage parseLinMessage();

SerialCommandType checkSerialCommands();
SerialCommandType findReadMessageType();
SerialCommandType findWriteMessageType();

template <typename WriteType, typename LineEndingType> inline constexpr void writeln(Stream *stream, const WriteType &toWrite, const LineEndingType &lineEnding)
{
    if (stream != nullptr) { stream->print(toWrite); stream->print(lineEnding); }
}


void setup()
{
	Serial.begin(115200L);
	globalInputStream = &Serial;
	globalOutputStream = &Serial;
}

void loop()
{
    SerialCommandType readCommand{checkSerialCommands()};
    if (readCommand != SerialCommandType::NoSerialCommand) {
        if (readCommand == SerialCommandType::ReadLinMessage) {
            writeln(globalOutputStream, F("Serial command received: ReadLinMessage"), LINE_ENDING);
            LinMessage linMessage{parseLinMessage()};
            if (linMessage == LinMessage{}) {
                writeln(globalOutputStream, F("Could not parse LIN Message"), LINE_ENDING);
            } else {
                char temp[LIN_MESSAGE_BUFFER];
                linMessage.toString(temp, LIN_MESSAGE_BUFFER);
                writeln(globalOutputStream, F("Parsed LIN message:"), LINE_ENDING);
                writeln(globalOutputStream, temp, LINE_ENDING); 
            }
        } else if (readCommand == SerialCommandType::WriteLinMessage) {
            writeln(globalOutputStream, F("Serial command received: WriteLinMessage"), LINE_ENDING);
            LinMessage linMessage{parseLinMessage()};
            if (linMessage == LinMessage{}) {
                writeln(globalOutputStream, F("Could not parse LIN Message"), LINE_ENDING);
            } else {
                char temp[LIN_MESSAGE_BUFFER];
                linMessage.toString(temp, LIN_MESSAGE_BUFFER);
                writeln(globalOutputStream, F("Parsed LIN message:"), LINE_ENDING);
                writeln(globalOutputStream, temp, LINE_ENDING); 
            }
        } else if (readCommand == SerialCommandType::WriteCanMessage) {
            writeln(globalOutputStream, F("Serial command received: WriteCanMessage"), LINE_ENDING);
            CanMessage canMessage{parseCanMessage()};
            if (canMessage == CanMessage{}) {
                writeln(globalOutputStream, F("Could not parse CAN Message"), LINE_ENDING);
            } else {
                char temp[CAN_MESSAGE_BUFFER];
                canMessage.toString(temp, CAN_MESSAGE_BUFFER);
                writeln(globalOutputStream, F("Parsed CAN message:"), LINE_ENDING);
                writeln(globalOutputStream, temp, LINE_ENDING); 
            }            
        } else {
            writeln(globalOutputStream, F("Unknown serial command"), LINE_ENDING);
        }
    }
}

SerialCommandType checkSerialCommands()
{
	if (globalInputStream->available()) {
		int serialRead{globalInputStream->read()};
		if (isPrintable(static_cast<char>(serialRead))) {
			char readChar{static_cast<char>(serialRead)};
			if (readChar == READ_MESSAGE_QUERY) {
				return findReadMessageType();
			} else if (readChar == WRITE_MESSAGE_QUERY) {
				return findWriteMessageType();
			}
		} else {
			return SerialCommandType::CommandTypeUnknown;
		}
	}
	return SerialCommandType::NoSerialCommand;
}

SerialCommandType findReadMessageType()
{
	unsigned long startTime{millis()};
	do {
		if (globalInputStream->available()) {
			int serialPeek{globalInputStream->peek()};
			if (isPrintable(static_cast<char>(serialPeek))) {
				char peekChar{static_cast<char>(serialPeek)};
				if (peekChar == LIN_MESSAGE_IDENTIFIER) {
					(void)globalInputStream->read();
					return SerialCommandType::ReadLinMessage;
				}
			} else {
				(void)globalInputStream->read();
			}
		}
	} while ((millis() - startTime) < FIND_READ_MESSAGE_TYPE_TIMEOUT);
	return SerialCommandType::CommandTypeUnknown;
}

SerialCommandType findWriteMessageType()
{
	unsigned long startTime{millis()};
	do {
		if (globalInputStream->available()) {
			int serialPeek{globalInputStream->peek()};
			if (isPrintable(static_cast<char>(serialPeek))) {
				char peekChar{static_cast<char>(serialPeek)};
				if (peekChar == CAN_MESSAGE_IDENTIFIER) {
					(void)globalInputStream->read();
					return  SerialCommandType::WriteCanMessage;
				} else if (peekChar == LIN_MESSAGE_IDENTIFIER) {
					(void)globalInputStream->read();
					return SerialCommandType::WriteLinMessage;
				}
			} else {
				(void)globalInputStream->read();
			}
		}
	} while ((millis() - startTime) < FIND_READ_MESSAGE_TYPE_TIMEOUT);
	return SerialCommandType::CommandTypeUnknown;
}

CanMessage parseCanMessage()
{
    char messageBuffer[CAN_MESSAGE_BUFFER];
    globalInputStream->setTimeout(PARSE_CAN_MESSAGE_TIMEOUT);
    size_t readByteCount{globalInputStream->readBytesUntil(LINE_ENDING, messageBuffer, CAN_MESSAGE_BUFFER)};
    globalInputStream->setTimeout(0);
    if (readByteCount > 0) {
        return CanMessage::parse(messageBuffer, DATA_DELIMITER);
    } else {
        return CanMessage{};
    }
}

LinMessage parseLinMessage()
{
    char messageBuffer[LIN_MESSAGE_BUFFER];
    globalInputStream->setTimeout(PARSE_LIN_MESSAGE_TIMEOUT);
    size_t readByteCount{globalInputStream->readBytesUntil(LINE_ENDING, messageBuffer, LIN_MESSAGE_BUFFER)};
    globalInputStream->setTimeout(0);
    if (readByteCount > 0) {
        return LinMessage::parse(messageBuffer, DATA_DELIMITER);
    } else {
        return LinMessage{};
    } 
}
