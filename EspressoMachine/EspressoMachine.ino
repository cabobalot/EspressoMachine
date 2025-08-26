#include "pins.h"

#include "psm.hpp"
#include "tempControl.h"
#include "pressureControl.h"


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


	tempControl::update();
	pressureControl::update();

	UI.setDisplayTemp(tempControl.getTemp());
	UI.update();

	PSM::update();


	
	// unsigned long time = millis();
	// static unsigned long lastTime;
	// if (time > lastTime + updateDelay) {
	// 	lastTime = time;

	// 	updateValue();
	// }

	

}



