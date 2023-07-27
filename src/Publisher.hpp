#pragma once

#include <Arduino.h>

class Publisher {
public:
    Publisher() = default;

    [[nodiscard]] bool
    connected() const;

    void
    setup();

    void
    connect();

    [[nodiscard]] bool
    publish(const char* topic, const char* payload, bool retained = true);
};