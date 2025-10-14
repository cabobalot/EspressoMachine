#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <max6675.h>
#include <SPI.h>
#include <Wire.h>

// Pin assignments
extern const int Temp_Clk;
extern const int Temp_Cs;
extern const int Temp_So;

class TemperatureSensor {
public:
  TemperatureSensor();
  float readTemperature();
  
private:
  MAX6675 thermocouple;
};

#endif // TEMPERATURE_SENSOR_H