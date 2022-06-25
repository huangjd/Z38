#ifndef _ARKNIGHTS_UTIL_
#define _ARKNIGHTS_UTIL_

#include <cstddef>

template <typename T, bool GuardValue = true>
bool insert(const T& obj, T* arr, size_t N) {
  size_t i;
  for (i = 0; (GuardValue || i < N) && arr[i]; i++) {
  }
  if (i < N) {
    arr[i] = obj;
    return true; 
  }
  return false;
}

template <typename T, bool GuardValue = true>
bool remove(const T& obj, T* arr, size_t N) {
  size_t i, j;
  for (i = 0, j = 0; (GuardValue || i < N) && arr[i] ; i++, j++) {
    if (arr[i] == obj) {
      i++;
    }
    arr[j] = arr[i];  
  }
  bool removed = (i > j);
  if (!GuardValue && removed) {
    arr[j] = T();
  }
  return removed;
}

static void blink(int ms) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(ms);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(ms);
  digitalWrite(LED_BUILTIN, LOW);
}

static void blinkN(int ms, unsigned n) {
  for (unsigned i = 0; i < n; i++) {
    blink(ms);
  }
}

#endif
