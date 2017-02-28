#include <iostream>
#include <string>
#include <ctime>
#include <cstring>

class FunctionRequestEntry
{
    using CallbackFunction = void (*)(const char *);
public:
    FunctionRequestEntry(const char *header, CallbackFunction callback) :
        m_header{header},
        m_callback{callback}
    {

    }

    void call()
    {
        if (this->m_callback) {
            if (this->m_header) {
                this->m_callback(this->m_header);
            }
        }
    }

    const char *header() const
    {
        return this->m_header;
    }

    CallbackFunction callback() const
    {
        return this->m_callback;
    }

    friend bool operator==(const FunctionRequestEntry &lhs, const FunctionRequestEntry &rhs)
    {
        return (strcmp(lhs.m_header, rhs.m_header) == 0);
    }

    friend bool operator<(const FunctionRequestEntry &lhs, const FunctionRequestEntry &rhs)
    {
        if ((!lhs.header()) || (strlen(lhs.header()) == 0)) {
            return false;
        } else if ((!rhs.header()) || (strlen(rhs.header()) == 0)) {
            return true;
        } else {
            size_t shortestString{(strlen(lhs.header()) < strlen(rhs.header())) ? strlen(lhs.header()) : strlen(rhs.header())};
            for (size_t i = 0; i < shortestString; i++) {
                if (lhs.header()[i] != rhs.header()[i]) {
                    return (lhs.header()[i] < rhs.header()[i]);       
                }
            }
            return false;     
        }
    }
    
    friend bool operator>(const FunctionRequestEntry &lhs, const FunctionRequestEntry &rhs)
    {
        return !(lhs < rhs);
    }

    unsigned long getHash() const
    {
        unsigned long returnHash{5381};
        int currentChar{0};
        char *temp{new char[strlen(this->m_header) + 1]};
        strcpy(temp, this->m_header);

        while (*temp++ != '\0') {
            currentChar = *(temp - 1);
            returnHash = ((returnHash << 5) + returnHash) + currentChar; /* hash * 33 + c */
        }
        return returnHash;
    }


private:
    const char *m_header;
    CallbackFunction m_callback;
};

template <typename First, typename Second>
struct Pair
{
public:
    Pair() = delete;
    Pair(First firstItem, Second secondItem) : 
        first{firstItem},
        second{secondItem}
    {

    }

    First first;
    Second second;
};

template <typename Value>
struct LinkedList
{

    template <typename Value>
    struct ListNode
    {
        ListNode *previous;
        ListNode *next;
        Value  value;
    }
public:
    LinkedList(
}

const char * const ARDUINO_TYPE_HEADER{"ardtype"};
const char * const ANALOG_READ_HEADER{"aread"};
const char * const ANALOG_WRITE_HEADER{"awrite"};
const char * const DIGITAL_READ_HEADER{"dread"};
const char * const DIGITAL_WRITE_HEADER{"dwrite"};
const char * const PIN_TYPE_CHANGE_HEADER{"ptchange"};
const char * const SOFT_DIGITAL_READ_HEADER{"sdread"};
const char * const SOFT_ANALOG_READ_HEADER{"saread"};
const char * const FIRMWARE_VERSION_HEADER{"version"};

void arduinoTypeRequest(const char *str)
{
    (void)str;
    std::cout << "arduinoTypeRequest called" << std::endl;
}

void analogReadRequest(const char *str)
{
    (void)str;
    std::cout << "analogReadRequest called" << std::endl;
}

void analogWriteRequest(const char *str)
{
    (void)str;
    std::cout << "analogWriteRequest called" << std::endl;
}

void digitalReadRequest(const char *str)
{
    (void)str;
    std::cout << "digitalReadRequest called" << std::endl;
}

void digitalWriteRequest(const char *str)
{
    (void)str;
    std::cout << "digitalWriteRequest called" << std::endl;
}

void softDigitalReadRequest(const char *str)
{
    (void)str;
    std::cout << "softDigitalReadRequest called" << std::endl;
}

void softAnalogReadRequest(const char *str)
{
    (void)str;
    std::cout << "softAnalogReadRequest called" << std::endl;
}

void firmwareVersionRequest(const char *str)
{
    (void)str;
    std::cout << "firmwareVersionRequest called" << std::endl;
}

int main()
{
    FunctionRequestEntry functionRequestEntry{ARDUINO_TYPE_HEADER, arduinoTypeRequest};
    functionRequestEntry.call();
    srand(time(0));
    Pair<int, const char *> testPair{rand(), "Hello!"};
    std::cout << "testPair.first = " << testPair.first << ", testPair.second = " << testPair.second << std::endl;
    return 0;
}
