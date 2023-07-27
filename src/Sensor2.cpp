#include "Sensor2.hpp"

#include <SparkFunCCS811.h>
#include <ArduinoJson.h>

#include "Publisher.hpp"

namespace {

/* The sensor object declaration */
CCS811 Sensor;

/* The MQTT topics to publish to */
const char* kCo2Topic = "airocat/co2";
const char* kTvocTopic = "airocat/tvoc";

} // namespace

Sensor2::Sensor2(Publisher& publisher)
    : _publisher{publisher}
    , _co2{publisher, "CO2, ppm", "airocat/co2"}
    , _tvoc{publisher, "TVOC, ppb", "airocat/tvoc"}
{
}

void
Sensor2::setEnvironmentalData(float humidity, float temperature)
{
    Sensor.setEnvironmentalData(humidity, temperature);
}

bool
Sensor2::setup(uint8_t address)
{
    Sensor.setI2CAddress(address);

    if (not Sensor.begin()) {
        Serial.println(F("Could init CCS811 sensor"));
        return false;
    }

    return true;
}

#if HOMEASSISTANT_INTEGRATE
void
Sensor2::integrate()
{
    static StaticJsonDocument<128> json;
    String output;
    json["device_class"] = "carbon_dioxide";
    json["unit_of_measurement"] = "ppm";
    json["entity_category"] = "diagnostic";
    json["name"] = kCo2Topic;
    json["state_topic"] = kCo2Topic;
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/co2/config", &output[0], true);

    json.clear(), output.clear();
    json["device_class"] = "volatile_organic_compounds_parts";
    json["unit_of_measurement"] = "ppb";
    json["entity_category"] = "diagnostic";
    json["name"] = kTvocTopic;
    json["state_topic"] = kTvocTopic;
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/tvoc/config", &output[0], true);
}
#endif

bool
Sensor2::read()
{
    if (!Sensor.dataAvailable()) {
        if (Sensor.checkForStatusError()) {
            reset();
            printError();
        }
        return false;
    }

    if (Sensor.readAlgorithmResults() != CCS811Core::CCS811_Stat_SUCCESS) {
        reset();
        return false;
    }

    publish();

    return true;
}

uint16_t
Sensor2::co2() const
{
    return _co2.get();
}

uint16_t
Sensor2::tvoc() const
{
    return _tvoc.get();
}

void
Sensor2::publish()
{
    static auto lastTimestamp{0u};

    const auto currTimestamp = millis();
    const auto delay = currTimestamp - lastTimestamp;

    bool needPublish{false};
    if (delay >= AIROCAT_DELAY) {
        lastTimestamp = currTimestamp;
        needPublish = true;
    }

    if (needPublish) {
        _co2.set(Sensor.getCO2());
        _tvoc.set(Sensor.getTVOC());
    }
}

void
Sensor2::reset()
{
    _co2.set(0);
    _tvoc.set(0);
}

void
Sensor2::printError()
{
    uint8_t error = Sensor.getErrorRegister();
    if (error == 0xFF) {
        Serial.println("Failed to get error register value");
    } else {
        Serial.print("Error: ");
        if (error & 1 << 5) {
            Serial.print("HeaterSupply");
        } else if (error & 1 << 4) {
            Serial.print("HeaterFault");
        } else if (error & 1 << 3) {
            Serial.print("MaxResistance");
        } else if (error & 1 << 2) {
            Serial.print("MeasModeInvalid");
        } else if (error & 1 << 1) {
            Serial.print("ReadRegInvalid");
        } else if (error & 1 << 0) {
            Serial.print("MsgInvalid");
        } else {
            Serial.print("Unknown");
        }
        Serial.println();
    }
}
