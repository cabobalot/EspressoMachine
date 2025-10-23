
#include "pressure_sensor.h"
#include <Arduino.h>

unsigned long prevMs_ = 0;
// Sensor parameters
const int pressurePin = 36;
const float sensorMinVoltage = 0.5;
const float sensorMaxVoltage = 4.5;

const float pressureMin = 0.0;
const float mPaMax = 1.2;

// ESP parameters
const float maxVoltage = 3.3;
const float adcResolution = 4095.0; // 12 bit resolution (0-3.3V)

// Voltage divider parameters
const float voltageDividerRatio = 1.515;

// Averaging constant
static const uint8_t NUM_READINGS = 20;
static uint16_t ringBuf[NUM_READINGS];  // ~40B
static uint8_t  head = 0;               // 下一个要写入的位置
static uint8_t  count = 0;              // 当前已有样本数 (<= NUM_READINGS)
static uint32_t sumAdc = 0;
             // 运行和，防溢出用32位
static inline uint16_t readAveragedAdc() {
  uint16_t newv = (uint16_t)analogRead(pressurePin);
    //Serial.print("RawData: ");
    //Serial.println(newv);
  if (count < NUM_READINGS) {
    // 缓冲未满：直接累加
    sumAdc += newv;
    ringBuf[head++] = newv;
    count++;
    if (head == NUM_READINGS) head = 0;
  } else {
    // 缓冲已满：弹出最旧值，加入新值
    uint16_t oldv = ringBuf[head];
    sumAdc += newv;
    sumAdc -= oldv;
    ringBuf[head] = newv;
    head++;
    if (head == NUM_READINGS) head = 0;
  }

  // 计算当前平均（四舍五入）
  uint32_t denom = (count == 0) ? 1 : count;
  uint16_t avg = (uint16_t)((sumAdc + denom / 2) / denom);
  return avg;
}
int calculatePressure() {
  int i = 0;
  int rawTotalValue = 0;
  static int avgValue;
  const unsigned long now = millis();

  //update every 3ms
  if (now - prevMs_ >= 3) {
    prevMs_ = now;

    avgValue = readAveragedAdc();
  }

  // Convert ADC value to ESP pin voltage (0-3.3V)
  float measuredVoltage = (avgValue / adcResolution) * maxVoltage;
  
  // Calculate the actual sensor output voltage (accounting for voltage divider)
  float actualVoltage = measuredVoltage * voltageDividerRatio;
  
  // Scale the reading to actual pressure in MPa
  float pressureRange = mPaMax - pressureMin;
  float voltageRange = sensorMaxVoltage - sensorMinVoltage;
  
  float pressure = ((actualVoltage - sensorMinVoltage) * pressureRange / voltageRange) + pressureMin; // in mPa
  
  // Ensure pressure is within expected range (for sanity checking)
  pressure = constrain(pressure, pressureMin, mPaMax);

  // Conversion to bar and PSI
  float bar = pressure * 10;
  float psi = pressure * 145.038;
  
  // printAvgData(avgValue, psi, bar, measuredVoltage);
  return psi;
}

const int printToPlotter = 0; // 1 to plot, 0 to print metadata

// To print to Serial Monitor and Plotter
void printAvgData(float avgSensorValue, float avgPsi, float avgBar, float avgVoltage) {
  if (printToPlotter) {
    Serial.println(avgPsi); // fill with desired measurement
  }
  else {
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
