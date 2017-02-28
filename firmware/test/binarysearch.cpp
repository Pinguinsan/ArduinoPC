#include <iostream>
#include <string>
#include <cstring>

class TableEntry 
{
    using CallbackFunction = void (*)(const char *);
public:
    TableEntry(const char *header, CallbackFunction callback) :
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

    friend bool operator==(const TableEntry &lhs, const TableEntry &rhs)
    {
        return (strcmp(lhs.m_header, rhs.m_header) == 0);
    }

    friend bool operator<(const TableEntry &lhs, const TableEntry &rhs)
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
    
    friend bool operator>(const TableEntry &lhs, const TableEntry &rhs)
    {
        return !(lhs < rhs);
    }


private:
    const char *m_header;
    CallbackFunction m_callback;
};

class SortedSet
{
public:
    void insert(TableEntry *entry)
    {

    }
    void emplace(const char *header, CallbackFunction function)
    {
        
    }

};

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
    std::cout << "arduinoTypeRequest called" << std::endl;
}

void analogReadRequest(const char *str)
{
    std::cout << "analogReadRequest called" << std::endl;
}

void analogWriteRequest(const char *str)
{
    std::cout << "analogWriteRequest called" << std::endl;
}

void digitalReadRequest(const char *str)
{
    std::cout << "digitalReadRequest called" << std::endl;
}

void digitalWriteRequest(const char *str)
{
    std::cout << "digitalWriteRequest called" << std::endl;
}

void softDigitalReadRequest(const char *str)
{
    std::cout << "softDigitalReadRequest called" << std::endl;
}

void softAnalogReadRequest(const char *str)
{
    std::cout << "softAnalogReadRequest called" << std::endl;
}

void firmwareVersionRequest(const char *str)
{
    (void)str;
    std::cout << "firmwareVersionRequest called" << std::endl;
}

int main()
{
    TableEntry tableEntry{ARDUINO_TYPE, [](const char *arg){ std::cout << arg << std::endl; }};
    tableEntry.call();
    std::cout << "sizeof(tableEntry) = " << sizeof(tableEntry) << std::endl;
    std::cout << "sizeof(ARDUINO_TYPE) = " << sizeof(ARDUINO_TYPE) << std::endl;
    return 0;
}
