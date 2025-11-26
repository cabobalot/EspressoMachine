#pragma once
#include <Arduino.h>
#include <PID_v1.h>
#include "psm.hpp"
#include "pressure_sensor.h"


#define MAX_PRESSURE 

class PressureControl {
public:
  // 执行端与 PID 的基本参数
  static constexpr uint32_t kCtrlMs   = 17;     // 控制周期 50ms
  static constexpr uint16_t kPsmRange = 128;    // PSM 档位（1-128）

  PressureControl(double kp, double ki, double kd);

  void init(uint8_t controlPin, uint8_t zeroCrossPin);   // 初始化：PID + PSM
  void setSetpoint(double psi);
  void setPercentage(uint8_t percentage);  // 设置百分比模式（1-128），用于steam模式
  
  void setAlwaysOn();
  void setAlwaysOff();

  void update();

  double getPressure()  const { return currentPsi_; }
  double getSetpoint()  const { return setpointPsi_; }
  double getOutputPct() const { return outputPct_; }

private:
  // PID 变量
  double currentPsi_  = 0.0;
  double setpointPsi_ = 0.0;
  double outputPct_   = 0.0;

  // PID 控制器
  PID pid_;

  // 调度
  unsigned long prevMs_ = 0;

  // 百分比模式（用于steam模式）
  bool isPercentageMode_ = false;
  uint8_t steamPercentage_ = 0;  // 1-128


  // 把 0..100% 映射到 PSM 档位（0..kPsmRange）
  static inline uint16_t pctToPsm(double pct) {
    if (!isfinite(pct)) pct = 0;
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    return (uint16_t)lround(pct * kPsmRange / 100.0);
  }
};
