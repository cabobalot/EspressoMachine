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


// setting mode to manual and automatic should prevent integral windup
void PressureControl::setSetpoint(double psi) {
  isPercentageMode_ = false;  // 切换到PID模式
  pid_.SetMode(AUTOMATIC);
  setpointPsi_ = psi;
}

void PressureControl::setPercentage(uint8_t percentage) {
  // 限制范围 1-128
  if (percentage < 1) percentage = 1;
  if (percentage > 128) percentage = 128;
  
  isPercentageMode_ = true;  // 切换到百分比模式
  pid_.SetMode(MANUAL);  // 禁用PID，避免干扰
  steamPercentage_ = percentage;
}

void PressureControl::setAlwaysOn() {
  isPercentageMode_ = false;  // 退出百分比模式
  pid_.SetMode(MANUAL);
  outputPct_ = 100;
}

void PressureControl::setAlwaysOff() {
  isPercentageMode_ = false;  // 退出百分比模式
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
      // Steam模式：直接使用百分比，不计算PID
      psm::setValue(steamPercentage_);
    } else {
      // Brew模式或其他模式：使用PID控制或直接输出
      if (pid_.GetMode() == AUTOMATIC) {
        pid_.Compute();   // 计算 0..100%
      }
      // 把百分比送给 dimmer（PSM）
      const uint16_t v = pctToPsm(outputPct_);
      // 如果是MANUAL模式（setAlwaysOn/Off），直接使用outputPct_；否则检查setpoint
      if (pid_.GetMode() == MANUAL) {
        psm::setValue(v);  // MANUAL模式：直接使用outputPct_
      } else {
        psm::setValue(setpointPsi_ ? v : 0); // AUTOMATIC模式：zero setpoint -> zero output
      }
    }
  }

  // PSM 需要每圈都 update 来跟过零节拍
  psm::update();
}
