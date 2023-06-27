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
    {}

    void
    set(T value, boolean retain = true)
    {
        if (value != _value) {
            _value = value;
            publish(retain);
        }
    }

    const T&
    get() const
    {
        return _value;
    }


private:
    void
    publish(boolean retain)
    {
        static StaticJsonDocument<128> json;
        static String output;

        json.clear(), output.clear();
        json[kFieldCaption] = _caption;
        json[kFieldValue] = _value;
        serializeJson(json, output);

        _publisher.publish(_topic.c_str(), &output[0], retain);
    }

private:
    Publisher& _publisher;
    String _caption;
    String _topic;
    T _value{};
};