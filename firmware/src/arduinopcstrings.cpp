#include "../include/arduinopcstrings.h"

namespace ArduinoPCStrings
{
#if defined(ARDUINO_AVR_UNO)
    const char *ARDUINO_TYPE{"arduino_uno"};
#elif defined(ARDUINO_AVR_NANO)
    const char *ARDUINO_TYPE{"arduino_nano"};
#elif defined(ARDUINO_AVR_MEGA1280) || defined(ARDUINO_AVR_MEGA2560)
    const char *ARDUINO_TYPE{"arduino_mega"};
#endif

#if defined(__HAVE_CAN_BUS__)
    const char *CLEAR_NEGATIVE_CAN_MASKS_HEADER{"{clearncanmasks"};
    const char *CURRENT_NEGATIVE_CAN_MASKS_HEADER{"{curncanmasks"};
    const char *CURRENT_POSITIVE_CAN_MASKS_HEADER{"{curpcanmasks"};
    const char *CLEAR_ALL_CAN_MASKS_HEADER{"{clearallcanmasks"};
    const char *CAN_INIT_HEADER{"{caninit"};
    const char *CAN_READ_HEADER{"{canread"};
    const char *CAN_WRITE_HEADER{"{canwrite"};
    const char *CAN_WRITE_ONCE_HEADER{"{canwriteo"};
    const char *CAN_LIVE_UPDATE_HEADER{"{canlup"};
    const char *CLEAR_CAN_MESSAGES_HEADER{"{clearcanmsgs"};
    const char *CLEAR_CAN_MESSAGE_BY_ID_HEADER{"{clearcanmsgid"};
    const char *CURRENT_CAN_MESSAGES_HEADER{"{curcanmsgs"};
    const char *CURRENT_CAN_MESSAGE_BY_ID_HEADER{"{curcanmsgid}"};
    const char *CLEAR_POSITIVE_CAN_MASKS_HEADER{"{clearpcanmasks"};
    
    const char *ADD_POSITIVE_CAN_MASK_HEADER{"{addpcanmask"};
    const char *ADD_NEGATIVE_CAN_MASK_HEADER{"{addncanmask"};
    const char *ALL_CURRENT_CAN_MASKS_HEADER{"{allcanmasks"};
    
    const char *REMOVE_POSITIVE_CAN_MASK_HEADER{"{rempcanmask"};
    const char *REMOVE_NEGATIVE_CAN_MASK_HEADER{"{remncanmask"};
#endif

    const char *HARDWARE_SERIAL_RX_PIN_TYPE{"hardserialrx"};
    const char *HARDWARE_SERIAL_TX_PIN_TYPE{"hardserialtx"};
    const char *SOFTWARE_SERIAL_RX_PIN_TYPE{"softserialrx"};
    const char *SOFTWARE_SERIAL_TX_PIN_TYPE{"softserialtx"};

    const char *INITIALIZATION_HEADER{"{arduinopc-firmware"};

    const char *ARDUINO_TYPE_HEADER{"{ardtype"};
    const char *ANALOG_READ_HEADER{"{aread"};
    const char *ANALOG_WRITE_HEADER{"{awrite"};
    const char *CHANGE_A_TO_D_THRESHOLD_HEADER{"{atodchange"};
    const char *CURRENT_A_TO_D_THRESHOLD_HEADER{"{atodthresh"};
    const char *ADD_SOFTWARE_SERIAL_HEADER{"{addsoftserial"};
    const char *REMOVE_SOFTWARE_SERIAL_HEADER{"{remsoftserial"};
    const char *ADD_HARDWARE_SERIAL_HEADER{"{addhardserial"};
    const char *REMOVE_HARDWARE_SERIAL_HEADER{"{remhardserial"};
    
    const char *CAN_BUS_ENABLED_HEADER{"{canbus"};
    
    const char *DIGITAL_READ_HEADER{"{dread"};
    const char *DIGITAL_WRITE_HEADER{"{dwrite"};
    const char *DIGITAL_WRITE_ALL_HEADER{"{dwriteall"};
    
    const char *IO_REPORT_HEADER{"{ioreport"};
    
    const char *PIN_TYPE_HEADER{"{ptype"};
    const char *PIN_TYPE_CHANGE_HEADER{"{ptchange"};
    
    const char *SOFT_DIGITAL_READ_HEADER{"{sdread"};
    const char *SOFT_ANALOG_READ_HEADER{"{saread"};

    const char *FIRMWARE_VERSION_HEADER{"{version"};
    const char *IO_REPORT_END_HEADER{"{ioreportend"};
    const char *DIGITAL_INPUT_IDENTIFIER{"din"};
    const char *DIGITAL_OUTPUT_IDENTIFIER{"dout"};
    const char *DIGITAL_INPUT_PULLUP_IDENTIFIER{"dinpup"};
    const char *ANALOG_INPUT_IDENTIFIER{"ain"};
    const char *ANALOG_OUTPUT_IDENTIFIER{"aout"};
    const char *UNSPECIFIED_IO_TYPE_IDENTIFIER{"unspecified"};
    const char *INVALID_HEADER{"{invalid"};
    const char *FIRMWARE_VERSION{"0.50"};

    
}
