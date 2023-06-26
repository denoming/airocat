#include "Sensor1.hpp"

#include <bsec.h>

static Bsec bme;

static bsec_virtual_sensor_t kSensorList[13] = {
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

static bool
checkStatus()
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

bool
Sensor1::setup(uint8_t address)
{
    bme.begin(address, Wire);
    if (!checkStatus()) {
        Serial.println("Error on init BME680 sensor");
        return false;
    }

    bme.updateSubscription(kSensorList, 13, BSEC_SAMPLE_RATE_LP);
    if (!checkStatus()) {
        Serial.println("Error on update subscription for BME680 sensor");
        return false;
    }

    return true;
}

bool
Sensor1::fetch()
{
    if (bme.run()) {
        _iaq = bme.iaq;
        _co2Eq = bme.co2Equivalent;
        _breathVocEq = bme.breathVocEquivalent;
        _temperature = bme.temperature;
        _humidity = bme.humidity;
        _pressure = bme.pressure;
        _gasResistance = bme.gasResistance;
        _gasPercentage = bme.gasPercentage;
    }
    return checkStatus();
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
    return _iaq;
}

float
Sensor1::co2Eq() const
{
    return _co2Eq;
}

float
Sensor1::breathVocEq() const
{
    return _breathVocEq;
}

float
Sensor1::temperature() const
{
    return _temperature;
}

float
Sensor1::humidity() const
{
    return _humidity;
}

float
Sensor1::pressure() const
{
    return _pressure;
}

float
Sensor1::gasResistance() const
{
    return _gasResistance;
}

float
Sensor1::gasPercentage() const
{
    return _gasPercentage;
}
