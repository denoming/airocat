#include "Sensor2.hpp"

#include <SparkFunCCS811.h>

#include "Publisher.hpp"

static CCS811 ccs;

Sensor2::Sensor2(Publisher& publisher)
    : _publisher{publisher}
    , _co2{publisher, "CO2, ppm", "airocat/co2"}
    , _tvoc{publisher, "TVOC, ppb", "airocat/tvoc"}
{
}

void
Sensor2::setEnvironmentalData(float humidity, float temperature)
{
    ccs.setEnvironmentalData(humidity, temperature);
}

bool
Sensor2::setup(uint8_t address)
{
    ccs.setI2CAddress(address);

    if (not ccs.begin()) {
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
    json["name"] = "airocat/co2";
    json["state_topic"] = "airocat/co2";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/co2/config", &output[0], true);

    json.clear(), output.clear();
    json["device_class"] = "volatile_organic_compounds_parts";
    json["unit_of_measurement"] = "ppb"; 
    json["entity_category"] = "diagnostic";
    json["name"] = "airocat/tvoc";
    json["state_topic"] = "airocat/tvoc";
    json["value_template"] = "{{ value_json.value }}";
    serializeJson(json, output);
    _publisher.publish("homeassistant/sensor/airocat/tvoc/config", &output[0], true);
}
#endif

bool
Sensor2::publish()
{
    if (!ccs.dataAvailable()) {
        if (ccs.checkForStatusError()) {
            reset();
            printError();
        }
        return false;
    }

    if (ccs.readAlgorithmResults() == CCS811Core::CCS811_Stat_SUCCESS) {
        _co2.set(ccs.getCO2());
        _tvoc.set(ccs.getTVOC());
    } else {
        reset();
    }

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
Sensor2::reset()
{
    _co2.set(0);
    _tvoc.set(0);
}

void
Sensor2::printError()
{
    uint8_t error = ccs.getErrorRegister();
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
