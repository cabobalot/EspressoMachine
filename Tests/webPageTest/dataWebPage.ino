#include "dataWebPage.h"

unsigned long testPrevMillis = 0;
const unsigned long testInterval = 1000;  // Change test temp every second
int toggle = 1;

void setup() {
  dataWebPage::init();

}

void loop() {
  unsigned long currMillis = millis();

  if (currMillis - testPrevMillis >= testInterval) {
    testPrevMillis = currMillis;
    toggle = !toggle;
  }
  
  if (toggle) {
    dataWebPage::update(10.0, 20.0, 44.5, 52.1);
  }
  else {
    dataWebPage::update(91.2, 67.9, 12.1, 86.85);
  }

}
