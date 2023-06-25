#include "Sensor2.hpp"

#include <SparkFunCCS811.h>

static CCS811 ccs;

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

bool
Sensor2::fetch()
{
    bool rv{false};
    if (ccs.dataAvailable()) {
        if (ccs.readAlgorithmResults() == CCS811Core::CCS811_Stat_SUCCESS) {
            _co2 = ccs.getCO2();
            _tvoc = ccs.getTVOC();
            rv = true;
        } else {
            reset();
        }
    } else if (ccs.checkForStatusError()) {
        reset();
        printError();
    }
    return rv;
}

uint16_t
Sensor2::co2() const
{
    return _co2;
}

uint16_t
Sensor2::tvoc() const
{
    return _tvoc;
}

void
Sensor2::reset()
{
    _co2 = 0;
    _tvoc = 0;
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
