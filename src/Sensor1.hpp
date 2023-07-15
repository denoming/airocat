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

    explicit Sensor1(Publisher& publisher);

    bool
    setup(uint8_t address);

#if HOMEASSISTANT_INTEGRATE
    void
    integrate();
#endif

    bool
    publish();

    [[nodiscard]] Status
    initialStabStatus() const;

    [[nodiscard]] Status
    powerOnStabStatus() const;

    [[nodiscard]] float
    iaq() const;

    [[nodiscard]] float
    co2Eq() const;

    [[nodiscard]] float
    breathVocEq() const;

    [[nodiscard]] float
    temperature() const;

    [[nodiscard]] float
    humidity() const;

    [[nodiscard]] float
    pressure() const;

    [[nodiscard]] float
    gasResistance() const;

    [[nodiscard]] float
    gasPercentage() const;

private:
    static bool
    verifyStatus();

#if AIROCAT_STATE
    static void
    loadState();

    static void
    saveState();
#endif

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
    DataValue<float> _initialStatus;
    DataValue<float> _powerOnStatus;
};