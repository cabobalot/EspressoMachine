#include "temp_sensor.h"

// Pin assignments
const int Temp_Clk = 5;
const int Temp_Cs = 23;
const int Temp_So = 19;

TemperatureSensor::TemperatureSensor()

  : thermocouple(Temp_Clk, Temp_Cs, Temp_So) {}

float TemperatureSensor::readTemperature() {
  return thermocouple.readCelsius();
}