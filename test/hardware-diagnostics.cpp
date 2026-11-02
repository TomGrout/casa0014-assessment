#include <Arduino.h>
#include <WiFiNINA.h>

const uint8_t ledPins[4]   = {5, 4, 3, 2};
const uint8_t p1Buttons[4] = {6, 7, 8, 9};
const uint8_t p2Buttons[4] = {A1, A2, A3, A4};

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000);
  Serial.println("--- HARDWARE DIAGNOSTICS ---");

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(p1Buttons[i], INPUT_PULLUP);
    pinMode(p2Buttons[i], INPUT_PULLUP);
    
    digitalWrite(ledPins[i], HIGH);
    delay(200);
    digitalWrite(ledPins[i], LOW);
  }
  
  Serial.print("WiFi Module Status: ");
  Serial.println(WiFi.status() == WL_NO_MODULE ? "FAILED" : "OK");
}

void loop() {
  for (uint8_t i = 0; i < 4; i++) {
    if (digitalRead(p1Buttons[i]) == LOW) {
      Serial.print("P1 Button "); Serial.print(i); Serial.println(" OK");
      digitalWrite(ledPins[i], HIGH);
      delay(250);
      digitalWrite(ledPins[i], LOW);
    }
    if (digitalRead(p2Buttons[i]) == LOW) {
      Serial.print("P2 Button "); Serial.print(i); Serial.println(" OK");
      digitalWrite(ledPins[i], HIGH);
      delay(250);
      digitalWrite(ledPins[i], LOW);
    }
  }
}