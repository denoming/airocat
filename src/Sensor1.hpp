#include <Arduino.h>

class Sensor1 {
public:
    enum Status {
        Ongoing,
        Finished,
    };

    Sensor1() = default;

    bool
    setup(uint8_t address);

    bool
    fetch();

    Status
    initialStabStatus() const;

    Status
    powerOnStabStatus() const;

    float
    iaq() const;

    float
    co2() const;

    float
    breathVoc() const;

    float
    temperature() const;

    float
    humidity() const;

    float
    pressure() const;

    float
    gasResistance() const;

    float
    gasPercentage() const;

private:
    float _iaq{0.0f};
    float _co2{0.0f};
    float _breathVoc{0.0f};
    float _temperature{0.0f};
    float _humidity{0.0f};
    float _pressure{0.0f};
    float _gasResistance{0.0f};
    float _gasPercentage{0.0f};
};