#ifndef TEMPCONTROL_H
#define TEMPCONTROL_H

#include "temp_sensor.h"

class tempControl {
private:

public:

	static void init();

	static void setSetpoint(float temp);

	static void setCurrentTemp(float temp);
	
	static float getSetpoint();

	static void setTemperature(float tempSetpoint);

	static float getTemperature();

	static void update();

};

#endif

