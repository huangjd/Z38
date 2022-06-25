#ifndef _ARKNIGHTS_LIGHTS_H_
#define _ARKNIGHTS_LIGHTS_H_
#include <FastLED.h>

static constexpr int LED_CONTROL_PIN = 8;

static constexpr unsigned NUM_LED = 20;
static constexpr unsigned MAX_COLOR_PROFILE = 4;
static constexpr unsigned MAX_LIGHT = 5;

static int colorProfile = 0;

static CRGB ColorProfiles[MAX_COLOR_PROFILE] = {
  CRGB::White,
  CRGB(0x2e2ef0),
  CRGB(0xc78508),
  CRGB(0xe06502),
};

static const unsigned MinimumIntensity[MAX_COLOR_PROFILE] = {
  8,
  16,
  32,
  32,
};

uint8_t padding[16];
uint8_t intensity[MAX_LIGHT + 1][NUM_LED];
uint8_t padding2[16];
CRGB leds[NUM_LED];

void currentLimit() {
  
}

void sendLed() {
  currentLimit();
  for (unsigned i = 0; i < NUM_LED; i++) {
    CRGB color = ColorProfiles[colorProfile];
    uint8_t ii = 0;
    for (unsigned j = 0; j < MAX_LIGHT; j++) {
      ii = max(ii, intensity[j][i]);
    }
    ii = max(ii, MinimumIntensity[colorProfile]);
    Serial.print(ii);
    Serial.print(' ');
    leds[i] = color.nscale8(ii);
  }

  Serial.println();
  FastLED.show();
}

struct LightProfile {
  int step;
  int stepSize;
  int maxIntensity;
  int position;

  LightProfile() : step(129), stepSize(1) {}
  
  LightProfile(int stepSize, int maxIntensity, int position) : 
    step(-128), stepSize(stepSize), maxIntensity(maxIntensity), position(position) {
  }

  int getIntensity() {
    return maxIntensity - maxIntensity * abs(step) / 128;
  }
} lightProfiles[MAX_LIGHT];

void updateLight(unsigned i) {
  LightProfile *lp = &lightProfiles[i];
  lp->step += lp->stepSize;
  if (lp->step > 128) {
    int x = random(1 << 16);
    int newStepSize = 1 + (x & 15);
    x >>= 4;
    int newMaxIntensity = 128 + (x & 127);
    x >>= 7;
    int newPosition = 2 + (x & 15);
    *lp = LightProfile(newStepSize, newMaxIntensity, newPosition);
    for (unsigned j = 0; j < NUM_LED; j++) {
      intensity[i][j] = 0;
    }
  }
  int iRef = lp->getIntensity();
  int x = iRef * (lp->step) / 256;
  if (lp->step < 0) {
    intensity[i][lp->position - 2] = -x;
    intensity[i][lp->position - 1] = iRef / 2 - x;
    intensity[i][lp->position] = iRef + x;
    intensity[i][lp->position + 1] = iRef / 2 + x;
    intensity[i][lp->position + 2] = 0;
  } else {
    intensity[i][lp->position - 2] = 0;
    intensity[i][lp->position - 1] = iRef / 2 - x;
    intensity[i][lp->position] = iRef - x;
    intensity[i][lp->position + 1] = iRef / 2 + x;
    intensity[i][lp->position + 2] = x;
  }
}

void initLights() {
  FastLED.addLeds<NEOPIXEL, LED_CONTROL_PIN>(leds, NUM_LED);
  for (unsigned j : {0, 4, 8, 11, 15, 19}) {
    intensity[MAX_LIGHT][j] = 16;
  }
}

void updateLights(int tier) {
  if (tier >= MAX_COLOR_PROFILE) {
    tier = MAX_COLOR_PROFILE - 1;
  }
  colorProfile = tier;
}

void enableLights() {
  pinMode(LED_CONTROL_PIN, OUTPUT);
  for (unsigned i = 0; i < MAX_LIGHT; i++) {
    updateLight(i);
  }
  sendLed();
}

void disableLights() {
  pinMode(LED_CONTROL_PIN, OUTPUT);
  for (unsigned i = 0; i < NUM_LED; i++) {
    leds[i] = 0;
  }
  FastLED.show();
}

#endif
