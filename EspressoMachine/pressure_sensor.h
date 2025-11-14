#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

// Sensor parameters
extern const int pressurePin;
extern const float sensorMinVoltage;
extern const float sensorMaxVoltage;

extern const float pressureMin;
extern const float mPaMax;

// ESP parameters
extern const float maxVoltage;
extern const float adcResolution; // 12 bit resolution (0-3.3V)

// Voltage divider parameters
extern const float voltageDividerRatio;

// Averaging constant
extern const int numReadings;

/*
Handles the calculations to read from the sensor
*/
int calculatePressure();

/*
Debugging code to check if data received is correct
*/
void printAvgData(float avgSensorValue, float avgPsi, float avgBar, float avgVoltage);

#endif // PRESSURE_SENSOR_H