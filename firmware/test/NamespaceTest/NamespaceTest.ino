namespace U55X 
{
    namespace U553 
    {
        namespace _75K308 
        {
            static const uint8_t variationCode{0x1};
            static const char *partNumber{"75K308"}; 
        }

        namespace _75K309
        {
            static const uint8_t variationCode{0x2};
            static const char *partNumber{"75K309"}; 
        }
    }

    namespace U554
    {
        namespace _75K313
        {
            static const uint8_t variationCode{0x4};
            static const char *partNumber{"75K313"}; 
        }

        namespace _75K314
        {
            static const uint8_t variationCode{0x8};
            static const char *partNumber{"75K314"}; 
        }

        namespace _75K315
        {
            static const uint8_t variationCode{0x10};
            static const char *partNumber{"75K315"}; 
        }

        namespace _75K316
        {
            static const uint8_t variationCode{0x20};
            static const char *partNumber{"75K316"};    
        }
    }
}

void setup() 
{
    Serial.begin(115200L);
}

void loop() 
{
    static uint8_t currentVariationCode{U55X::U553::_75K308::variationCode};
}
