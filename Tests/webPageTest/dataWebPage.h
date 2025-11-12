#ifndef DATAWEBPAGE_H
#define DATAWEBPAGE_H

class dataWebPage {
private:

public:

  enum MachineState {
    IDLE_STATE,
    BREW_STATE,
    STEAM_STATE,
    HOT_WATER_STATE
  };

	static void init();

  static void update(float temp, float pressure, float tempSetPoint, float pressureSetPoint, MachineState state);

};

#endif