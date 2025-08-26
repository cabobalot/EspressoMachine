#ifndef PRESSURECONTROL_H
#define PRESSURECONTROL_H

class pressureControl {
private:

public:
	static void setPressure(int pressureSetpoint);

	static float getPressure();

	static void update();

	static void startBrew(byte mode);

	static void setMaxPressure(int pressure);

	static void setPreinfuseTime(int preinfuseTime);

}

#endif
