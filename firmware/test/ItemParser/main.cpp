#include <iostream>
#include <cstring>
#include "utilities.h"

int main()
{
    const char *temp{"fish,15,4.0,2,5,lolfish"};
    int *first = new int{0};
    int *second = new int{0};
    double *third = new double{0.00};
    int *fourth = new int{0};
    int *fifth = new int{0};
    char *sixth = new char[10];

    Utilities::genericSplitCast(temp, ",", first, second, third, fourth, fifth, sixth);
    std::cout << "temp = " << temp << std::endl;
    std::cout << "first (int) parsed to ";
    if (!first) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *first << std::endl;
    }

    std::cout << "second (int) parsed to ";
    if (!second) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *second << std::endl;
    }

    std::cout << "third (double) parsed to ";
    if (!third) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *third << std::endl;
    }

    std::cout << "fourth (int) parsed to ";
    if (!fourth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *fourth<< std::endl;
    }

    std::cout << "fifth (int) parsed to ";
    if (!fifth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *fifth << std::endl;
    }

    std::cout << "sixth (c string) parsed to ";
    if (!sixth) {
        std::cout << "nullptr" << std::endl;
    } else {
        std::cout << *sixth << std::endl;
    }
}