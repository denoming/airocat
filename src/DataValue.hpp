#include <ArduinoJson.h>

template<typename T>
class DataValue {
public:
    static constexpr const char* kFieldCaption = "caption";
    static constexpr const char* kFieldValue = "value";

    DataValue(PubSubClient& client, String caption, String topic, T value = {})
        : _client{client}
        , _caption{std::move(caption)}
        , _topic{std::move(topic)}
        , _value{std::move(value)}
    {}

    void
    set(T value, boolean retain)
    {
        if (value != _value) {
            _value = value;
            publish(retain);
        }
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

        _client.publish(_topic.c_str(), &output[0], retain);
    }

private:
    PubSubClient& _client;
    String _caption;
    String _topic;
    T _value{};
};