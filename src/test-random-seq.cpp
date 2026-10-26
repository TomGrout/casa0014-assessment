#include <Arduino.h>

// Define the 4 LED pins
const int ledPins[4] = {4, 5, 6, 7}; 

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open (optional)

  // Configure all 4 LED pins as outputs and ensure they start OFF
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Seed the random number generator using electrical noise from analog pin A0
  randomSeed(analogRead(A0));
  
  Serial.println("[test] random led sequence");
}

void loop() {
  // Pick a random index between 0 and 3
  int randomIndex = random(0, 4);
  int activePin = ledPins[randomIndex];

  // Turn the chosen LED on
  digitalWrite(activePin, HIGH);
  Serial.print("flash -> Pin D");
  Serial.println(activePin);

  // Leave it lit for 300ms
  delay(300);

  // Turn it off
  digitalWrite(activePin, LOW);

  // Brief pause before picking the next one
  delay(200);
}