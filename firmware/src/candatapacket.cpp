#include "../include/candatapacket.h"

CanDataPacket::CanDataPacket() :
    CanDataPacket{0, 0, 0, 0, 0, 0, 0, 0}
{

}

CanDataPacket::CanDataPacket(unsigned char first, unsigned char second, 
                             unsigned char third, unsigned char fourth, 
                             unsigned char fifth, unsigned char sixth, 
                             unsigned char seventh, unsigned char eighth)
{
    this->m_dataPacket = (unsigned char *)malloc(NUMBER_OF_BYTES_IN_DATA_PACKET * sizeof(unsigned char));
    this->m_dataPacket[0] = first;
    this->m_dataPacket[1] = second;
    this->m_dataPacket[2] = third;
    this->m_dataPacket[3] = fourth;
    this->m_dataPacket[4] = fifth;
    this->m_dataPacket[5] = sixth;
    this->m_dataPacket[6] = seventh;
    this->m_dataPacket[7] = eighth;
}

CanDataPacket::CanDataPacket(unsigned char *packet)
{
    for (int i = 0; i < (NUMBER_OF_BYTES_IN_DATA_PACKET - 1); i++) {
        if (!(packet + i)) {
            this->m_dataPacket[i] = 0;
        } else {
            this->m_dataPacket[i] = *(packet + i);
        }
    }
}                                            

CanDataPacket::CanDataPacket(const CanDataPacket &dataPacket)
{
    this->m_dataPacket = (unsigned char *)malloc(NUMBER_OF_BYTES_IN_DATA_PACKET * sizeof(unsigned char));
    dataPacket.toBasicArray(this->m_dataPacket);
}

CanDataPacket::~CanDataPacket()
{
    free(this->m_dataPacket);
}

void CanDataPacket::setDataPacket(unsigned char first, unsigned char second, 
                                  unsigned char third, unsigned char fourth, 
                                  unsigned char fifth, unsigned char sixth, 
                                  unsigned char seventh, unsigned char eighth)
{
    this->m_dataPacket = (unsigned char *)malloc(NUMBER_OF_BYTES_IN_DATA_PACKET * sizeof(unsigned char));
    this->m_dataPacket[0] = first;
    this->m_dataPacket[1] = second;
    this->m_dataPacket[2] = third;
    this->m_dataPacket[3] = fourth;
    this->m_dataPacket[4] = fifth;
    this->m_dataPacket[5] = sixth;
    this->m_dataPacket[6] = seventh;
    this->m_dataPacket[7] = eighth;
}

void CanDataPacket::setDataPacket(unsigned char *packet)
{
    for (int i = 0; i < (NUMBER_OF_BYTES_IN_DATA_PACKET - 1); i++) {
        if (!(packet + i)) {
            this->m_dataPacket[i] = 0;
        } else {
            this->m_dataPacket[i] = *(packet + i);
        }
    }
}

bool CanDataPacket::setNthByte(int index, unsigned char nth)
{
    if ((index >= 0) && (index < NUMBER_OF_BYTES_IN_DATA_PACKET)) {
        this->m_dataPacket[index] = nth;
        return true;
    } else {
        return false;
    }
}

unsigned char CanDataPacket::nthByte(int index) const
{
    if ((index >= 0) && (index < NUMBER_OF_BYTES_IN_DATA_PACKET)) {
        return this->m_dataPacket[index];
    } else {
        return 0;
    }

}

void CanDataPacket::toBasicArray(unsigned char *copyArray) const
{
    for (int i = 0; i < (NUMBER_OF_BYTES_IN_DATA_PACKET - 1); i++) {
        copyArray[i] = this->m_dataPacket[i];
    }
}