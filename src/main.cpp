#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include "config.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastReconnectAttempt = 0;

uint8_t vespera_payload[PAYLOAD_SIZE];
const char* const COLOURS[4] = {"YELLOW", "BLUE", "RED", "GREEN"};

// 3. NON-BLOCKING GAME STATE & INPUT DEBOUNCING
enum GameState {
  STATE_SHOW_SEQ,
  STATE_PLAYING,
  STATE_ROUND_TRANSITION,
  STATE_VICTORY
};

GameState currentState = STATE_SHOW_SEQ;

uint8_t targetSequence[SEQUENCE_LENGTH];
uint8_t p1Progress = 0;
uint8_t p2Progress = 0;
uint8_t p1Fins = 0; // 0 to 6 fins (Fins 0-5)
uint8_t p2Fins = 0; // 0 to 6 fins (Fins 6-11)
uint8_t matchWinner = 0;

// Button detection & debounce tracking
bool p1LastPinState[4] = {HIGH, HIGH, HIGH, HIGH};
bool p2LastPinState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long p1LastPressTime[4] = {0, 0, 0, 0};
unsigned long p2LastPressTime[4] = {0, 0, 0, 0};

// Non blocking local LED timers
unsigned long ledOffTime[4] = {0, 0, 0, 0};

// Non blocking state timers
unsigned long stateTimer = 0;
unsigned long lastAnimFrameTime = 0;
uint8_t animFrame = 0;

 
// 4. VESPERA PAYLOAD & NETWORK FUNCTIONS
void clearLocalLEDs() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(PIN_LEDS[i], LOW);
    ledOffTime[i] = 0;
  }
}

void publishVesperaPayload() {
  if (mqttClient.connected()) {
    mqttClient.publish(TOPIC_CHANNEL_SELECT, VESPERA_CHANNEL_ID);
    bool success = mqttClient.publish(TOPIC_LUMINAIRE_DATA, vespera_payload, PAYLOAD_SIZE);
    if (success) {
      Serial.println("-> MQTT Publish SUCCESS");
    } else {
      Serial.println("-> MQTT Publish FAILED (Packet dropped)");
    }
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
  for (uint8_t f = 0; f < p1Fins && f < FINS_TO_WIN; f++) {
    setFinColour(f, P1_COLOUR[0], P1_COLOUR[1], P1_COLOUR[2]);
  }

  // Player 2 fills Fins 6 to 11 (Bytes 108-215)
  for (uint8_t f = 0; f < p2Fins && f < FINS_TO_WIN; f++) {
    setFinColour(6 + f, P2_COLOUR[0], P2_COLOUR[1], P2_COLOUR[2]);
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

    digitalWrite(PIN_LEDS[c], HIGH);
    delay(400);
    digitalWrite(PIN_LEDS[c], LOW);
    delay(220);
  }

  // Flush any button states held down during the sequence preview
  for (uint8_t i = 0; i < 4; i++) {
    p1LastPinState[i] = digitalRead(PIN_P1_BTNS[i]);
    p2LastPinState[i] = digitalRead(PIN_P2_BTNS[i]);
  }

  p1Progress = 0;
  p2Progress = 0;
  currentState = STATE_PLAYING;
  Serial.println("GO! Players race to input the sequence!");
}

void handlePlayerPress(uint8_t player, uint8_t index) {
  unsigned long now = millis();

  // Non-blocking visual echo on the corresponding LED
  digitalWrite(PIN_LEDS[index], HIGH);
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
    if (progress >= SEQUENCE_LENGTH) {
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
      if (p1Fins >= FINS_TO_WIN || p2Fins >= FINS_TO_WIN) {
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
      digitalWrite(PIN_LEDS[i], LOW);
      ledOffTime[i] = 0;
    }
  }

  // Poll all 8 buttons using falling edge detection (HIGH -> LOW)
  for (uint8_t i = 0; i < 4; i++) {
    // Player 1 Button i
    bool p1State = digitalRead(PIN_P1_BTNS[i]);
    if (p1State == LOW && p1LastPinState[i] == HIGH && (now - p1LastPressTime[i] >= DEBOUNCE_MS)) {
      p1LastPressTime[i] = now;
      handlePlayerPress(1, i);
      if (currentState != STATE_PLAYING) return; // Stop polling if round just ended
    }
    p1LastPinState[i] = p1State;

    // Player 2 Button i
    bool p2State = digitalRead(PIN_P2_BTNS[i]);
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
  while (!Serial);

  Serial.println(SECRET_WIFIPASS);
  Serial.println(SECRET_WIFIUSER);
  Serial.println(SECRET_MQTTPASS);
  Serial.println(SECRET_MQTTUSER);
  Serial.println("ffs");

  // Wait up to 2.5s for Serial Monitor, then boot standalone for power 
  unsigned long serialWait = millis();
  while (!Serial && (millis() - serialWait < 2500));

  for (uint8_t i = 0; i < 4; i++) {
    pinMode(PIN_LEDS[i], OUTPUT);
    digitalWrite(PIN_LEDS[i], LOW);
    pinMode(PIN_P1_BTNS[i], INPUT_PULLUP);
    pinMode(PIN_P2_BTNS[i], INPUT_PULLUP);
  }

  // Seed RNG from floating analog pin A0 (keep A0 unconnected)
  randomSeed(analogRead(PIN_RNG_SEED));

  // Expand PubSubClient packet buffer to fit topic + 216 byte payload cleanly
  mqttClient.setBufferSize(MQTT_BUF_SIZE);
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(SECRET_WIFIUSER);
  WiFi.begin(SECRET_WIFIUSER, SECRET_WIFIPASS);
  unsigned long wifiWait = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiWait < 6000)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Test - Waiting for MQTT broker...");
  // Trap the code here until MQTT is fully connected
  while (!mqttClient.connected()) {
    maintainNetwork();
    delay(250); // Small pause to prevent spamming the network
  }
  
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
          digitalWrite(PIN_LEDS[i], (i == (animFrame % 4)) ? HIGH : LOW);
        }
        animFrame++;
      }
      if (now - stateTimer >= 800) {
        startNewRound();
      }
      break;

    case STATE_VICTORY:
      // 6 second 8Hz (125ms) Vespera total takeover animation across all 72 LEDs
      if (now - lastAnimFrameTime >= VICTORY_FRAME_MS) {
        lastAnimFrameTime = now;
        const uint8_t* winRGB = (matchWinner == 1) ? P1_COLOUR : P2_COLOUR;

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
          digitalWrite(PIN_LEDS[i], (i == (animFrame % 4)) ? HIGH : LOW);
        }
        animFrame++;
      }

      if (now - stateTimer >= VICTORY_ANIM_MS) {
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