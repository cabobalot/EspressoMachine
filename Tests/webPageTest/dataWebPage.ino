#include "dataWebPage.h"

unsigned long testPrevMillis = 0;
const unsigned long testInterval = 5000;  // Change test temp every 5 second
int state;
int toggle = 0;

dataWebPage::MachineState brew = dataWebPage::MachineState::BREW_STATE;

dataWebPage::MachineState idle = dataWebPage::MachineState::IDLE_STATE;

dataWebPage::MachineState steam = dataWebPage::MachineState::STEAM_STATE;

dataWebPage::MachineState hotWater = dataWebPage::MachineState::HOT_WATER_STATE;

void setup() {
  dataWebPage::init();
  state = 1;

}
  
void loop() {
  unsigned long currMillis = millis();

  if (currMillis - testPrevMillis >= testInterval) {
    testPrevMillis = currMillis;
    switch (state){
      case 1:
        state = 2;
      break;
      case 2:
        state = 3;
      break;
      case 3:
        state = 4;
      break;
      case 4:
        state = 1;
      break;
      default: state = 1;
    }
    // toggle = !toggle;
    // switch (state) {
    //   case 1:
    //     dataWebPage::update(10.0, 20.0, 44.5, 52.1, brew);
    //     Serial.println("Brew!");  
    //     state = 2;      
    //     break;
    //   case 2:
    //     dataWebPage::update(100, 50, 100, 50, idle);
    //     Serial.println("Idle!");
    //     state = 1;
    //     break;
      // case 3:
      //   dataWebPage::update(50, 20, 20, 40, steam);
      //   Serial.println("Steam!");
      //   state = 4;
      //   break;
      // case 4:
      //   dataWebPage::update(5, 90, 5, 90, hotWater);
      //   Serial.println("HotWater!");
      //   state = 1;
      //   break;
      // default: Serial.println("broken...");
    // }
  }
    // if (toggle) {
    //   dataWebPage::update(10.0, 20.0, 44.5, 52.1, steam);
    // }
    // else {
    //   dataWebPage::update(100, 50, 100, 50, hotWater);
    // }
  if (state == 1) {
    dataWebPage::update(100, 14, 12, 48, brew);
  }
  else if (state == 2) {
    dataWebPage::update(101, 24, 41, 52, steam);
  }
  else if (state == 3) {
    dataWebPage::update(100, 22, 49, 5, hotWater);
  }
  else {
    dataWebPage::update(10, 20.13, 44.5, 52.1, idle);
  }
}
  
