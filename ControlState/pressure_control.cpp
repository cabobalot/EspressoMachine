#include "pressure_control.h"

PressureControl::PressureControl(double kp, double ki, double kd)
: pid_(&currentPsi_, &outputPct_, &setpointPsi_, kp, ki, kd, DIRECT) {
  // 输出范围限定为单向（只能“加压/加热”）
  pid_.SetOutputLimits(0, 100);         // 0..100 %
  pid_.SetSampleTime(kCtrlMs);          // 与调度周期一致
}

void PressureControl::init(uint8_t controlPin, uint8_t zeroCrossPin) {
  // 初始化 PSM（零过零整周跳变调功）
  psm::config(controlPin, zeroCrossPin);

  // 启动 PID 自动模式
  pid_.SetMode(AUTOMATIC);

  // 初始输出清零
  outputPct_ = 0;
  psm::setValue(0);
}

void PressureControl::setSetpoint(double psi) {
  setpointPsi_ = psi;
}

void PressureControl::setCurrentPressure(double psi) {
  currentPsi_ = psi;
}
double PressureControl::setPressure(double setPsi) {
  setSetpoint(setPsi);
  pid_.Compute();   // 计算 0..100%

  // 把百分比送给 dimmer（PSM）
  const uint16_t v = pctToPsm(outputPct_);
  psm::setValue(v);

  return outputPct_;
}

void PressureControl::update() {
  const unsigned long now = millis();

  if (now - prevMs_ >= kCtrlMs) {
    prevMs_ = now;

    // 执行一次 PID -> PSM
    setPressure(setpointPsi_);
  }

  // PSM 需要每圈都 update 来跟过零节拍
  psm::update();
}
