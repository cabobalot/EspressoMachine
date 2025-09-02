#include "pressure_control.h"
#include "pressure_sensor.h"
#include <Arduino.h>

PressureControl::PressureControl()
  : targetPsi(0), currentPsi(0), output(0),
    pid(&currentPsi, &output, &targetPsi, 0.8, 0.1, 0.0, DIRECT)
{
    pid.SetOutputLimits(0, 100);   // 输出范围 0–100 %
    pid.SetSampleTime(50);         // 50 ms 一次
    pid.SetMode(AUTOMATIC);        // 启动 PID
}

void PressureControl::setTarget(double psi) { targetPsi = psi; }
double PressureControl::getTarget() const { return targetPsi; }
double PressureControl::getPressure() const { return currentPsi; }

double PressureControl::update() {
    double raw = calculatePressure();
    currentPsi = 0.85 * currentPsi + 0.15 * raw;
    pid.Compute();
    //return a percentage 
    return output;
}
