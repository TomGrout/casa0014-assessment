#include <Arduino.h>

// Pin Mapping (order: Yellow, Blue, Red, Green)
const int ledPins[4]   = {5, 4, 3, 2};       // D5 (Yel), D4 (Blue), D3 (Red), D2 (Grn)
const int p1Buttons[4] = {6, 7, 8, 9};       // D6 (Yel), D7 (Blue), D8 (Red), D9 (Grn)
const int p2Buttons[4] = {A1, A2, A3, A4};   // A1 (Yel), A2 (Blue), A3 (Red), A4 (Grn)
const char* colours[4] = {"YELLOW", "BLUE", "RED", "GREEN"};

int targetSequence[3];
int p1Progress = 0;
int p2Progress = 0;

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
  // light up the matching LED while pressed
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

    delay(200); // debounce + visual
    digitalWrite(ledPins[index], LOW);

    if (progress >= 3) {
      Serial.print("\n*** PLAYER ");
      Serial.print(player);
      Serial.println(" WINS THE ROUND! ***");

      // victory wave across the 4 LEDs
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

  // A0 is left unconnected to seed random numbers
  // !! IMPORTANT: do not connect A0 to anything
  randomSeed(analogRead(A0));

  while (!Serial);
  testAllLEDsOnBoot();
  generateNewSequence();
}

void loop() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(p1Buttons[i]) == LOW) {
      handlePress(1, i);
    }
    if (digitalRead(p2Buttons[i]) == LOW) {
      handlePress(2, i);
    }
  }
}