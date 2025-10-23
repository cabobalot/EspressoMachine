#include "temp_sensor.h"
#include "pins.h"

// Pin assignments
const int Temp_Clk = PIN_TEMP_SENS_SCK;
const int Temp_Cs = PIN_TEMP_SENS_CS ;
const int Temp_So = PIN_TEMP_SENS_SO;

TemperatureSensor::TemperatureSensor()

  : thermocouple(Temp_Clk, Temp_Cs, Temp_So) {}

float TemperatureSensor::readTemperature() {
  return thermocouple.readCelsius();
}