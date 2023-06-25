#include <Arduino.h>

class Sensor2 {
public:
    Sensor2() = default;

    void
    setEnvironmentalData(float humidity, float temperature);

    bool
    setup(uint8_t address);

    bool
    fetch();

    uint16_t
    co2() const;

    uint16_t
    tvoc() const;

private:
    void
    reset();

    void
    printError();

private:
    uint16_t _co2{0};
    uint16_t _tvoc{0};
};