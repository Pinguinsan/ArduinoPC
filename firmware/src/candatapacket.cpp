#include "../include/candatapacket.h"

CanDataPacket::CanDataPacket() :
    m_dataPacket{std::vector<unsigned char>{0, 0, 0, 0, 0, 0, 0, 0}}
{

}

CanDataPacket::CanDataPacket(unsigned char first, unsigned char second, 
                             unsigned char third, unsigned char fourth, 
                             unsigned char fifth, unsigned char sixth, 
                             unsigned char seventh, unsigned char eighth) :
    m_dataPacket{std::vector<unsigned char>{first, second, third, fourth, 
                                            fifth, sixth, seventh, eighth}}
{

}                                            

CanDataPacket::CanDataPacket(const std::vector<unsigned char> &dataPacket) :
    m_dataPacket(dataPacket)
{

}

CanDataPacket::CanDataPacket(const CanDataPacket &dataPacket) :
    m_dataPacket{dataPacket.m_dataPacket}
{

}

void CanDataPacket::setDataPacket(const std::vector<unsigned char> &dataPacket)
{
    this->m_dataPacket = dataPacket;
}

void CanDataPacket::setDataPacket(unsigned char first, unsigned char second, 
                                  unsigned char third, unsigned char fourth, 
                                  unsigned char fifth, unsigned char sixth, 
                                  unsigned char seventh, unsigned char eighth)
{
    this->m_dataPacket = std::vector<unsigned char>{first, second, third, fourth,
                                                    fifth, sixth, seventh, eighth};
}                         

bool CanDataPacket::setNthByte(int index, unsigned char nth)
{
    if ((index >= 0) && (index < 8)) {
        this->m_dataPacket.at(index) = nth;
        return true;
    } else {
        return false;
    }
}

void CanDataPacket::toBasicArray(unsigned char copyArray[8]) const
{
    int i = 0;
    for (auto &it : this->m_dataPacket) {
        copyArray[i++] = it;
    }
}

std::vector<unsigned char> CanDataPacket::dataPacket() const
{
    return m_dataPacket;
}

CanDataPacket CanDataPacket::combineDataPackets(const CanDataPacket &first, const CanDataPacket &second)
{
    std::vector<unsigned char> constructorArg;
    std::vector<unsigned char> firstCopy = first.dataPacket();
    std::vector<unsigned char> secondCopy = second.dataPacket();
    std::vector<unsigned char>::const_iterator firstIter = firstCopy.begin();
    std::vector<unsigned char>::const_iterator secondIter = secondCopy.begin();
    while(firstIter != firstCopy.end()) {
        constructorArg.push_back((*firstIter++) | (*secondIter++));
    }
    return CanDataPacket(constructorArg);
}
