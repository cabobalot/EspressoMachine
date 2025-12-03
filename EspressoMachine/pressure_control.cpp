#include "pressure_control.h"

PressureControl::PressureControl(double kp, double ki, double kd)
: pid_(&currentPsi_, &outputPct_, &setpointPsi_, kp, ki, kd, DIRECT) {
  // Output range limited to unidirectional (only "pressurize/heat")
  pid_.SetOutputLimits(0, 100);         // 0..100 %
  pid_.SetSampleTime(kCtrlMs);          // Consistent with scheduling period
}

void PressureControl::init(uint8_t controlPin, uint8_t zeroCrossPin) {
  // Initialize PSM (zero-crossing full-cycle skip power control)
  psm::config(controlPin, zeroCrossPin);

  // Start PID automatic mode
  pid_.SetMode(AUTOMATIC);

  // Clear initial output
  outputPct_ = 0;
  psm::setValue(0);
}


// setting mode to manual and automatic should prevent integral windup
void PressureControl::setSetpoint(double psi) {
  isPercentageMode_ = false;  // Switch to PID mode
  pid_.SetMode(AUTOMATIC);
  setpointPsi_ = psi;
}

void PressureControl::setPercentage(uint8_t percentage) {
  // Limit range 1-128
  if (percentage < 1) percentage = 1;
  if (percentage > 128) percentage = 128;
  
  isPercentageMode_ = true;  // Switch to percentage mode
  pid_.SetMode(MANUAL);  // Disable PID to avoid interference
  steamPercentage_ = percentage;
}

void PressureControl::setAlwaysOn() {
  isPercentageMode_ = false;  // Exit percentage mode
  pid_.SetMode(MANUAL);
  outputPct_ = 100;
}

void PressureControl::setAlwaysOff() {
  isPercentageMode_ = false;  // Exit percentage mode
  pid_.SetMode(MANUAL);
  outputPct_ = 0;
}


// void PressureControl::setCurrentPressure(double psi) {
//   currentPsi_ = psi;
// }

void PressureControl::update() {
  currentPsi_ = calculatePressure();
  

  const unsigned long now = millis();
  if (now - prevMs_ >= kCtrlMs) {
    prevMs_ = now;
    // Serial.println(currentPsi_);

    if (isPercentageMode_) {
      // Steam mode: directly use percentage, don't calculate PID
      psm::setValue(steamPercentage_);
    } else {
      // Brew mode or other modes: use PID control or direct output
      if (pid_.GetMode() == AUTOMATIC) {
        pid_.Compute();   // Calculate 0..100%
      }
      // Send percentage to dimmer (PSM)
      const uint16_t v = pctToPsm(outputPct_);
      // If MANUAL mode (setAlwaysOn/Off), directly use outputPct_; otherwise check setpoint
      if (pid_.GetMode() == MANUAL) {
        psm::setValue(v);  // MANUAL mode: directly use outputPct_
      } else {
        psm::setValue(setpointPsi_ ? v : 0); // AUTOMATIC mode: zero setpoint -> zero output
      }
    }
  }

  // PSM needs update every cycle to follow zero-crossing timing
  psm::update();
}
