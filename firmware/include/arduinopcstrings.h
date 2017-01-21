#ifndef ARDUINOPC_ARDUINOPCSTRINGS_H
#define ARDUINOPC_ARDUINOPCSTRINGS_H

#include "arduinoinit.h"

namespace ArduinoPCStrings
{
    extern const char *ARDUINO_TYPE;

#if defined(__HAVE_CAN_BUS__)
    extern const char *CAN_INIT_HEADER;
    extern const char *CAN_READ_HEADER;
    extern const char *CAN_WRITE_HEADER;
    extern const char *CAN_WRITE_ONCE_HEADER;
    extern const char *CAN_LIVE_UPDATE_HEADER;

    extern const char *CLEAR_CAN_MESSAGES_HEADER;
    extern const char *CLEAR_CAN_MESSAGE_BY_ID_HEADER;
    extern const char *CURRENT_CAN_MESSAGES_HEADER;
    extern const char *CURRENT_CAN_MESSAGE_BY_ID_HEADER;
    
    extern const char *ADD_POSITIVE_CAN_MASK_HEADER;
    extern const char *REMOVE_POSITIVE_CAN_MASK_HEADER;
    extern const char *CURRENT_POSITIVE_CAN_MASKS_HEADER;
    extern const char *CLEAR_POSITIVE_CAN_MASKS_HEADER;

    extern const char *ADD_NEGATIVE_CAN_MASK_HEADER;
    extern const char *REMOVE_NEGATIVE_CAN_MASK_HEADER;
    extern const char *CURRENT_NEGATIVE_CAN_MASKS_HEADER;
    extern const char *CLEAR_NEGATIVE_CAN_MASKS_HEADER;

    extern const char *ALL_CURRENT_CAN_MASKS_HEADER;
    extern const char *CLEAR_ALL_CAN_MASKS_HEADER;
#endif

    extern const char *INITIALIZATION_HEADER;

    extern const char *HARDWARE_SERIAL_RX_PIN_TYPE;
    extern const char *HARDWARE_SERIAL_TX_PIN_TYPE;
    extern const char *SOFTWARE_SERIAL_RX_PIN_TYPE;
    extern const char *SOFTWARE_SERIAL_TX_PIN_TYPE;

    extern const char *ADD_SOFTWARE_SERIAL_HEADER;
    extern const char *REMOVE_SOFTWARE_SERIAL_HEADER;
    extern const char *ADD_HARDWARE_SERIAL_HEADER;
    extern const char *REMOVE_HARDWARE_SERIAL_HEADER;

    extern const char *DIGITAL_INPUT_IDENTIFIER;
    extern const char *DIGITAL_OUTPUT_IDENTIFIER;
    extern const char *ANALOG_INPUT_IDENTIFIER;
    extern const char *ANALOG_OUTPUT_IDENTIFIER;
    extern const char *DIGITAL_INPUT_PULLUP_IDENTIFIER;
    extern const char *INVALID_HEADER;
    extern const char *REMOVE_SOFTWARE_SERIAL_HEADER;
    extern const char *DIGITAL_READ_HEADER;
    extern const char *ANALOG_READ_HEADER;
    extern const char *DIGITAL_WRITE_HEADER;
    extern const char *DIGITAL_WRITE_ALL_HEADER;
    extern const char *ANALOG_WRITE_HEADER;
    extern const char *SOFT_DIGITAL_READ_HEADER;
    extern const char *SOFT_ANALOG_READ_HEADER;
    extern const char *PIN_TYPE_HEADER;
    extern const char *PIN_TYPE_CHANGE_HEADER;
    extern const char *ARDUINO_TYPE_HEADER;
    extern const char *CHANGE_A_TO_D_THRESHOLD_HEADER;
    extern const char *CURRENT_A_TO_D_THRESHOLD_HEADER;

    extern const char *IO_REPORT_HEADER;
    extern const char *IO_REPORT_END_HEADER;

    extern const char *CAN_BUS_ENABLED_HEADER;
    extern const char *FIRMWARE_VERSION_HEADER;
    extern const char *INVALID_LENGTH_EXCEEDED_HEADER;
    extern const char *UNSPECIFIED_IO_TYPE_IDENTIFIER;
    extern const char *FIRMWARE_VERSION;
    extern const char *HEARTBEAT_HEADER;
}

#endif //ARDUINOPC_ARDUINOPCSTRINGS_H