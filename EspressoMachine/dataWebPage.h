#ifndef DATAWEBPAGE_H
#define DATAWEBPAGE_H

#include "EspressoMachine.h"

class dataWebPage {
private:

public:

	static void init();

  static void update(float temp, float pressure, float tempSetPoint, float pressureSetPoint, MachineState state);

};

#endif