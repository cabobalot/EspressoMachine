#include "temp_sensor.h"
#include "pins.h"


// Note: this library uses software SPI which is slightly not ideal and slow
MAX6675 TemperatureSensor::thermocouple = MAX6675(PIN_TEMP_SENS_SCK, PIN_TEMP_SENS_CS, PIN_TEMP_SENS_SO); // Initializes the temperature sensor object
float TemperatureSensor::temperature = 0;

/*
Handles retrieving the temperature from the sensor
*/
float TemperatureSensor::getTemperature() {
  static unsigned long lastReadTime = 0;
  if (millis() - lastReadTime > 500) { // Read from the sensor every 0.5 secs
    TemperatureSensor::temperature = TemperatureSensor::readTemperature();
    lastReadTime = millis();
  }
  return temperature;
}

/*
Function from thermocouple library to read temperature.
*/
float TemperatureSensor::readTemperature() {
  return thermocouple.readCelsius();
}
