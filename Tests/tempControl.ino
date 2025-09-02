#include <Arduino.h>
#include "tempControl.h"

static float targetTemp = 0.0; 
static float currentTemp = 0.0;


//-------------------Test Code------------------------

float testTemps[] = {100.0, 25.0, 50.0, 10.0, 40.0, 80.0, 50.0, 5.0, 70.0, 45.0, 90.0};
float simTemps[] =  {10.0, 20.0, 30.0, 40.0, 41.0, 42.0, 55.0, 60.0, 20.0, 10.0, 45.0};
int testIndex = 0;
const int numEntries = sizeof(testTemps) / sizeof(testTemps[0]);

unsigned long testPrevMillis = 0;
const unsigned long testInterval = 5000;  // Change test temp every 5 seconds

//----------------------------------------------------

void setup() {
  Serial.begin(115200);
  
  tempControl::init(); 

  // Set initial target temperature
  tempControl::setSetpoint(45.0);  
    
  // Set initial temperature
  tempControl::setCurrentTemp(20.0); 

}

void loop() {
    // Simulate temperature readings for testing
    unsigned long currMillis = millis();
    
    // Walk through temps
    if (currMillis - testPrevMillis >= testInterval && testIndex < numEntries) {
        testPrevMillis = currMillis;
        
        tempControl::setCurrentTemp(simTemps[testIndex++]);
        float simulatedTemp = testTemps[testIndex++];
        // tempControl::setSetpoint(simulatedTemp);
    }
    
    tempControl::update();
        
}
