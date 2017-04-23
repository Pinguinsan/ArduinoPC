#include "linmessage.h"
#include "canmessage.h"

Stream *globalInputStream{nullptr};
Stream *globalOutputStream{nullptr};

enum SerialCommandType {
	CommandTypeUnknown = -1,
	NoSerialCommand = 0,
	ReadLinMessage = 1,
	SendLinMessage = 2,
	ReadCanMessage = 3
};

#define READ_MESSAGE_QUERY 'r'
#define WRITE_MESSAGE_QUERY 'w'
#define LIN_MESSAGE_IDENTIFIER 'l'
#define CAN_MESSAGE_IDENTIFIER 'c'

#define FIND_READ_MESSAGE_TYPE_TIMEOUT 200
#define FIND_WRITE_MESSAGE_TYPE_TIMEOUT 200

SerialCommandType checkSerialCommands();
SerialCommandType findReadMessageType();
SerialCommandType findWriteMessageType();

void setup()
{
	Serial->begin(115200L);
	globalInputStream = &Serial;
	globalOutputStream = &Serial;
}

void loop()
{

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
					return SerialCommandType::ReadLinType;
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
					return  SerialCommandType::WriteCanType;
				} else if (peekChar == LIN_MESSAGE_IDENTIFIER) {
					(void)globalInputStream->read();
					return SerialCommandType::WriteLinType;
				}
			} else {
				(void)globalInputStream->read();
			}
		}
	} while ((millis() - startTime) < FIND_READ_MESSAGE_TYPE_TIMEOUT);
	return SerialCommandType::CommandTypeUnknown;
}

