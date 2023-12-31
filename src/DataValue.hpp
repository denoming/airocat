#pragma once

#include <ArduinoJson.h>

#include "Publisher.hpp"

template<typename T>
class DataValue {
public:
    static constexpr const char* kFieldCaption = "caption";
    static constexpr const char* kFieldValue = "value";

    DataValue(Publisher& publisher, String caption, String topic, T value = {})
        : _publisher{publisher}
        , _caption{std::move(caption)}
        , _topic{std::move(topic)}
        , _value{std::move(value)}
        , _published{false}
    {
    }

    [[nodiscard]] bool
    published() const
    {
        return _published;
    }

    void
    set(T value)
    {
        if (_value != value) {
            _published = false;
            _value = value;
        }
    }

    const T&
    get() const
    {
        return _value;
    }

    void
    publish(bool retain = false)
    {
        static StaticJsonDocument<128> json;
        static String output;

        json.clear(), output.clear();
        json[kFieldCaption] = _caption;
        json[kFieldValue] = _value;
        serializeJson(json, output);

        if (_publisher.publish(_topic.c_str(), &output[0], retain)) {
            _published = true;
        }
    }

private:
private:
    Publisher& _publisher;
    String _caption;
    String _topic;
    T _value{};
    bool _published;
};