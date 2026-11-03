#include <Arduino.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "config.h"
#include "arduino_secrets.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
uint8_t payload[PAYLOAD_SIZE];

void setFin(uint8_t fin, uint8_t r, uint8_t g, uint8_t b) {
  if (fin >= NUM_FINS) return;
  uint16_t offset = fin * LEDS_PER_FIN * 3;
  for (uint8_t i = 0; i < LEDS_PER_FIN; i++) {
    payload[offset + (i * 3)]     = r;
    payload[offset + (i * 3) + 1] = g;
    payload[offset + (i * 3) + 2] = b;
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 2500);
  WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS);
  while (WiFi.status() != WL_CONNECTED) delay(200);

  mqttClient.setBufferSize(MQTT_BUF_SIZE);
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  char cid[24];
  snprintf(cid, sizeof(cid), "PattTest-%lu", millis() & 0xFFFF);
  mqttClient.connect(cid, SECRET_MQTTUSER, SECRET_MQTTPASS);
  mqttClient.publish(TOPIC_CHANNEL_SELECT, VESPERA_CHANNEL_ID);
}

void loop() {
  if (!mqttClient.connected()) return;

  // Cycle 1: sweep through each of the 12 fins individually
  for (uint8_t f = 0; f < NUM_FINS; f++) {
    memset(payload, 0, PAYLOAD_SIZE);
    setFin(f, 0, 200, 255); // Highlight fin f in blue
    mqttClient.publish(TOPIC_LUMINAIRE_DATA, payload, PAYLOAD_SIZE);
    delay(300);
  }

  // Cycle 2: clear luminaire
  memset(payload, 0, PAYLOAD_SIZE);
  mqttClient.publish(TOPIC_LUMINAIRE_DATA, payload, PAYLOAD_SIZE);
  delay(1000);
}