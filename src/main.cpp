#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#include "Sensor1.hpp"
#include "Sensor2.hpp"

#define BME680_I2C_ADDR (0x77)
#define CCS811_I2C_ADDR (0x5A)

static Sensor1 sensor1;
static Sensor2 sensor2;

void
printInfo1();

void
printInfo2();

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
}

void
loop()
{
    if (sensor1.fetch()) {
        printInfo1();
    } else {
        Serial.println(F("Sensor1: No data"));
        Serial.println();
    }

    if (sensor2.fetch()) {
        printInfo2();

        /* Set temperature and humidity to compensation Sensor2 */
        sensor2.setEnvironmentalData(sensor1.temperature(), sensor1.humidity());
    } else {
        Serial.println(F("Sensor2: No data"));
        Serial.println();
    }

    delay(2000);
}

void
printInfo1()
{
    Serial.print("IAQ: "), Serial.print(sensor1.iaq()), Serial.println();
    Serial.print("CO2 (equivalent): "), Serial.print(sensor1.co2()), Serial.println();
    Serial.print("BreathVoc (equivalent): "), Serial.print(sensor1.breathVoc()), Serial.println();
    Serial.print("Temperature: "), Serial.print(sensor1.temperature()), Serial.println(" °C");
    Serial.print("Humidity: "), Serial.print(sensor1.humidity()), Serial.println(" %");
    Serial.print("Pressure: "), Serial.print(sensor1.pressure()), Serial.println(" hPa");
    Serial.print("Gar (resistance): "), Serial.print(sensor1.gasResistance()), Serial.println(" Ohm");
    Serial.print("Gar (percentage): "), Serial.print(sensor1.gasPercentage()), Serial.println(" %");
    Serial.print("Initial Stab Status: ");
    Serial.print(sensor1.initialStabStatus() == Sensor1::Ongoing ? "Ongoing" : "Finished");
    Serial.println();
    Serial.print("PowerOn Stab Status: ");
    Serial.print(sensor1.powerOnStabStatus() == Sensor1::Ongoing ? "Ongoing" : "Finished");
    Serial.println();
    Serial.println("=============================");
}

void
printInfo2()
{
    Serial.print("CO2: "), Serial.print(sensor2.co2()), Serial.println(" ppm");
    Serial.print("TVOC: "), Serial.print(sensor2.tvoc()), Serial.println(" ppb");
    Serial.println("=============================");
}