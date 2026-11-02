#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

 
// 1. HARDWARE PIN MAPPING (Order: Yellow, Blue, Red, Green)
const uint8_t ledPins[4]   = {5, 4, 3, 2};     // D5 (Yel), D4 (Blu), D3 (Red), D2 (Grn)
const uint8_t p1Buttons[4] = {6, 7, 8, 9};     // D6 (Yel), D7 (Blu), D8 (Red), D9 (Grn)
const uint8_t p2Buttons[4] = {A1, A2, A3, A4}; // A1 (Yel), A2 (Blu), A3 (Red), A4 (Grn)
const char* const COLOURS[4] = {"YELLOW", "BLUE", "RED", "GREEN"};

 
// 2. VESPERA LUMINAIRE & MQTT CONFIGURATION
const char* MQTT_BROKER = "mqtt.cetools.org";
const uint16_t MQTT_PORT = 1884; // Use 1884 for authenticated or 1883 for open lab port

const uint8_t NUM_FINS = 12;
const uint8_t LEDS_PER_FIN = 6;
const uint8_t TOTAL_LEDS = 72;
const uint16_t PAYLOAD_SIZE = 216; // 72 LEDs * 3 RGB bytes

// Static 216-byte buffer 
uint8_t vespera_payload[PAYLOAD_SIZE];

// Player Signature Colours on Vespera (R, G, B)
const uint8_t P1_RGB[3] = {0, 220, 255}; // Cyan for Player 1 (Fins 0-5 / Bytes 0-107)
const uint8_t P2_RGB[3] = {255, 0, 120}; // Magenta for Player 2 (Fins 6-11 / Bytes 108-215)

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastReconnectAttempt = 0;

 
// 3. NON-BLOCKING GAME STATE & INPUT DEBOUNCING
enum GameState {
  STATE_SHOW_SEQ,
  STATE_PLAYING,
  STATE_ROUND_TRANSITION,
  STATE_VICTORY
};

GameState currentState = STATE_SHOW_SEQ;

uint8_t targetSequence[3];
uint8_t p1Progress = 0;
uint8_t p2Progress = 0;
uint8_t p1Fins = 0; // 0 to 6 fins (Fins 0-5)
uint8_t p2Fins = 0; // 0 to 6 fins (Fins 6-11)
uint8_t matchWinner = 0;

// Button detection & debounce tracking
const unsigned long DEBOUNCE_MS = 35;
bool p1LastPinState[4] = {HIGH, HIGH, HIGH, HIGH};
bool p2LastPinState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long p1LastPressTime[4] = {0, 0, 0, 0};
unsigned long p2LastPressTime[4] = {0, 0, 0, 0};

// Non blocking local LED timers
const unsigned long LED_ECHO_MS = 90;
unsigned long ledOffTime[4] = {0, 0, 0, 0};

// Non blocking state timers
unsigned long stateTimer = 0;
unsigned long lastAnimFrameTime = 0;
uint8_t animFrame = 0;

 
// 4. VESPERA PAYLOAD & NETWORK FUNCTIONS
void clearLocalLEDs() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
    ledOffTime[i] = 0;
  }
}

void publishVesperaPayload() {
  if (mqttClient.connected()) {
    mqttClient.publish(VESPERA_TOPIC, vespera_payload, PAYLOAD_SIZE);
  }
}

void setFinColour(uint8_t finIndex, uint8_t r, uint8_t g, uint8_t b) {
  if (finIndex >= NUM_FINS) return;
  uint16_t startByte = finIndex * LEDS_PER_FIN * 3; // 18 bytes per fin
  for (uint8_t led = 0; led < LEDS_PER_FIN; led++) {
    uint16_t idx = startByte + (led * 3);
    vespera_payload[idx]     = r;
    vespera_payload[idx + 1] = g;
    vespera_payload[idx + 2] = b;
  }
}

void updateScoreboardPayload() {
  memset(vespera_payload, 0, PAYLOAD_SIZE);

  // Player 1 fills Fins 0 to 5 (Bytes 0-107)
  for (uint8_t f = 0; f < p1Fins && f < 6; f++) {
    setFinColour(f, P1_RGB[0], P1_RGB[1], P1_RGB[2]);
  }

  // Player 2 fills Fins 6 to 11 (Bytes 108-215)
  for (uint8_t f = 0; f < p2Fins && f < 6; f++) {
    setFinColour(6 + f, P2_RGB[0], P2_RGB[1], P2_RGB[2]);
  }

  publishVesperaPayload();
}

void maintainNetwork() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS);
    return;
  }

  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 3000) {
      lastReconnectAttempt = now;
      char clientId[24];
      snprintf(clientId, sizeof(clientId), "MKR1010-%lu", now & 0xFFFF);

      if (mqttClient.connect(clientId, SECRET_MQTTUSER, SECRET_MQTTPASS)) {
        Serial.println("MQTT Connected to mqtt.cetools.org");
        publishVesperaPayload();
      }
    }
  } else {
    mqttClient.loop();
  }
}

 
// 5. GAMEPLAY & SEQUENCE GENERATION
void startNewRound() {
  clearLocalLEDs();
  Serial.println("\n--- NEW ROUND: Watch the 3-step sequence! ---");

  // Brief pre-round pause so players lift fingers off buttons
  delay(500);

  for (uint8_t i = 0; i < 3; i++) {
    targetSequence[i] = random(0, 4);
    uint8_t c = targetSequence[i];

    Serial.print("Step ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(COLOURS[c]);

    digitalWrite(ledPins[c], HIGH);
    delay(400);
    digitalWrite(ledPins[c], LOW);
    delay(220);
  }

  // Flush any button states held down during the sequence preview
  for (uint8_t i = 0; i < 4; i++) {
    p1LastPinState[i] = digitalRead(p1Buttons[i]);
    p2LastPinState[i] = digitalRead(p2Buttons[i]);
  }

  p1Progress = 0;
  p2Progress = 0;
  currentState = STATE_PLAYING;
  Serial.println("GO! Players race to input the sequence!");
}

void handlePlayerPress(uint8_t player, uint8_t index) {
  unsigned long now = millis();

  // Non-blocking visual echo on the corresponding LED
  digitalWrite(ledPins[index], HIGH);
  ledOffTime[index] = now + LED_ECHO_MS;

  uint8_t &progress = (player == 1) ? p1Progress : p2Progress;

  if (index == targetSequence[progress]) {
    progress++;
    Serial.print("P");
    Serial.print(player);
    Serial.print(" ");
    Serial.print(COLOURS[index]);
    Serial.print(" -> CORRECT (");
    Serial.print(progress);
    Serial.println("/3)");

    // Check for Round Win (3/3 matched)
    if (progress >= 3) {
      if (player == 1) p1Fins++;
      else p2Fins++;

      Serial.print("*** PLAYER ");
      Serial.print(player);
      Serial.print(" WINS THE ROUND! Score: P1=");
      Serial.print(p1Fins);
      Serial.print(" | P2=");
      Serial.print(p2Fins);
      Serial.println(" ***");

      updateScoreboardPayload();

      // Check for Match Win (6 fins = all 36 LEDs on that player's half)
      if (p1Fins >= 6 || p2Fins >= 6) {
        matchWinner = player;
        currentState = STATE_VICTORY;
        stateTimer = now;
        lastAnimFrameTime = 0;
        animFrame = 0;
        Serial.print(">>> PLAYER ");
        Serial.print(matchWinner);
        Serial.println(" TAKES OVER VESPERA! <<<");
      } else {
        currentState = STATE_ROUND_TRANSITION;
        stateTimer = now;
        animFrame = 0;
      }
    }
  } else {
    // NON-BLOCKING INSTANT RECOVERY:
    // If the button pressed matches Step 1 (targetSequence[0]), count it as Step 1
    // so a player restarting from the beginning never loses their first input.
    if (index == targetSequence[0]) {
      progress = 1;
      Serial.print("P");
      Serial.print(player);
      Serial.print(" ");
      Serial.print(COLOURS[index]);
      Serial.println(" -> MISMATCH, restarted at Step 1/3");
    } else {
      progress = 0;
      Serial.print("P");
      Serial.print(player);
      Serial.print(" ");
      Serial.print(COLOURS[index]);
      Serial.println(" -> WRONG, reset to 0/3");
    }
  }
}

void pollInputsNonBlocking() {
  unsigned long now = millis();

  // Turn off local echo LEDs whose nonblocking timer has expired
  for (uint8_t i = 0; i < 4; i++) {
    if (ledOffTime[i] > 0 && now >= ledOffTime[i]) {
      digitalWrite(ledPins[i], LOW);
      ledOffTime[i] = 0;
    }
  }

  // Poll all 8 buttons using falling edge detection (HIGH -> LOW)
  for (uint8_t i = 0; i < 4; i++) {
    // Player 1 Button i
    bool p1State = digitalRead(p1Buttons[i]);
    if (p1State == LOW && p1LastPinState[i] == HIGH && (now - p1LastPressTime[i] >= DEBOUNCE_MS)) {
      p1LastPressTime[i] = now;
      handlePlayerPress(1, i);
      if (currentState != STATE_PLAYING) return; // Stop polling if round just ended
    }
    p1LastPinState[i] = p1State;

    // Player 2 Button i
    bool p2State = digitalRead(p2Buttons[i]);
    if (p2State == LOW && p2LastPinState[i] == HIGH && (now - p2LastPressTime[i] >= DEBOUNCE_MS)) {
      p2LastPressTime[i] = now;
      handlePlayerPress(2, i);
      if (currentState != STATE_PLAYING) return;
    }
    p2LastPinState[i] = p2State;
  }
}

 
// 6. SETUP & MAIN LOOP
void setup() {
  Serial.begin(9600);
  // Wait up to 2.5s for Serial Monitor, then boot standalone for power 
  unsigned long serialWait = millis();
  while (!Serial && (millis() - serialWait < 2500));

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    pinMode(p1Buttons[i], INPUT_PULLUP);
    pinMode(p2Buttons[i], INPUT_PULLUP);
  }

  // Seed RNG from floating analog pin A0 (keep A0 unconnected)
  randomSeed(analogRead(A0));

  // Expand PubSubClient packet buffer to fit topic + 216 byte payload cleanly
  mqttClient.setBufferSize(512);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(SECRET_WIFIUSER);
  WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS);
  unsigned long wifiWait = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiWait < 6000)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  maintainNetwork();
  updateScoreboardPayload(); // Clear Vespera on boot
  startNewRound();
}

void loop() {
  maintainNetwork();
  unsigned long now = millis();

  switch (currentState) {
    case STATE_PLAYING:
      pollInputsNonBlocking();
      break;

    case STATE_ROUND_TRANSITION:
      // Quick non blocking local LED wave (600ms total) before next round
      if (now - lastAnimFrameTime >= 70) {
        lastAnimFrameTime = now;
        for (uint8_t i = 0; i < 4; i++) {
          digitalWrite(ledPins[i], (i == (animFrame % 4)) ? HIGH : LOW);
        }
        animFrame++;
      }
      if (now - stateTimer >= 800) {
        startNewRound();
      }
      break;

    case STATE_VICTORY:
      // 6 second 8Hz (125ms) Vespera total takeover animation across all 72 LEDs
      if (now - lastAnimFrameTime >= 125) {
        lastAnimFrameTime = now;
        const uint8_t* winRGB = (matchWinner == 1) ? P1_RGB : P2_RGB;

        for (uint8_t f = 0; f < NUM_FINS; f++) {
          if ((f + animFrame) % 2 == 0) {
            setFinColour(f, winRGB[0], winRGB[1], winRGB[2]);
          } else {
            setFinColour(f, winRGB[0] / 8, winRGB[1] / 8, winRGB[2] / 8);
          }
        }
        publishVesperaPayload();

        // Chase local 4 LEDs in sync
        for (uint8_t i = 0; i < 4; i++) {
          digitalWrite(ledPins[i], (i == (animFrame % 4)) ? HIGH : LOW);
        }
        animFrame++;
      }

      if (now - stateTimer >= 6000) {
        p1Fins = 0;
        p2Fins = 0;
        updateScoreboardPayload();
        startNewRound();
      }
      break;

    default:
      startNewRound();
      break;
  }
}