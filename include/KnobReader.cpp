#include <Encoder.cpp>

class KnobReader
{
private:
    long lastRead = 0;
    int value = 0;
    boolean valuesetmanually = false;

public:
    Encoder * encoder;
    int min_value{0}, max_value{1024};

    KnobReader(uint8_t pin1, uint8_t pin2) : encoder(new Encoder(pin1, pin2)) {};

    KnobReader(uint8_t pin1, uint8_t pin2, uint min, uint max) : encoder(new Encoder(pin1, pin2)), min_value(min), max_value(max)
    {
        Serial.println("KnobReader");
    }

    int *updateAndReturn()
    {
        // do not rely on internal value
        auto delta = encoder->readAndReset();

        if (delta == 0 && !valuesetmanually)
        {
            return nullptr;
        }
        double speed = 1;
        const long temp = millis();
        if (lastRead)
        {
            const auto diff = (temp - lastRead);
            speed = abs(delta) * 50.0 / diff;
#ifdef MDEBUG
            Serial.print("diff: ");
            Serial.print(diff, 3);
            Serial.print(" speed: ");
            Serial.print(speed, 3);
#endif
        }
        lastRead = temp;
#ifdef MDEBUG
        Serial.print(" delta: ");
        Serial.print(delta);
#endif
        if (speed > 1.0)
        {
            delta *= speed;
        }

        value += delta;
        if (value > max_value)
        {
            value = max_value;
        }
        if (value < min_value)
        {
            value = min_value;
        }
#ifdef MDEBUG
        Serial.print(" value: ");
        Serial.println(value);
#endif
        valuesetmanually = false;
        return &value;
    }

    void setValue(int _value)
    {
        valuesetmanually = true;
        value = _value;
    }
};