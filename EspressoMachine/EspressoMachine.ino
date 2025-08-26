#include "psm.hpp"
#include "pins.h"

unsigned long updateDelay = 100;
unsigned long lastUpdateTime = 0;

unsigned int value = 0;

void setup() {
  psm::config(PIN_DIMMER_CONTROL, PIN_DIMMER_ZERO_CROSS);
  Serial.begin(115200);
}

void updateValue() {
  psm::setValue(64);
}

void 


void loop() {
  unsigned long time = millis();
  static unsigned long lastTime;
  if (time > lastTime + updateDelay) {
    lastTime = time;

    updateValue();
  }
  
  psm::update();

}



