// Sensor parameters
const int pressurePin = 36;
const float sensorMinVoltage = 0.5;
const float sensorMaxVoltage = 4.5;

const float pressureMin = 0.0;
const float mPaMax = 1.2;

// ESP parameters
const float maxVoltage = 3.3;
const float adcResolution = 4095.0; // 12 bit resolution (0-3.3V)

// Voltage divder parameters
const float voltageDividerRatio = 1.515;

// Averaging constant
const int numReadings = 20;

int calculatePressure() {
  int i = 0;
  int rawTotalValue = 0;
  int avgValue = 0;
  unsigned long lastTime = 0;

  for (i; i < numReadings; i++) {
    if (millis() - lastTime > 5) {
      int sensorValue = analogRead(pressurePin);
      rawTotalValue += sensorValue;
      lastTime = millis();
    }
  }
  avgValue = rawTotalValue / numReadings;

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

