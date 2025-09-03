#include <Arduino.h>
#include "tempControl.hpp"
#include "temp_sensor.hpp"

static float targetTemp = 0.0; 
static float currentTemp = 0.0;

const byte valuePin = 32; // potentiometer input



void setup() {
	Serial.begin(115200);


	tempControl::init();
	TemperatureSensor::init();

	// Set initial target temperature
	tempControl::setSetpoint(45.0);

	// Set initial temperature
	tempControl::setCurrentTemp(20.0);

}

void loop() {
    int readValue = analogRead(valuePin) / 8; // roughly 0-128 Celsius
    
	tempControl::setSetpoint(readValue);
	tempControl::setCurrentTemp(TemperatureSensor::getTemperature());

    tempControl::update();
        
}
