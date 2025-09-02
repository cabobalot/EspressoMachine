#ifndef PRESSURE_CONTROL_H
#define PRESSURE_CONTROL_H

#include <PID_v1.h>

class PressureControl {
public:
    PressureControl();

    void setTarget(double psi);
    double getTarget() const;
    double getPressure() const;

    double update();   // 每个周期调用，返回输出百分比

private:
    double targetPsi;
    double currentPsi;
    double output;     // PID 输出

    PID pid;           // 使用 Arduino PID_v1 库
};

#endif
