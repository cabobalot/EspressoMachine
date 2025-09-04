#include <Arduino.h>
#include "tempControl.h"
#include "temp_sensor.h"

static float targetTemp = 0.0; 
static float currentTemp = 0.0;

TemperatureSensor tempSensor;

const byte valuePin = 32; // potentiometer input


void setup() {
	Serial.begin(115200);

	tempControl::init();

	// Set initial target temperature
	tempControl::setSetpoint(45.0);

	// Set initial temperature
	tempControl::setCurrentTemp(20.0);

}

void loop() {
    int readValue = analogRead(valuePin) / 32; // roughly 0-128 Celsius
    
	tempControl::setSetpoint(readValue); 
	// Serial.println(TemperatureSensor::getTemperature())
	
	static unsigned long lastTemp = 0;
  if (millis() - lastTemp > 500) {
    float t = tempSensor.readTemperature();
    // Serial.println(t);
		tempControl::setCurrentTemp(t);
    lastTemp = millis();
  };

   tempControl::update();
        
}
