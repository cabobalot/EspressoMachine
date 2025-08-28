#include <Arduino.h>
#include "tempControl.h"
#include <PID_v1.h>

#define PIN_OUTPUT 26
#define PWM_CHANNEL 0
#define PWM_FREQ 1
#define PWM_RES 7

// Static variables to hold current temp and target
static double currentTemp = 0.0;
static double setpoint = 0.0;
static double outputDuty= 0.0;

static double Kp = 0.1, Ki = 0.1, Kd = 0.1;

// PID object from library
static PID tempPID(&currentTemp, &outputDuty, &setpoint, Kp, Ki, Kd, DIRECT);

void tempControl::init() {
  ledcAttachChannel(PIN_OUTPUT, PWM_FREQ, PWM_RES, PWM_CHANNEL); // (pin, 1Hz frequency, 7-bit resolution, channel)
  tempPID.SetMode(AUTOMATIC);
}

void tempControl::setSetpoint(float temp){
  setpoint = temp;
}

void tempControl::setCurrentTemp(float temp){
  currentTemp = temp;
}

void tempControl::setTemperature(float tempSetpoint) {

  //PID Controller  
  tempPID.Compute();
  // Change Duty Cycle
  ledcWrite(PWM_CHANNEL, (int)outputDuty);

}

float tempControl::getTemperature() {
  return currentTemp;
}

float tempControl::getSetpoint(){
  return setpoint;
}


void tempControl::update() {
    if(currMillis - prevMillis >= interval) {
      prevMillis = currMillis;

      currentTemp = tempControl::getTemperature(); // grab current temp
      
      tempControl::setTemperature(targetTemp); // reach that temperature

    }
}


