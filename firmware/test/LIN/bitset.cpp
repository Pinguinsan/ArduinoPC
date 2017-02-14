#include "bitset.h"

Bitset::Bitset(uint8_t numberOfBits, Endian endian) :
    m_numberOfBits{numberOfBits},
    m_underlyingValue{0},
    m_endian{endian}
{

}

void Bitset::setAllBits()
{
    for (int i = 0; i < this->m_numberOfBits; i++) {
        this->setBit(i);
    }
}

void Bitset::resetAllBits()
{
    this->m_underlyingValue = 0;
}

void Bitset::setBit(uint8_t whichOne)
{
    if (whichOne > this->m_numberOfBits) {

    }
    if (this->m_endian == Endian::LittleEndian) {
        this->m_underlyingValue |= 1 << (this->m_numberOfBits - whichOne - 1);
    } else {
        this->m_underlyingValue |= 1 << whichOne;
    }
}

void Bitset::resetBit(uint8_t whichOne)
{
    if (whichOne > this->m_numberOfBits) {

    }
    if (this->m_endian == Endian::LittleEndian) {
        this->m_underlyingValue &= ~(1 << (this->m_numberOfBits - whichOne - 1));
    } else {
        this->m_underlyingValue &= ~(1 << whichOne);
    }
}

void Bitset::setByte(uint8_t whatByte)
{
    for (int i = 0; i < this->m_numberOfBits; i++) {
        ((whatByte >> i) & 1) ? this->setBit(i) : this->resetBit(i);
    }
}

bool Bitset::operator[](uint8_t whichOne)       
{ 
    if (whichOne > this->m_numberOfBits) {

    }
    if (this->m_endian == Endian::LittleEndian) {
        return static_cast<bool>((this->m_underlyingValue >> (this->m_numberOfBits - whichOne - 1)) & 1);
    } else {
        return static_cast<bool>((this->m_underlyingValue >> whichOne) & 1);
    }
}

uint8_t Bitset::underlyingValue() const
{
    return this->m_underlyingValue;
}

uint8_t Bitset::numberOfBits() const
{
    return this->m_numberOfBits;
}

size_t Bitset::toString(char *out, uint8_t spacing)
{
    char temp[2];
    size_t maximumSize = this->m_numberOfBits  + (spacing * this->m_numberOfBits - 1) + 1;
    if (this->m_endian == Endian::LittleEndian) {
        for (int i = this->m_numberOfBits - 1; i >= 0; i--) {
            snprintf(temp, 2, "%i", this->operator[](i));
            if (i == (this->m_numberOfBits - 1)) {
                strncpy(out, temp, maximumSize);
            } else {
                strncat(out, temp, maximumSize);
            }
            if ((i - 1) >= 0) {
                for (uint8_t spaceCount = 0; spaceCount < spacing; spaceCount++) {
                    strncat(out, " ", maximumSize);
                }
            }
        }
    } else if (this->m_endian == Endian::BigEndian) {
        for (int i = 0; i < this->m_numberOfBits; i++) {
            snprintf(temp, 2, "%i", this->operator[](i));
            if (i == 0) {
                strncpy(out, temp, maximumSize);
            } else {
                strncat(out, temp, maximumSize);
            } 
            if ((i + 1) != this->m_numberOfBits) {
                for (uint8_t spaceCount = 0; spaceCount < spacing; spaceCount++) {
                    strncat(out, " ", maximumSize);
                }
            }
        }
    } else {

    }
    return strlen(out);
}