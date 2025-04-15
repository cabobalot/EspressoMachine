#ifndef PSM_H
#define PSM_H

#include <Arduino.h>

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




#endif
