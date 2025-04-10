#include "psm.h"

const byte valuePin = 32; // potentiometer input
const byte controlPin = 33; // triac output
const byte interruptPin = 25; // phase sensing


unsigned long updateDelay = 100;
unsigned long lastUpdateTime = 0;

unsigned int value = 0;

psm dimmer(controlPin, interruptPin);

void setup() {
  Serial.begin(115200);
}

void updateValue() {
  int readValue = analogRead(valuePin) / 4;
  Serial.print(readValue);
  int oldValue = value * valueFactor * 2;

  if (readValue > (oldValue + valueFactor * 3) || (readValue + valueFactor) < oldValue) { // add some hysteresis to filter out noise
    value = readValue / valueFactor / 2;
    dimmer.setValue(value);
  }
}



void loop() {
  unsigned long time = millis();
  static unsigned long lastTime;
  if (time > lastTime + updateDelay) {
    lastTime = time;

    updateValue();

    unsigned long dif = time - lastUpdateTime;
    unsigned long intPerSec = (intCount * 1000) / dif;
    Serial.printf(" int/sec:%i dif:%i\n", intPerSec, dif);
    
    
    // Serial.flush();

    // intCount = 0;
    // delay(20);
    // Serial.printf("after 20ms, intCount:%i", intCount);
    // Serial.println();
    

    intCount = 0;
    lastUpdateTime = time;
  }
  
  dimmer.update();

}



