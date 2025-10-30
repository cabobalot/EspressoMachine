#include "temp_sensor.h"
#include "pins.h"


// note this library uses software SPI which is slightly not ideal and slow
MAX6675 TemperatureSensor::thermocouple = MAX6675(PIN_TEMP_SENS_SCK, PIN_TEMP_SENS_CS, PIN_TEMP_SENS_SO);
float TemperatureSensor::temperature = 0;

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
