
#include "psm.hpp"

volatile bool zeroCross = false;

// IRAM_ATTR void zeroCrossISR() {
void zeroCrossISR() {
  zeroCross = true;
}

unsigned int psm::value = 0;
unsigned int psm::a = 0;
bool psm::skip = false;

byte psm::controlPin = 0;
byte psm::interruptPin = 0;

void psm::config(byte control, byte interrupt) {
	controlPin = control;
	interruptPin = interrupt;

	pinMode(psm::controlPin, OUTPUT);
	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), zeroCrossISR, RISING); // low-to-high transition = start of negative half-wave
}


void psm::debounceZeroCross() {
  static unsigned long firedTime;
  if (millis() < firedTime + 5) {
    zeroCross = false;
    return;
  }

  if (zeroCross) {
    firedTime = millis();
    calculateSkip();
    zeroCross = false;
  }
}

void psm::calculateSkip() {
  static bool wholeWaveFlag = false;

  if (wholeWaveFlag) {
    wholeWaveFlag = false;
    return;
  }
  else {
    wholeWaveFlag = true;

    a += value;

    if (a >= range) {
      a -= range; // >50% input means longer pulses
      skip = false;
    } else {
      skip = true;
    }

    if (a > range) {
      a = 0;
      skip = false;
    }

    // updateControl();
  }
  
}

void psm::updateControl() {
  if (skip) {
    digitalWrite(controlPin, LOW);
  } else {
    digitalWrite(controlPin, HIGH);
  }
}

void psm::setValue(unsigned int v) {
	a = 0;
	value = v;
  Serial.println(v);
}


/**
 * @brief call this very frequently
 * 
 */
void psm::update() {
	debounceZeroCross();
	updateControl();
}