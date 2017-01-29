#ifndef ARDUINOPC_DEBUG_H
#define ARDUINOPC_DEBUG_H

class Debug
{
public:
    Debug(bool debug) :
        m_debug{debug}
    {
        if (this->m_debug) {
            Serial.begin(115200L);
        }
    }

    template <typename T>
    void print(const T &toPrint)
    {
        if (this->m_debug) {
            Serial.print(toPrint);
        }
    }

    template <typename T>
    void println(const T &toPrint)
    {
        if (this->m_debug) {
            Serial.println(toPrint);
            delay(1000);
        }
    }

private:
    bool m_debug;
};

#endif //ARDUINOPC_DEBUG_H