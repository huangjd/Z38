#ifndef _ARKNIGHTS_POWER_MANAGEMENT_
#define _ARKNIGHTS_POWER_MANAGEMENT_

#include <Snooze.h>

#include "Util.h"

static constexpr int CHIP_ENABLE_PIN = 2;
static constexpr int CASCADE_PIN = 5;
static constexpr int LOAD_ENABLE_PIN = 6;
static constexpr int MASTER_ID_PIN = 7;

static constexpr int SERIAL_FLUSH_WAIT_MS = 500;

static SnoozeDigital digital;
static SnoozeTimer timer;
static SnoozeBlock config(timer, digital);

class PowerManagement {
  static constexpr int MAX_CALLBACK = 4;

  static constexpr unsigned long DEBOUNCE_TIME_MS = 1000;

  bool awake;
  unsigned long lastAwake;
  
  void (*awakeCallbacks[MAX_CALLBACK + 1])();
  void (*sleepCallbacks[MAX_CALLBACK + 1])();

  static void onAwake() {
    Serial.begin(9600);
    randomSeed(millis());
    pinMode(CASCADE_PIN, OUTPUT);
    digitalWrite(CASCADE_PIN, LOW);
    pinMode(LOAD_ENABLE_PIN, OUTPUT);
    digitalWrite(LOAD_ENABLE_PIN, HIGH);
  }

  static void onSleep() {
    Serial.flush();
    pinMode(CASCADE_PIN, OUTPUT);
    digitalWrite(CASCADE_PIN, HIGH);
    pinMode(LOAD_ENABLE_PIN, OUTPUT);
    digitalWrite(LOAD_ENABLE_PIN, LOW);
  }

public:
  PowerManagement() : awake(true), lastAwake(millis()), awakeCallbacks{0}, sleepCallbacks{0} {
    registerAwakeCallback(onAwake);
    registerSleepCallback(onSleep);
    pinMode(CHIP_ENABLE_PIN, INPUT_PULLUP);
  }

  void loop(int (*sleepMode)(SNOOZE_BLOCK) = Snooze.hibernate) {
    unsigned long now = millis();
    if (now - lastAwake > DEBOUNCE_TIME_MS && digitalRead(CHIP_ENABLE_PIN) == HIGH) {
      for (unsigned i = 0; sleepCallbacks[i]; i++) {
        sleepCallbacks[i]();
      }
      pinMode(CHIP_ENABLE_PIN, OUTPUT);
      digitalWrite(CHIP_ENABLE_PIN, HIGH);
      digital.pinMode(CHIP_ENABLE_PIN, INPUT_PULLUP, FALLING);
      if (digitalRead(CHIP_ENABLE_PIN) == HIGH) {
        delay(SERIAL_FLUSH_WAIT_MS);
        sleepMode(config);
        delay(SERIAL_FLUSH_WAIT_MS);
      }
      detachInterrupt(digitalPinToInterrupt(CHIP_ENABLE_PIN));
      awake = true;
      pinMode(CHIP_ENABLE_PIN, INPUT_PULLUP);
      lastAwake = millis();
      for (unsigned i = 0; awakeCallbacks[i]; i++) {
        awakeCallbacks[i]();
      }
    }
  }

  unsigned long getLastAwake() const {
    return lastAwake;
  }

  bool registerAwakeCallback(void (*f)()) {
    return insert(f, awakeCallbacks, MAX_CALLBACK);
  }

  bool registerSleepCallback(void (*f)()) {
    return insert(f, sleepCallbacks, MAX_CALLBACK);
  }

  bool unregisterAwakeCallback(void (*f)()) {
    return remove(f, awakeCallbacks, MAX_CALLBACK);
  }

  bool unregisterSleepCallback(void (*f)()) {
    return remove(f, sleepCallbacks, MAX_CALLBACK);
  }
};

#endif
