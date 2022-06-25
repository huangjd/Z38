#include "HackSD.h"

#include <SPI.h>
#include <FS.h>
#include <Audio.h>
#include <Wire.h>
#include <SerialFlash.h>

#include "Util.h"

AudioPlayMemory          playMem;
AudioPlaySdWav           playSdWav;
AudioMixer4              mixer;
#ifdef USE_PWM
AudioOutputPWM           audioOutput;
#else
AudioOutputAnalog        audioOutput;
#endif 
AudioConnection          patchCord1(playMem, 0, mixer, 0);
AudioConnection          patchCord2(playSdWav, 0, mixer, 1);
AudioConnection          patchCord3(mixer, audioOutput);

void initAudio() {
  AudioMemory(16);
}

void enableAudio(const char *file, float level) {
  if (file) {
    mixer.gain(0, 0);
    mixer.gain(1, level);
    playSdWav.play(file);
  }
}

void enableAudio(const char *file) {
  if (file && file[0]) {
    mixer.gain(0, 0);
    mixer.gain(1, 10); 
    playSdWav.play(file);
  } else {
    mixer.gain(0, 10);
    mixer.gain(1, 0);
    extern const unsigned int KokodayoData[31201];
    playMem.play(KokodayoData);
  }
}

void disableAudio() {
  playSdWav.stop();
  playMem.stop();
}
