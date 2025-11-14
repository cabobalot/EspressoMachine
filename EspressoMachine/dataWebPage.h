#ifndef DATAWEBPAGE_H
#define DATAWEBPAGE_H

#include "EspressoMachine.h"

/*
Handles the local webpage hosting and display for brew parameters.
Graphs brew parameters when in brew and steam modes.
*/
class dataWebPage {
private:

public:

  /*
  Initializes the local accesspoint and hosts the webpage.
  */
	static void init();

  /*
  Updates the information on the webpage
  */
  static void update(float temp, float pressure, float tempSetPoint, float pressureSetPoint, MachineState state);

};

#endif // DATAWEBPAGE_H