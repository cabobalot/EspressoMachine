#pragma once
#include <Arduino.h>
#include <PID_v1.h>
#include "psm.hpp"
#include "pressure_sensor.h"


#define MAX_PRESSURE 

class PressureControl {
public:
  // Basic parameters for actuator and PID
  static constexpr uint32_t kCtrlMs   = 17;     // Control period 50ms
  static constexpr uint16_t kPsmRange = 128;    // PSM range (1-128)

  PressureControl(double kp, double ki, double kd);

  void init(uint8_t controlPin, uint8_t zeroCrossPin);   // Initialize: PID + PSM
  void setSetpoint(double psi);
  void setPercentage(uint8_t percentage);  // Set percentage mode (1-128), for steam mode
  
  void setAlwaysOn();
  void setAlwaysOff();

  void update();

  double getPressure()  const { return currentPsi_; }
  double getSetpoint()  const { return setpointPsi_; }
  double getOutputPct() const { return outputPct_; }

private:
  // PID variables
  double currentPsi_  = 0.0;
  double setpointPsi_ = 0.0;
  double outputPct_   = 0.0;

  // PID controller
  PID pid_;

  // Scheduling
  unsigned long prevMs_ = 0;

  // Percentage mode (for steam mode)
  bool isPercentageMode_ = false;
  uint8_t steamPercentage_ = 0;  // 1-128


  // Map 0..100% to PSM range (0..kPsmRange)
  static inline uint16_t pctToPsm(double pct) {
    if (!isfinite(pct)) pct = 0;
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    return (uint16_t)lround(pct * kPsmRange / 100.0);
  }
};
