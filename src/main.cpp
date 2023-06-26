#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "Sensor1.hpp"
#include "Sensor2.hpp"
#include "DataValue.hpp"

#define BME680_I2C_ADDR (0x77)
#define CCS811_I2C_ADDR (0x5A)

static Sensor1 sensor1;
static Sensor2 sensor2;
static WiFiClient wifiClient;
static PubSubClient client{wifiClient};

static DataValue<float> iaqValue{client, "IAQ", "airocat/iaq"};
static DataValue<float> co2EqValue{client, "CO2 (equivalent)", "airocat/co2Eq"};
static DataValue<float> breathVocEqValue{client, "BreathVoc (equivalent)", "airocat/breathVocEq"};
static DataValue<float> tempValue{client, "Temperature, °C", "airocat/temperature"};
static DataValue<float> humidityValue{client, "Humidity, %", "airocat/humidity"};
static DataValue<float> pressureValue{client, "Pressure, hPa", "airocat/pressure"};
static DataValue<float> gasResValue{client, "Gar (resistance), Ohm", "airocat/gasResistance"};
static DataValue<float> gasPerValue{client, "Gar (percentage), %", "airocat/gasPercentage"};
static DataValue<int> initStatusValue{client, "Initial Stab Status", "airocat/initialStabStatus", -1};
static DataValue<int> powerOnStatusValue{client, "PowerOn Stab Status", "airocat/powerOnStabStatus", -1};
static DataValue<uint16_t> co2Value{client, "CO2, ppm", "airocat/co2"};
static DataValue<uint16_t> tvocValue{client, "TVOC, ppb", "airocat/tvoc"};

static void
wifiSetup()
{
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

static void
mqttSetup()
{
    client.setServer(MQTT_HOST, MQTT_PORT);
}

static void
mqttConnect()
{
    while (!client.connected()) {
        Serial.print("Connecting to MQTT: ");
        Serial.println(MQTT_HOST);
        String clientId = "airocat-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println("MQTT connected");
        } else {
            Serial.print("MQTT connecting failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

static void
sendData1()
{
    iaqValue.set(sensor1.iaq(), true);
    co2EqValue.set(sensor1.co2Eq(), true);
    breathVocEqValue.set(sensor1.breathVocEq(), true);
    tempValue.set(sensor1.temperature(), true);
    humidityValue.set(sensor1.humidity(), true);
    pressureValue.set(sensor1.pressure(), true);
    gasResValue.set(sensor1.gasResistance(), true);
    gasPerValue.set(sensor1.gasPercentage(), true);
    initStatusValue.set(sensor1.initialStabStatus(), true);
    powerOnStatusValue.set(sensor1.powerOnStabStatus(), true);
}

static void
sendData2()
{
    co2Value.set(sensor2.co2(), true);
    tvocValue.set(sensor2.tvoc(), true);
}

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

    wifiSetup();
    mqttSetup();
}

void
loop()
{
    if (!client.connected()) {
        mqttConnect();
    }
    client.loop();

    if (sensor1.fetch()) {
        sendData1();
    } else {
        Serial.println(F("Sensor1: No data"));
        Serial.println();
    }

    if (sensor2.fetch()) {
        sendData2();

        /* Set temperature and humidity to compensation Sensor2 */
        sensor2.setEnvironmentalData(sensor1.temperature(), sensor1.humidity());
    } else {
        Serial.println(F("Sensor2: No data"));
        Serial.println();
    }

    delay(2000);
}
