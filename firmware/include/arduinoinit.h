#ifndef ARDUINOPC_ARDUINOINIT_H
#define ARDUINOPC_ARDUINOINIT_H

/*
 * Un/comment the below line to enable and disable
 * the CAN bus, using the SEEEDStudio CAN Shield
 * https://github.com/Seeed-Studio/CAN_BUS_Shield
 */
//#define __HAVE_CAN_BUS__

#include <Arduino.h>

// Declared weak in Arduino.h to allow user redefinitions.
// Weak empty variant initialization function.
// May be redefined by variant files.
void init() __attribute__((weak));
void initVariant() __attribute__((weak));
void setupUSB() __attribute__((weak));

extern void init();
extern void initVariant();
extern void setupUSB();
extern void arduinoInit(); 
extern int atexit(void (* )());

#endif //ARDUINOPC_ARDUINOINIT_H

