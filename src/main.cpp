#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// Pin Mapping (order: Yellow, Blue, Red, Green)
const int ledPins[4]   = {5, 4, 3, 2};       // D5 (Yel), D4 (Blue), D3 (Red), D2 (Grn)
const int p1Buttons[4] = {6, 7, 8, 9};       // D6 (Yel), D7 (Blue), D8 (Red), D9 (Grn)
const int p2Buttons[4] = {A1, A2, A3, A4};   // A1 (Yel), A2 (Blue), A3 (Red), A4 (Grn)
const char* colours[4] = {"YELLOW", "BLUE", "RED", "GREEN"};

const char* MQTT_BROKER = "mqtt.cetools.org";
const uint16_t MQTT_PORT = 1884;

const uint8_t NUM_FINS = 12;
const uint8_t LEDS_PER_FIN = 6;
const uint8_t TOTAL_LEDS = 72;
const uint16_t PAYLOAD_SIZE = 216;

uint8_t vespera_payload[PAYLOAD_SIZE];

const uint8_t P1_RGB[3] = {0, 220, 255}; // Cyan for Player 1 (Fins 0-5 / Bytes 0-107)
const uint8_t P2_RGB[3] = {255, 0, 120}; // Magenta for Player 2 (Fins 6-11 / Bytes 108-215)

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastReconnectAttempt = 0;

int targetSequence[3];
int p1Progress = 0;
int p2Progress = 0;

void publishMqtt(const char* topic, const char* payload) {
  if (mqttClient.connected()) {
    mqttClient.publish(topic, payload);
  }
}

void setupWifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(SECRET_WIFIUSER);
  
  while (WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

boolean reconnectMqtt() {
  if (mqttClient.connect("VesperaGameClient")) {
    Serial.println("MQTT connected.");
  }
  return mqttClient.connected();
}

void testAllLEDsOnBoot() {
  Serial.println("Testing LEDs (Yellow -> Blue -> Red -> Green)....");
  for (int i = 0; i < 4; i++) {
    Serial.print("Lighting LED: ");
    Serial.println(colours[i]);
    digitalWrite(ledPins[i], HIGH);
    delay(350);
    digitalWrite(ledPins[i], LOW);
    delay(150);
  }
}

void generateNewSequence() {
  Serial.println("\n--- NEW ROUND: Watch the 3 step sequence! ---");
  delay(800);

  for (int i = 0; i < 3; i++) {
    targetSequence[i] = random(0, 4);
    int c = targetSequence[i];

    Serial.print("Step ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(colours[c]);

    digitalWrite(ledPins[c], HIGH);
    delay(450);
    digitalWrite(ledPins[c], LOW);
    delay(250);
  }

  p1Progress = 0;
  p2Progress = 0;
  Serial.println("GO! Players race to input the sequence!");
}

void flashError() {
  for (int f = 0; f < 2; f++) {
    for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], HIGH);
    delay(120);
    for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);
    delay(120);
  }
}

void handlePress(int player, int index) {
  digitalWrite(ledPins[index], HIGH);

  int &progress = (player == 1) ? p1Progress : p2Progress;

  Serial.print("Player ");
  Serial.print(player);
  Serial.print(" pressed ");
  Serial.println(colours[index]);

  if (index == targetSequence[progress]) {
    progress++;
    Serial.print(" -> CORRECT! (");
    Serial.print(progress);
    Serial.println("/3)");

    delay(200);
    digitalWrite(ledPins[index], LOW);

    if (progress >= 3) {
      Serial.print("\n*** PLAYER ");
      Serial.print(player);
      Serial.println(" WINS THE ROUND! ***");

      // publish winner event
      publishMqtt("student/vespera/game/winner", player == 1 ? "player_1" : "player_2");

      for (int r = 0; r < 3; r++) {
        for (int i = 0; i < 4; i++) {
          digitalWrite(ledPins[i], HIGH);
          delay(70);
          digitalWrite(ledPins[i], LOW);
        }
      }
      delay(1000);
      generateNewSequence();
    }
  } else {
    Serial.println(" -> WRONG! Sequence reset to 0.");
    digitalWrite(ledPins[index], LOW);
    flashError();
    progress = 0;
  }
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    pinMode(p1Buttons[i], INPUT_PULLUP);
    pinMode(p2Buttons[i], INPUT_PULLUP);
  }

  randomSeed(analogRead(A0));

  while (!Serial);

  setupWifi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  reconnectMqtt();

  generateNewSequence();
}

void loop() {
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnectMqtt()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqttClient.loop();
  }

  for (int i = 0; i < 4; i++) {
    if (digitalRead(p1Buttons[i]) == LOW) {
      handlePress(1, i);
    }
    if (digitalRead(p2Buttons[i]) == LOW) {
      handlePress(2, i);
    }
  }
}