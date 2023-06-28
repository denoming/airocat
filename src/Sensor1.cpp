#include "Sensor1.hpp"

#include <bsec.h>

static Bsec bme;

namespace {

bsec_virtual_sensor_t kSensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
};

bool
verifySensorStatus()
{
    String output;
    if (bme.bsecStatus != BSEC_OK) {
        if (bme.bsecStatus < BSEC_OK) {
            output = F("BSEC error code : ") + String(bme.bsecStatus);
            Serial.println(output);
            return false;
        } else {
            output = F("BSEC warning code : ") + String(bme.bsecStatus);
            Serial.println(output);
        }
    }
    if (bme.bme68xStatus != BME68X_OK) {
        if (bme.bme68xStatus < BME68X_OK) {
            output = F("BME680 error code : ") + String(bme.bme68xStatus);
            Serial.println(output);
            return false;
        } else {
            output = F("BME680 warning code : ") + String(bme.bme68xStatus);
            Serial.println(output);
        }
    }
    return true;
}

}

Sensor1::Sensor1(Publisher& publisher)
    : _publisher{publisher}
    , _iaq(publisher, "IAQ", "airocat/iaq")
    , _co2Eq{publisher, "CO2 (equivalent)", "airocat/co2Eq"}
    , _breathVocEq{publisher, "BreathVoc (equivalent)", "airocat/breathVocEq"}
    , _temperature{publisher, "Temperature, °C", "airocat/temperature"}
    , _humidity{publisher, "Humidity, %", "airocat/humidity"}
    , _pressure{publisher, "Pressure, hPa", "airocat/pressure"}
    , _gasResistance{publisher, "Gar (resistance), Ohm", "airocat/gasResistance"}
    , _gasPercentage{publisher, "Gar (percentage), %", "airocat/gasPercentage"}
{
}

bool
Sensor1::setup(uint8_t address)
{
    bme.begin(address, Wire);
    if (!verifySensorStatus()) {
        Serial.println("Error on init BME680 sensor");
        return false;
    }

    bme.updateSubscription(kSensorList, 13, BSEC_SAMPLE_RATE_LP);
    if (!verifySensorStatus()) {
        Serial.println("Error on update subscription for BME680 sensor");
        return false;
    }

    return true;
}

#if HOMEASSISTANT_INTEGRATE
void
Sensor1::integrate()
{
    static StaticJsonDocument<128> json;
    String output;
    json["device_class"] = "aqi";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/iaq";
    json["state_topic"] = "airocat/iaq";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/iaq/config", &output[0], true);

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/co2Eq";
    json["state_topic"] = "airocat/co2Eq";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/co2Eq/config", &output[0], true);

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/breathVocEq";
    json["state_topic"] = "airocat/breathVocEq";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/breathVocEq/config", &output[0], true);

    json.clear(), output.clear();
    json["device_class"] = "temperature";
    json["unit_of_measurement"] = "°C";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/temperature";
    json["state_topic"] = "airocat/temperature";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/temperature/config", &output[0], true);

    json.clear(), output.clear();
    json["device_class"] = "humidity";
    json["unit_of_measurement"] = "%";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/humidity";
    json["state_topic"] = "airocat/humidity";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/humidity/config", &output[0], true);

    json.clear(), output.clear();
    json["device_class"] = "pressure";
    json["unit_of_measurement"] = "hPa";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/pressure";
    json["state_topic"] = "airocat/pressure";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/pressure/config", &output[0], true);

    json.clear(), output.clear();
    json["unit_of_measurement"] = "Ohm";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/gasResistance";
    json["state_topic"] = "airocat/gasResistance";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/gasResistance/config", &output[0], true);

    json.clear(), output.clear();
    json["unit_of_measurement"] = "%";
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/gasPercentage";
    json["state_topic"] = "airocat/gasPercentage";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/gasPercentage/config", &output[0], true);

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/initialStabStatus";
    json["state_topic"] = "airocat/initialStabStatus";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/initialStabStatus/config", &output[0], true);

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/powerOnStabStatus";
    json["state_topic"] = "airocat/powerOnStabStatus";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/powerOnStabStatus/config", &output[0], true);
}
#endif

bool
Sensor1::publish()
{
    if (!bme.run()) {
        return verifySensorStatus();
    }

    _iaq.set(bme.iaq);
    _co2Eq.set(bme.co2Equivalent);
    _breathVocEq.set(bme.breathVocEquivalent);
    _temperature.set(bme.temperature);
    _humidity.set(bme.humidity);
    _pressure.set(bme.pressure);
    _gasResistance.set(bme.gasResistance);
    _gasPercentage.set(bme.gasPercentage);

    return true;
}

Sensor1::Status
Sensor1::initialStabStatus() const
{
    return (bme.stabStatus == 0) ? Status::Ongoing : Status::Finished;
}

Sensor1::Status
Sensor1::powerOnStabStatus() const
{
    return (bme.runInStatus == 0) ? Status::Ongoing : Status::Finished;
}

float
Sensor1::iaq() const
{
    return _iaq.get();
}

float
Sensor1::co2Eq() const
{
    return _co2Eq.get();
}

float
Sensor1::breathVocEq() const
{
    return _breathVocEq.get();
}

float
Sensor1::temperature() const
{
    return _temperature.get();
}

float
Sensor1::humidity() const
{
    return _humidity.get();
}

float
Sensor1::pressure() const
{
    return _pressure.get();
}

float
Sensor1::gasResistance() const
{
    return _gasResistance.get();
}

float
Sensor1::gasPercentage() const
{
    return _gasPercentage.get();
}
