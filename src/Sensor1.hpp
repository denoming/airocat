#pragma once

#include <Arduino.h>

#include "DataValue.hpp"

class Publisher;

class Sensor1 {
public:
    enum Status {
        Ongoing = 0,
        Finished,
    };

    Sensor1(Publisher& publisher);

    bool
    setup(uint8_t address);

#if HOMEASSISTANT_INTEGRATE
    void
    integrate();
#endif

    bool
    publish();

    Status
    initialStabStatus() const;

    Status
    powerOnStabStatus() const;

    float
    iaq() const;

    float
    co2Eq() const;

    float
    breathVocEq() const;

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
    Publisher& _publisher;
    DataValue<float> _iaq;
    DataValue<float> _co2Eq;
    DataValue<float> _breathVocEq;
    DataValue<float> _temperature;
    DataValue<float> _humidity;
    DataValue<float> _pressure;
    DataValue<float> _gasResistance;
    DataValue<float> _gasPercentage;
};