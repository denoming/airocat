#pragma once

#include <Arduino.h>

#include "DataValue.hpp"

class Publisher;

class Sensor2 {
public:
    explicit Sensor2(Publisher& publisher);

    void
    setEnvironmentalData(float humidity, float temperature);

    [[nodiscard]] bool
    setup(uint8_t address);

#if HOMEASSISTANT_INTEGRATE
    void
    integrate();
#endif

    [[nodiscard]] bool
    read();

    void
    publish();

    [[nodiscard]] uint16_t
    co2() const;

    [[nodiscard]] uint16_t
    tvoc() const;

private:
    void
    reset();

    static void
    printError();

private:
    Publisher& _publisher;
    DataValue<uint16_t> _co2;
    DataValue<uint16_t> _tvoc;
};