#ifndef ARDUINOPC_BITSET_H
#define ARDUINOPC_BITSET_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum Endian
{ 
    LittleEndian, 
    BigEndian
};

class Bitset
{
public:
    Bitset(uint8_t numberOfBits, Endian endian = Endian::LittleEndian);
    void setAllBits();
    void resetAllBits();
    void setBit(uint8_t whichOne);
    void resetBit(uint8_t whichOne);
    void setByte(uint8_t whatByte);
    bool operator[](uint8_t whichOne);
    uint8_t underlyingValue() const;
    uint8_t numberOfBits() const;
    size_t toString(char *out, uint8_t spacing = 0);

private:
    uint8_t m_numberOfBits;
    uint8_t m_underlyingValue;
    Endian m_endian;

};

#endif //ARDUINOPC_BITSET_H