#include <Arduino.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "config.h"
#include "arduino_secrets.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);
  Serial.println("=== TEST SUITE: MQTT & Broker Communication ===");

  WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

  mqttClient.setBufferSize(MQTT_BUF_SIZE);
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);

  Serial.println("Connecting to MQTT broker...");
  char cid[24];
  snprintf(cid, sizeof(cid), "MKR-Diag-%lu", millis() & 0xFFFF);

  if (mqttClient.connect(cid, SECRET_MQTTUSER, SECRET_MQTTPASS)) {
    Serial.println("MQTT Handshake OK!");
    
    // Test channel claim
    bool chOk = mqttClient.publish(TOPIC_CHANNEL_SELECT, VESPERA_CHANNEL_ID);
    Serial.println(chOk ? "Channel Select OK" : "Channel Select FAILED");

    // Test payload publish (216 bytes)
    uint8_t testPayload[PAYLOAD_SIZE];
    memset(testPayload, 0x10, PAYLOAD_SIZE); // Dim white fill
    bool pubOk = mqttClient.publish(TOPIC_LUMINAIRE_DATA, testPayload, PAYLOAD_SIZE);
    Serial.println(pubOk ? "216-byte Payload Publish SUCCESS!" : "Publish FAILED (Check buffer size)");
  } else {
    Serial.print("MQTT Connection Failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void loop() {
  mqttClient.loop();
}