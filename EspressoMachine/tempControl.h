#ifndef TEMPCONTROL_H
#define TEMPCONTROL_H

class tempControl {
private:

public:
	static void setTemperature(float tempSetpoint);

	static float getTemperature();

	static void update();

}

#endif
