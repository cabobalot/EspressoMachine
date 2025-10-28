#pragma once
#include <Arduino.h>
#include <PID_v1.h>
#include "psm.hpp"

class PressureControl {
public:
  // 执行端与 PID 的基本参数
  static constexpr uint32_t kCtrlMs   = 50;     // 控制周期 50ms
  static constexpr uint16_t kPsmRange = 127;    // PSM 档位（按你的 psm 实现改）

  PressureControl(double kp, double ki, double kd);

  void init(uint8_t controlPin, uint8_t zeroCrossPin);   // 初始化：PID + PSM
  void setSetpoint(double psi);
  void setCurrentPressure(double psi);

  double setPressure(double setPsi);

  void update();

  void setAlwaysOn();
  void setAlwaysOff();

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


  // 把 0..100% 映射到 PSM 档位（0..kPsmRange）
  static inline uint16_t pctToPsm(double pct) {
    if (!isfinite(pct)) pct = 0;
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    return (uint16_t)lround(pct * kPsmRange / 100.0);
  }
};
