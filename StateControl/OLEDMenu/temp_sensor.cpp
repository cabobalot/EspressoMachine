#include "temp_sensor.h"

// Pin assignments
const int Temp_Clk = 18;
const int Temp_Cs = 5;
const int Temp_So = 19;

//TODO are we ok that this library uses software SPI??
MAX6675 TemperatureSensor::thermocouple = MAX6675(Temp_Clk, Temp_Cs, Temp_So);
float TemperatureSensor::temperature = 0;

void TemperatureSensor::init() {
  // thermocouple = MAX6675(Temp_Clk, Temp_Cs, Temp_So);
  // temperature = 0;
}

float TemperatureSensor::getTemperature() {
  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime > 500) {
    TemperatureSensor::temperature = TemperatureSensor::readTemperature();
    lastReadTime = millis();
  }

  return temperature;
}

float TemperatureSensor::readTemperature() {
  return thermocouple.readCelsius();
}