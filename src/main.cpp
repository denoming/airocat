#include <Arduino.h>
#include <Wire.h>

#include "Publisher.hpp"
#include "Sensor1.hpp"
#include "Sensor2.hpp"

#define BME680_I2C_ADDR (0x77)
#define CCS811_I2C_ADDR (0x5A)

static Publisher publisher;
static Sensor1 sensor1{publisher};
static Sensor2 sensor2{publisher};

void
setup()
{
    /* Init serial */
    Serial.begin(9600);
    while (!Serial) {
        delay(10);
    }

    /* Init I2C on SCL(D1) and SDA(D2) */
    Wire.begin(D2, D1);
    delay(1000);

    while (!sensor1.setup(BME680_I2C_ADDR)) {
        Serial.println(F("Error on init Sensor1"));
        delay(1000);
    }

    while (!sensor2.setup(CCS811_I2C_ADDR)) {
        Serial.println(F("Error on init Sensor2"));
        delay(1000);
    }

    publisher.setup();
}

void
loop()
{
    if (!publisher.connected()) {
        publisher.connect();
#if HOMEASSISTANT_INTEGRATE
        sensor1.integrate();
        sensor2.integrate();
#endif
    }

    if (!sensor1.publish()) {
        Serial.println(F("Sensor1: No data"));
        Serial.println();
    }

    if (sensor2.publish()) {
        /* Set temperature and humidity to compensation Sensor2 */
        sensor2.setEnvironmentalData(sensor1.temperature(), sensor1.humidity());
    } else {
        Serial.println(F("Sensor2: No data"));
        Serial.println();
    }

    delay(2000);
}
