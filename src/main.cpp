#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
#define WIFI_SSID "TP-Link_5672"
#define WIFI_PASSWORD "31386678"

// HiveMQ credentials
#define MQTT_HOST "7124107a4ecb4c649fd3603493ad7203.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "iamfaiqahmed"
#define MQTT_PASS "Faiq1122!@"

#define TOPIC_TEMP "sensor/temperature"
#define TOPIC_HUMIDITY "sensor/humidity"
#define TOPIC_PRESSURE "sensor/pressure"

#define SDA_PIN 33
#define SCL_PIN 32

Adafruit_BME280 bme;
WiFiClientSecure espClient;
PubSubClient client(espClient);
bool bmeReady = false;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void connectMQTT() {
  espClient.setInsecure();
  client.setServer(MQTT_HOST, MQTT_PORT);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", MQTT_USER, MQTT_PASS)) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Wire.begin(SDA_PIN, SCL_PIN);
  connectWiFi();
  connectMQTT();
}

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  // Har baar BME280 detect karo
  bmeReady = bme.begin(0x76);
  if (!bmeReady) bmeReady = bme.begin(0x77);

  if (!bmeReady) {
    Serial.println("BME280 not found! Check wiring...");
    delay(1000);
    return;
  }

  float temp = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Pressure: "); Serial.println(pressure);

  client.publish(TOPIC_TEMP, String(temp).c_str());
  client.publish(TOPIC_HUMIDITY, String(humidity).c_str());
  client.publish(TOPIC_PRESSURE, String(pressure).c_str());

  delay(1000);
}