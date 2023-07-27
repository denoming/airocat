#include "Sensor1.hpp"

#include <bsec.h>
#if AIROCAT_STATE
#include <EEPROM.h>
#endif
#if HOMEASSISTANT_INTEGRATE
#include <ArduinoJson.h>
#endif

namespace {

/* The sensor object declaration */
Bsec Sensor;

/* Save state period: every 360 minutes (4 times a day) */
constexpr const auto kSaveStatePeriod = UINT32_C(3 * 60 * 1000);

#if AIROCAT_STATE
/**
 * Configure the BSEC library:
 * 18v/33v = Voltage at Vdd. 1.8V or 3.3V
 * 3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
 * 4d/28d = Operating age of the sensor in days
 *
 * Possible config variants:
 * - generic_18v_3s_4d
 * - generic_18v_3s_28d
 * - generic_18v_300s_4d
 * - generic_18v_300s_28d
 * - generic_33v_3s_4d
 * - generic_33v_3s_28d
 * - generic_33v_300s_4d
 * - generic_33v_300s_28d
 */
const uint8 BsecConfig[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};
#endif

/* The list of sensors to activate */
bsec_virtual_sensor_t BsecSensorList[13] = {BSEC_OUTPUT_IAQ,
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
                                            BSEC_OUTPUT_GAS_PERCENTAGE};

#if AIROCAT_STATE
/* The sensor state data */
uint8_t BsecState[BSEC_MAX_STATE_BLOB_SIZE]{};
#endif

/* The MQTT topics to publish to */
const char* kIaqTopic = "airocat/iaq";
const char* kCo2EqTopic = "airocat/co2Eq";
const char* kBreathVocEqTopic = "airocat/breathVocEq";
const char* kTemperatureTopic = "airocat/temperature";
const char* kHumidityTopic = "airocat/humidity";
const char* kPressureTopic = "airocat/pressure";
const char* kGasResistanceTopic = "airocat/gasResistance";
const char* kGasPercentageTopic = "airocat/gasPercentage";
const char* kInitStabStatusTopic = "airocat/initialStabStatus";
const char* kPowerOnStabStatusTopic = "airocat/powerOnStabStatus";

} // namespace

Sensor1::Sensor1(Publisher& publisher)
    : _publisher{publisher}
    , _iaq{publisher, "IAQ", kIaqTopic}
    , _co2Eq{publisher, "CO2 (equivalent)", kCo2EqTopic}
    , _breathVocEq{publisher, "BreathVoc (equivalent)", kBreathVocEqTopic}
    , _temperature{publisher, "Temperature, °C", kTemperatureTopic}
    , _humidity{publisher, "Humidity, %", kHumidityTopic}
    , _pressure{publisher, "Pressure, hPa", kPressureTopic}
    , _gasResistance{publisher, "Gar (resistance), Ohm", kGasResistanceTopic}
    , _gasPercentage{publisher, "Gar (percentage), %", kGasPercentageTopic}
    , _initialStatus{publisher, "Initial stabilization status", kInitStabStatusTopic, -1.f}
    , _powerOnStatus{publisher, "Power-on stabilization status", kPowerOnStabStatusTopic, -1.f}
{
}

bool
Sensor1::setup(uint8_t address)
{
#if AIROCAT_STATE
    EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1);
#endif

    Sensor.begin(address, Wire);
    if (!verifyStatus()) {
        Serial.println("BME680: Error on init");
        return false;
    }

#if AIROCAT_STATE
    Sensor.setConfig(BsecConfig);
    if (!verifyStatus()) {
        Serial.println("BME680: Error on set config");
        return false;
    }
#endif

    Sensor.updateSubscription(BsecSensorList, 13, BSEC_SAMPLE_RATE_CONT);
    if (!verifyStatus()) {
        Serial.println("BME680: Error on update subscription");
        return false;
    }

#if AIROCAT_STATE
    loadState();
    if (!verifyStatus()) {
        Serial.println("BME680:  Error on load sensor state");
        return false;
    }
#endif

    return true;
}

#if HOMEASSISTANT_INTEGRATE
void
Sensor1::integrate()
{
    DynamicJsonDocument json{256};
    String output;
    json["device_class"] = "aqi";
    json["entity_category"] = "diagnostic";
    json["name"] = kIaqTopic;
    json["state_topic"] = kIaqTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/iaq/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kIaqTopic);
    }

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = kCo2EqTopic;
    json["state_topic"] = kCo2EqTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/co2Eq/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kCo2EqTopic);
    }

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = kBreathVocEqTopic;
    json["state_topic"] = kBreathVocEqTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/breathVocEq/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kBreathVocEqTopic);
    }

    json.clear(), output.clear();
    json["device_class"] = "temperature";
    json["entity_category"] = "diagnostic";
    json["unit_of_measurement"] = "C";
    json["name"] = kTemperatureTopic;
    json["state_topic"] = kTemperatureTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/temperature/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kTemperatureTopic);
    }

    json.clear(), output.clear();
    json["device_class"] = "humidity";
    json["unit_of_measurement"] = "%";
    json["entity_category"] = "diagnostic";
    json["name"] = kHumidityTopic;
    json["state_topic"] = kHumidityTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/humidity/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kHumidityTopic);
    }

    json.clear(), output.clear();
    json["device_class"] = "pressure";
    json["unit_of_measurement"] = "hPa";
    json["entity_category"] = "diagnostic";
    json["name"] = kPressureTopic;
    json["state_topic"] = kPressureTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/pressure/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kPressureTopic);
    }

    json.clear(), output.clear();
    json["unit_of_measurement"] = "Ohm";
    json["entity_category"] = "diagnostic";
    json["name"] = kGasResistanceTopic;
    json["state_topic"] = kGasResistanceTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/gasResistance/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kGasResistanceTopic);
    }

    json.clear(), output.clear();
    json["unit_of_measurement"] = "%";
    json["entity_category"] = "diagnostic";
    json["name"] = kGasPercentageTopic;
    json["state_topic"] = kGasPercentageTopic;
    json["value_template"] = "{{ value_json.value | round(1) }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/gasPercentage/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kGasPercentageTopic);
    }

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = kInitStabStatusTopic;
    json["state_topic"] = kInitStabStatusTopic;
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/initialStabStatus/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kInitStabStatusTopic);
    }

    json.clear(), output.clear();
    json["entity_category"] = "diagnostic";
    json["name"] = kPowerOnStabStatusTopic;
    json["state_topic"] = kPowerOnStabStatusTopic;
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    if (!_publisher.publish("homeassistant/sensor/airocat/powerOnStabStatus/config", &output[0])) {
        Serial.print("Unable to register: "), Serial.println(kPowerOnStabStatusTopic);
    }
}
#endif

bool
Sensor1::read()
{
    if (!Sensor.run()) {
        return verifyStatus();
    }

    _iaq.set(Sensor.iaq);
    _co2Eq.set(Sensor.co2Equivalent);
    _breathVocEq.set(Sensor.breathVocEquivalent);
    _temperature.set(Sensor.temperature);
    _humidity.set(Sensor.humidity);
    _pressure.set(Sensor.pressure);
    _gasResistance.set(Sensor.gasResistance);
    _gasPercentage.set(Sensor.gasPercentage);
    _initialStatus.set(Sensor.stabStatus);
    _powerOnStatus.set(Sensor.runInStatus);

#if AIROCAT_STATE
    saveState();
    if (!verifyStatus()) {
        Serial.println("BME680: Unable save sensor state");
    }
#endif

    return true;
}

void
Sensor1::publish()
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
        if (!_iaq.published() && stabilized()) {
            _iaq.publish();
        }
        if (!_co2Eq.published() && stabilized()) {
            _co2Eq.publish();
        }
        if (!_breathVocEq.published()) {
            _breathVocEq.publish();
        }
        if (!_temperature.published()) {
            _temperature.publish();
        }
        if (!_humidity.published()) {
            _humidity.publish();
        }
        if (!_pressure.published()) {
            _pressure.publish();
        }
        if (!_gasResistance.published()) {
            _gasResistance.publish();
        }
        if (!_gasPercentage.published()) {
            _gasPercentage.publish();
        }
        if (!_initialStatus.published()) {
            _initialStatus.publish();
        }
        if (!_powerOnStatus.published()) {
            _powerOnStatus.publish();
        }
    }
}

bool
Sensor1::stabilized() const
{
    return (initialStabStatus() == Status::Finished && powerOnStabStatus() == Status::Finished);
}

Sensor1::Status
Sensor1::initialStabStatus() const
{
    return (_initialStatus.get() == 0.f) ? Status::Ongoing : Status::Finished;
}

Sensor1::Status
Sensor1::powerOnStabStatus() const
{
    return (_powerOnStatus.get() == 0.f) ? Status::Ongoing : Status::Finished;
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

bool
Sensor1::verifyStatus()
{
    String output;
    if (Sensor.bsecStatus != BSEC_OK) {
        if (Sensor.bsecStatus < BSEC_OK) {
            output = "BSEC: Error " + String(Sensor.bsecStatus);
            Serial.println(output);
            return false;
        } else {
            output = "BSEC: Warning " + String(Sensor.bsecStatus);
            Serial.println(output);
        }
    }
    if (Sensor.bme68xStatus != BME68X_OK) {
        if (Sensor.bme68xStatus < BME68X_OK) {
            output = "BME680: Error " + String(Sensor.bme68xStatus);
            Serial.println(output);
            return false;
        } else {
            output = "BME680: Warning" + String(Sensor.bme68xStatus);
            Serial.println(output);
        }
    }
    return true;
}

#if AIROCAT_STATE
void
Sensor1::loadState()
{
    if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
        Serial.println("BME680: Reading state from EEPROM");
        for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
            BsecState[i] = EEPROM.read(i + 1);
        }
        Sensor.setState(BsecState);
    } else {
        Serial.println("BME680: Erasing EEPROM");
        for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++) {
            EEPROM.write(i, 0);
        }
        EEPROM.commit();
    }
}

void
Sensor1::saveState()
{
    static uint32 saveCounter{0};
    bool needUpdate = false;
    if (saveCounter == 0) {
        /* First state update when IAQ accuracy is >= 3 */
        if (Sensor.iaqAccuracy >= 3) {
            needUpdate = true;
            saveCounter++;
        }
    } else {
        /* Update every kSaveStatePeriod minutes */
        if ((saveCounter * kSaveStatePeriod) < millis()) {
            needUpdate = true;
            saveCounter++;
        }
    }
    if (needUpdate) {
        Serial.println("BME680: Writing state to EEPROM");
        Sensor.getState(BsecState);
        for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
            EEPROM.write(i + 1, BsecState[i]);
        }
        EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
        EEPROM.commit();
    }
}
#endif