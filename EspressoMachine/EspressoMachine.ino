#include "pins.h"

#include "psm.hpp"
#include "tempControl.h"
#include "pressureControl.h"


void setup() {
  psm::config(PIN_DIMMER_CONTROL, PIN_DIMMER_ZERO_CROSS);
  Serial.begin(115200);

  tempControl::init();
}


void loop() {
	// Update temperature and setpoint
	UI.setDisplayTemp(tempControl::getTemperature());
	tempControl::setTemperature(UI.getTempSetpoint());

	// Update pressure and setpoint
	UI.setDisplayPresure(pressureControl::getPressure());
	pressureControl::setPressure(UI.getPressureSetpoint());
	pressureControl::setMaxPressure(UI.getMaxPressureSetpoint());
	pressureControl::setPreinfuseTime(UI.getPreinfuseTime());

	// update everything
	tempControl::update();
	pressureControl::update();
	UI.update();
	PSM::update();

}




