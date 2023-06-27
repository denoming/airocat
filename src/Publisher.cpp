#include "Publisher.hpp"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient{wifiClient};

bool
Publisher::connected() const
{
    return mqttClient.connected();
}

void
Publisher::setup()
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

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void
Publisher::connect()
{
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT: ");
        Serial.println(MQTT_HOST);
        String clientId = "airocat-";
        clientId += String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println("MQTT connected");
        } else {
            Serial.print("MQTT connecting failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

bool
Publisher::publish(const char* topic, const char* payload, boolean retained)
{
    Serial.print("Publish to "), Serial.println(topic);
    return mqttClient.publish(topic, payload, retained);
}