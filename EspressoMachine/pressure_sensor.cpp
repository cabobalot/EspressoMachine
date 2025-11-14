#include <Arduino.h>
#include "pressure_sensor.h"
#include"pins.h"

// Sensor parameters
const int pressurePin = PIN_PRESSURE_SENSE;
const float sensorMinVoltage = 0.5;
const float sensorMaxVoltage = 4.5;

const float pressureMin = 0.0;
const float mPaMax = 1.2;

// ESP32 specific parameters
const float maxVoltage = 3.3;
const float adcResolution = 4095.0; // 12 bit resolution (0-3.3V)

// Voltage divider parameters
const float voltageDividerRatio = 1.515;

// Averaging constant
static const uint8_t NUM_READINGS = 20;
static uint16_t ringBuf[NUM_READINGS];  // ~40B
static uint8_t  head = 0;               // Next location to write to
static uint8_t  count = 0;              // Current number of samples (<= NUM_READINGS)
static uint32_t sumAdc = 0;             // Run and prevent overflow

/*
Averages the pressure readings coming in from the sensor
*/
static inline uint16_t readAveragedAdc() {
  uint16_t newv = (uint16_t)analogRead(pressurePin);
  if (count < NUM_READINGS) {
    // Buffer not full, accumulate directly
    sumAdc += newv;
    ringBuf[head++] = newv;
    count++;
    if (head == NUM_READINGS) head = 0;
  } else {
    // Buffer full, pop the old value, add the new one in
    uint16_t oldv = ringBuf[head];
    sumAdc += newv;
    sumAdc -= oldv;
    ringBuf[head] = newv;
    head++;
    if (head == NUM_READINGS) head = 0;
  }

  // Calculate the current average (rounded)
  uint32_t denom = (count == 0) ? 1 : count;
  uint16_t avg = (uint16_t)((sumAdc + denom / 2) / denom);
  return avg;
}

/*
Handles the calculations to take voltage sensor readings into a pressure format.
*/
int calculatePressure() {
  int i = 0;
  int rawTotalValue = 0;
  static int avgValue;
  static float bar = 0;
  static float psi = 0;
  static unsigned long prevMs_ = 0;

  const unsigned long now = millis();

  // update every 3ms
  if (now - prevMs_ >= 3) {
    prevMs_ = now;

    avgValue = readAveragedAdc();
    // Convert ADC value to ESP pin voltage (0-3.3V)
    float measuredVoltage = (avgValue / adcResolution) * maxVoltage;
    
    // Calculate the actual sensor output voltage (accounting for voltage divider)
    float actualVoltage = measuredVoltage * voltageDividerRatio;
    
    // Scale the reading to actual pressure in MPa
    float pressureRange = mPaMax - pressureMin;
    float voltageRange = sensorMaxVoltage - sensorMinVoltage;
    
    float pressure = ((actualVoltage - sensorMinVoltage) * pressureRange / voltageRange) + pressureMin; // in mPa
    
    // Ensure pressure is within expected range (sanity check)
    pressure = constrain(pressure, pressureMin, mPaMax);

    // Conversion to bar and PSI
    bar = pressure * 10;
    psi = pressure * 145.038;
  }
  return psi; // PSI is default, but mPa or bar is valid.
}

const int printToPlotter = 0; // 1 to plot, 0 to print metadata

/*
Prints data either to a serial plotter or directly into the console
*/
void printAvgData(float avgSensorValue, float avgPsi, float avgBar, float avgVoltage) {
  if (printToPlotter) {
    Serial.println(avgPsi); // fill with desired measurement
  }
  else { // Print to console
    Serial.print("Raw ADC: ");
    Serial.println(avgSensorValue);
    Serial.print("Measured Voltage: ");
    Serial.println(avgVoltage, 3);
    Serial.print("Bar: ");
    Serial.println(avgBar, 3);
    Serial.print("PSI: ");
    Serial.println(avgPsi, 3);
  }
}
