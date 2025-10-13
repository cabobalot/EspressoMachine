#include <Arduino.h>
#include "tempControl.h"
#include <PID_v1.h> // PID library
#include "driver/mcpwm.h" // ESP32 PWM library
#include "soc/mcpwm_periph.h"

#define OUTPUT_PIN 26
#define PWM_FREQ 1

// Static variables to hold current temp and target
static double currentTemp = 0.0;
static double setpoint = 0.0;
static double outputDuty= 0.0;

static double Kp = 1.1, Ki = 0.01, Kd = 0.01;

unsigned long prevMillis = 0;
const unsigned long interval = 1000; 

// PID object from library
static PID tempPID(&currentTemp, &outputDuty, &setpoint, Kp, Ki, Kd, DIRECT);

void tempControl::init() {

  // PID setup
  tempPID.SetMode(AUTOMATIC);
  tempPID.SetOutputLimits(0, 100);  // clamp output

  // PWM setup
  mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_1, 60000);

  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, OUTPUT_PIN);
  mcpwm_config_t pwm_config;
  pwm_config.frequency = PWM_FREQ;
  pwm_config.cmpr_a = 0.0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);

  Serial.println(mcpwm_get_frequency(MCPWM_UNIT_0, MCPWM_TIMER_1));
}

void tempControl::setSetpoint(float temp){
  setpoint = temp;
}

void tempControl::setCurrentTemp(float temp){
  currentTemp = temp;
}

void tempControl::setTemperature(float tempSetpoint) {

  //PID Controller  
  if(tempPID.Compute()) {
    // Change Duty Cycle
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, outputDuty);
  }

}

float tempControl::getTemperature() {
  return currentTemp;
}

float tempControl::getSetpoint(){
  return setpoint;
}


void tempControl::update() {

    unsigned long currMillis = millis();

    if(currMillis - prevMillis >= interval) {
      prevMillis = currMillis;

      currentTemp = tempControl::getTemperature(); // grab current temp
      
      tempControl::setTemperature(setpoint); // reach that temperature

      //Test code
    //   Serial.print("CurrentTemp:");
    //   Serial.print(currentTemp, 2);
    //   Serial.print(",Setpoint:");
    //   Serial.print(setpoint, 2);
    //   Serial.print(",PWMOutput:");
    //   Serial.println(outputDuty, 2);

    }
}