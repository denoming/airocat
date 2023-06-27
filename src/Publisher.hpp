#pragma once

#include <Arduino.h>

class Publisher {
public:
    Publisher() = default;

    bool
    connected() const;

    void
    setup();

    void
    connect();

    bool
    publish(const char* topic, const char* payload, boolean retained);
};