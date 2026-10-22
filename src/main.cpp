#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
void setup() {
  Serial.begin(9600);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  bool btn1 = digitalRead(0);
  bool btn2 = digitalRead(1);
  bool btn3 = digitalRead(2);

  if (btn1 == LOW) {
    Serial.println("Button 1 (D0) Pressed!");
    delay(200);
  }
  if (btn2 == LOW) {
    Serial.println("Button 2 (D1) Pressed!");
    delay(200); 
  }
  if (btn3 == LOW) {
    Serial.println("Button 3 (D2) Pressed!");
    delay(200); 
  }
}