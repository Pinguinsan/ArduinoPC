#ifndef ARDUINOPC_ARDUINOPCSTRINGS_H
#define ARDUINOPC_ARDUINOPCSTRINGS_H

#include "arduinopcdefs.h"

namespace ArduinoPCStrings
{
#if defined(ARDUINO_AVR_UNO)
    const char * const ARDUINO_TYPE{"arduino_uno"};
#elif defined(ARDUINO_AVR_NANO)
    const char * const ARDUINO_TYPE{"arduino_nano"};
#elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    const char * const ARDUINO_TYPE{"arduino_mega"};
#endif

#if defined(__HAVE_CAN_BUS__)
    const char * const CLEAR_NEGATIVE_CAN_MASKS_HEADER{"clearncanmasks"};
    const char * const CURRENT_NEGATIVE_CAN_MASKS_HEADER{"curncanmasks"};
    const char * const CURRENT_POSITIVE_CAN_MASKS_HEADER{"curpcanmasks"};
    const char * const CLEAR_ALL_CAN_MASKS_HEADER{"clearallcanmasks"};
    const char * const CAN_INIT_HEADER{"caninit"};
    const char * const CAN_READ_HEADER{"canread"};
    const char * const CAN_WRITE_HEADER{"canwrite"};
    const char * const CAN_WRITE_ONCE_HEADER{"canwriteo"};
    const char * const CAN_LIVE_UPDATE_HEADER{"canlup"};
    const char * const CLEAR_CAN_MESSAGES_HEADER{"clearcanmsgs"};
    const char * const CLEAR_CAN_MESSAGE_BY_ID_HEADER{"clearcanmsgid"};
    const char * const CURRENT_CAN_MESSAGES_HEADER{"curcanmsgs"};
    const char * const CURRENT_CAN_MESSAGE_BY_ID_HEADER{"curcanmsgid"};
    const char * const CLEAR_POSITIVE_CAN_MASKS_HEADER{"clearpcanmasks"};
    
    const char * const ADD_POSITIVE_CAN_MASK_HEADER{"addpcanmask"};
    const char * const ADD_NEGATIVE_CAN_MASK_HEADER{"addncanmask"};
    const char * const ALL_CURRENT_CAN_MASKS_HEADER{"allcanmasks"};
    
    const char * const REMOVE_POSITIVE_CAN_MASK_HEADER{"rempcanmask"};
    const char * const REMOVE_NEGATIVE_CAN_MASK_HEADER{"remncanmask"};
#endif

#if defined(__HAVE_LIN_BUS__)

#endif

    const char * const HARDWARE_SERIAL_RX_PIN_TYPE{"hardserialrx"};
    const char * const HARDWARE_SERIAL_TX_PIN_TYPE{"hardserialtx"};
    const char * const SOFTWARE_SERIAL_RX_PIN_TYPE{"softserialrx"};
    const char * const SOFTWARE_SERIAL_TX_PIN_TYPE{"softserialtx"};

    const char * const INITIALIZATION_HEADER{"arduinopc-firmware"};

    const char * const ARDUINO_TYPE_HEADER{"ardtype"};
    const char * const ANALOG_READ_HEADER{"aread"};
    const char * const ANALOG_WRITE_HEADER{"awrite"};
    const char * const CHANGE_A_TO_D_THRESHOLD_HEADER{"atodchange"};
    const char * const CURRENT_A_TO_D_THRESHOLD_HEADER{"atodthresh"};
    const char * const ADD_SOFTWARE_SERIAL_HEADER{"addsoftserial"};
    const char * const REMOVE_SOFTWARE_SERIAL_HEADER{"remsoftserial"};
    
    const char * const CAN_BUS_ENABLED_HEADER{"canbus"};
    const char * const LIN_BUS_ENABLED_HEADER{"linbus"};
    
    const char * const DIGITAL_READ_HEADER{"dread"};
    const char * const DIGITAL_WRITE_HEADER{"dwrite"};
    const char * const DIGITAL_WRITE_ALL_HEADER{"dwriteall"};
    
    const char * const IO_REPORT_HEADER{"ioreport"};
    
    const char * const PIN_TYPE_HEADER{"ptype"};
    const char * const PIN_TYPE_CHANGE_HEADER{"ptchange"};
    
    const char * const SOFT_DIGITAL_READ_HEADER{"sdread"};
    const char * const SOFT_ANALOG_READ_HEADER{"saread"};

    const char * const FIRMWARE_VERSION_HEADER{"version"};
    const char * const IO_REPORT_END_HEADER{"ioreportend"};
    const char * const DIGITAL_INPUT_IDENTIFIER{"din"};
    const char * const DIGITAL_OUTPUT_IDENTIFIER{"dout"};
    const char * const DIGITAL_INPUT_PULLUP_IDENTIFIER{"dinpup"};
    const char * const ANALOG_INPUT_IDENTIFIER{"ain"};
    const char * const ANALOG_OUTPUT_IDENTIFIER{"aout"};
    const char * const UNSPECIFIED_IO_TYPE_IDENTIFIER{"unspecified"};
    const char * const INVALID_HEADER{"invalid"};
    const char * const FIRMWARE_VERSION{"0.50"};   
}

#endif //ARDUINOPC_ARDUINOPCSTRINGS_H