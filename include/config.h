// Set macros and hardcoded variables here, helps keep main.cpp cleaner & more readable

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

 
// 1. MQTT BROKER & TOPIC CONFIGURATION
// Change channel ID here if desired
#define VESPERA_CHANNEL_ID     "14" 
#define MQTT_BROKER_HOST       "mqtt.cetools.org"
#define MQTT_BROKER_PORT       1884

// Topic
#define TOPIC_PREFIX           "student/CASA0014/luminaire/"
#define TOPIC_CHANNEL_SELECT   TOPIC_PREFIX "user"
#define TOPIC_LUMINAIRE_DATA   TOPIC_PREFIX VESPERA_CHANNEL_ID

 
// 2. VESPERA & BUFFER SIZES
const uint8_t NUM_FINS         = 12;
const uint8_t LEDS_PER_FIN     = 6;
const uint8_t TOTAL_LEDS       = NUM_FINS * LEDS_PER_FIN; // 72 LEDs
const uint16_t PAYLOAD_SIZE    = TOTAL_LEDS * 3;          // 216 RGB bytes
const uint16_t MQTT_BUF_SIZE   = 512;

 
// 3. HARDWARE PIN DEFINITIONS
// Array indices: 0 = Yellow, 1 = Blue, 2 = Red, 3 = Green
const uint8_t PIN_LEDS[4]      = {5, 4, 3, 2};
const uint8_t PIN_P1_BTNS[4]   = {6, 7, 8, 9};
const uint8_t PIN_P2_BTNS[4]   = {A1, A2, A3, A4};
const uint8_t PIN_RNG_SEED     = A0; // Floating analog pin (for random seed)

 
// 4. GAMEPLAY & TIMING CONFIGURATION
const uint8_t SEQUENCE_LENGTH  = 3;
const uint8_t FINS_TO_WIN      = 6; // 6 fins per player = 36 LEDs
const unsigned long DEBOUNCE_MS      = 35;
const unsigned long LED_ECHO_MS      = 90;
const unsigned long VICTORY_ANIM_MS  = 6000;
const unsigned long VICTORY_FRAME_MS = 250; // 4Hz throttled publish

// Player Colors (R, G, B)
const uint8_t P1_COLOUR[3]     = {0, 220, 25}; // Green
const uint8_t P2_COLOUR[3]     = {220, 0, 250};  // Purple

#endif