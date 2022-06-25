#include "HackSD.h"

#include "PowerManagement.h"
#include "Lights.h"

#include <cstdlib>
#include <cstring>

static constexpr int MY_SD_CS_PIN = 10;
static constexpr int LED_PIN = 13;
static constexpr unsigned int MISO_PARALLEL_PIN_COUNT = 2;
static constexpr int MISO_PARALLEL_PINS[MISO_PARALLEL_PIN_COUNT] = {14, 15};

static char buf[260];

PowerManagement pm;

static constexpr int GACHA_EFFECT_DELAY_MS = 1000;
static constexpr int AUDIO_EFFECT_DELAY_MS = 4000;
static constexpr int LIGHT_EFFECT_DELAY_MS = 42;

static bool isMaster = false;

static constexpr unsigned int GACHA_TIERS = 4;
static int gachaDistribution[GACHA_TIERS] = {40, 70, 90, 100};

static bool awake = false;

#define SPI_SPEED SD_SCK_MHZ(4)
void initSd() {
  bool b = SD.sdfs.begin(MY_SD_CS_PIN, SPI_SPEED);  
  Serial.print("SD CARD IS ");
  Serial.println(b);
  if (b) {
    File f = SD.open("config.txt");
    Serial.print("Config file is ");
    Serial.println((bool) f);
    if (f) {
      f.read(buf, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = 0;
      f.close();
      char* s = strtok(buf, "\r\n");
      char* next = strtok(NULL, "\r\n");
      s = strtok(s, ", ");
      for (unsigned int i = 0; i < GACHA_TIERS; i++) {
        if (s != NULL) {
          gachaDistribution[i] = atoi(s);
          s = strtok(NULL, ", ");
        }
      }
    }
  }
  buf[0] = '/';
  buf[2] = 0;
  for (unsigned int i = 0; i < GACHA_TIERS; i++) {
    buf[1] = i + '0';
    Serial.print("Found following files in folder ");
    Serial.println(i);
    SD.sdfs.ls(buf);
  }
  
  Serial.print("Distribution is ");
  for (unsigned int i = 0; i < GACHA_TIERS - 1; i++) {
    Serial.print(gachaDistribution[i]);
    Serial.print(", ");
  }
  Serial.println(gachaDistribution[GACHA_TIERS - 1]);
}

const char* getAudioPath(int tier) {
  buf[0] = '/';
  buf[1] = tier + '0';
  buf[2] = '/';
  buf[3] = 0;
  int count = 0;
  if (!SD.sdfs.begin(MY_SD_CS_PIN, SPI_SPEED)) {
    Serial.println("Can't open sdfs again");
    buf[0] = 0;
    return nullptr;
  }
  File root = SD.open(buf);
  if (!root) {
    Serial.print("Can't open ");
    Serial.println(buf);
    buf[0] = 0;
    return nullptr;
  }
  while (File f = root.openNextFile()) {
    count++;
    if (random(count) == 0) {
      strcpy(buf + 3, f.name());
    }
    f.close();
  }
  if (SD.exists(buf)) {
    return buf;
  }
  Serial.print("Can't find ");
  Serial.println(buf);
  buf[0] = 0;
  return nullptr;
}

void onAwake() {
  Serial.flush();
  Serial.println("Activated");
  Serial.println(millis());
  Serial.flush();
  awake = true;
  if (isMaster) {
    int gacha = random(gachaDistribution[GACHA_TIERS - 1]);
    int tier = 0;
    for (unsigned int i = 0; i < GACHA_TIERS - 1; i++) {
      if (gacha > gachaDistribution[i]) {
        tier++;
      } else {
        break;
      }
    }
    Serial.print("Gacha tier is ");
    Serial.println(tier);
    const char *path = getAudioPath(tier);
    if (path) {
      Serial.println(path);
    } else {
      tier = 0;
      Serial.println("Can't find audio file, using default");
    }
    Serial.flush();
    for (unsigned int i = 0; i < MISO_PARALLEL_PIN_COUNT; i++) {
      pinMode(MISO_PARALLEL_PINS[i], OUTPUT);
      digitalWrite(MISO_PARALLEL_PINS[i], tier & (1 << i) ? HIGH : LOW);
    }
    delay(GACHA_EFFECT_DELAY_MS);
    enableAudio("/pull.wav", tier == GACHA_TIERS - 1 ? 3 : 1);
  } else {
    for (unsigned int i = 0; i < MISO_PARALLEL_PIN_COUNT; i++) {
      pinMode(MISO_PARALLEL_PINS[i], INPUT);
    }
  }
  Serial.flush();
}

void onSleep() {
  awake = false;
  if (isMaster) {
    disableAudio();
    SD.sdfs.end();
  } else {
    disableLights();
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  
  pinMode(MASTER_ID_PIN, INPUT_PULLUP);
  delay(1);
  if (digitalRead(MASTER_ID_PIN) == LOW) {
    isMaster = true;
  }
  
  if (isMaster) {
    for (unsigned int i = 0; i < MISO_PARALLEL_PIN_COUNT; i++) {
      pinMode(MISO_PARALLEL_PINS[i], OUTPUT);
      digitalWrite(MISO_PARALLEL_PINS[i], LOW);
    }
    initSd();
    initAudio();
  } else {
    for (unsigned int i = 0; i < MISO_PARALLEL_PIN_COUNT; i++) {
      pinMode(MISO_PARALLEL_PINS[i], INPUT);
    }
    initLights();
  }

  pm.registerAwakeCallback(onAwake);
  pm.registerSleepCallback(onSleep);

  Serial.println("Init completed");
  Serial.flush();
}

void loop() {
  if (isMaster) {
    pm.loop();  
  } else {
    pm.loop(Snooze.sleep);
  }
  
  if (isMaster) {
    if (awake && millis() - pm.getLastAwake() >= AUDIO_EFFECT_DELAY_MS) {
      enableAudio(buf);
      awake = false;
    }
  } else {
    static int lastTier = -1;
    int tier = 0;
    for (unsigned int i = 0; i < MISO_PARALLEL_PIN_COUNT; i++) {
      auto value = digitalRead(MISO_PARALLEL_PINS[i]);
      tier |= ((value == HIGH) << i);
    }
    if (tier != lastTier) {
      Serial.println(tier);
      lastTier = tier;
      updateLights(tier);
    }
    enableLights();
  }
  delay(LIGHT_EFFECT_DELAY_MS);
}
