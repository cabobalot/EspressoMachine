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
  /*
  Gets the temperature from the sensor
  */
  static float getTemperature();

private:
  /*
  Initializes the object
  */
  static MAX6675 thermocouple;

  /*
  Reads the temperature from the thermocouple object
  */
  static float readTemperature();

  /*
  Holds current temperature
  */
  static float temperature;
};

#endif // TEMPERATURE_SENSOR_H