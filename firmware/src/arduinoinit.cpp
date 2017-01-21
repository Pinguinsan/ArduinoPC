#include "../include/arduinoinit.h"


void init()
{

}

void initVariant()
{

}

void setupUSB()
{

}

void arduinoInit()
{
    init();
    initVariant();
    #if defined(USBCON)
      USBDevice.attach();
    #endif
}

int atexit(void (* )())
{
    return 0;
}