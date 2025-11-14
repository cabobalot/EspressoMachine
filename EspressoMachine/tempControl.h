#ifndef TEMPCONTROL_H
#define TEMPCONTROL_H

#include "temp_sensor.h"

/*
Handles and regulates the temperature of the machine.
*/
class tempControl {
private:

public:

	// Initializes all parameters for temperature control.
	static void init();

	// Sets the setpoint of the machine.
	static void setSetpoint(float temp);

	// Sets the current temperature of the machine.
	static void setCurrentTemp(float temp);
	
	// Gets the current setpoint
	static float getSetpoint();

	// Sets the current temperature
	static void setTemperature(float tempSetpoint);

	// Gets the current temperature
	static float getTemperature();

	// Controls the output of the relay
	static void update();

};

#endif

