#ifndef PSM_H
#define PSM_H

#include "psm.h"

volatile bool zeroCross = false;

IRAM_ATTR static void zeroCrossISR() {
  zeroCross = true;
}

class psm {
private:
	static byte controlPin;
	static byte interruptPin;

	static const unsigned int range = 127;
	static const unsigned int valueFactor = 4; // precalculated constant 1024 [10 bit ADC resolution] / 128 [range + 1] / 2

	static unsigned int value;
	
	static unsigned int a;
	static bool skip;

	static void debounceZeroCross();
	static void calculateSkip();
	static void updateControl();

public:
	static void config(byte control, byte interrupt);

	static void setValue(unsigned int v);

	static void update();
};

unsigned int psm::value = 0;
unsigned int psm::a = 0;
bool psm::skip = false;

void psm::config(byte control, byte interrupt) {
	psm::controlPin = control;
	psm::interruptPin = interrupt;

	pinMode(controlPin, OUTPUT);
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
      // a -= range;
      a = 0;
      skip = false;
    } else {
      skip = true;
    }

    // if (a > range) {
    //   a = 0;
    //   skip = false;
    // }

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
}


/**
 * @brief call this very frequently
 * 
 */
void psm::update() {
	debounceZeroCross();
	updateControl();
}





#endif
