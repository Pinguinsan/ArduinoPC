#ifndef ARDUINOPC_CANDATAPACKET_H
#define ARDUINOPC_CANDATAPACKET_H

#include <stdlib.h>
#include "arduinopcstrings.h"

#define NUMBER_OF_BYTES_IN_DATA_PACKET 8

class CanDataPacket
{
public:
    CanDataPacket();
    CanDataPacket(unsigned char first, unsigned char second, 
                  unsigned char third, unsigned char fourth, 
                  unsigned char fifth, unsigned char sixth, 
                  unsigned char seventh, unsigned char eighth);
    CanDataPacket(const CanDataPacket &dataPacket);       
    CanDataPacket(unsigned char *packet);
    ~CanDataPacket();           
    void setDataPacket(unsigned char first, unsigned char second, 
                       unsigned char third, unsigned char fourth, 
                       unsigned char fifth, unsigned char sixth, 
                       unsigned char seventh, unsigned char eighth);
    void setDataPacket(unsigned char *packet);
    bool setNthByte(int index, unsigned char nth);
    unsigned char nthByte(int index) const;
    void toBasicArray(unsigned char *copyArray) const;

    friend bool operator==(const CanDataPacket &lhs, const CanDataPacket &rhs) 
    {
        for (int i = 0; i < (NUMBER_OF_BYTES_IN_DATA_PACKET - 1); i++) {
            if (lhs.nthByte(i) != rhs.nthByte(i)) {
                return false;
            }
        }
        return true;
    }

private:
    unsigned char *m_dataPacket;
};

#endif //ARDUINOPC_CANDATAPACKET_H
