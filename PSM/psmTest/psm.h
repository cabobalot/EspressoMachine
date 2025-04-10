#ifndef PSM_H
#define PSM_H

#include <arduino>

class psm {
private:
	byte controlPin;
	byte interruptPin;

	const unsigned int range = 127;
	const unsigned int valueFactor = 4; // precalculated constant 1024 [10 bit ADC resolution] / 128 [range + 1] / 2

	volatile bool zeroCross = false;

	unsigned int value = 0;
	
	unsigned int a = 0;
	bool skip = false;

	void zeroCrossISR();
	void debounceZeroCross();
	void calculateSkip();
	void updateControl();

public:
	psm(byte controlPin, interruptPin);

	void setValue(unsigned int v);

	void update();
};

psm::psm(byte controlPin, interruptPin) {
	this.controlPin = controlPin;
	this.interruptPin = interruptPin;

	pinMode(controlPin, OUTPUT);
	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), zeroCrossISR, RISING); // low-to-high transition = start of negative half-wave
}



IRAM_ATTR void psm::zeroCrossISR() {
  zeroCross = true;
}

void psm::debounceZeroCross() {
  static unsigned long firedTime;
  if (millis() < firedTime + 5) {
    zeroCross = false;
    return;
  }

  if (zeroCross) {
    firedTime = millis();
    intCount++;
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

void psm::setValue(int v) {
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
