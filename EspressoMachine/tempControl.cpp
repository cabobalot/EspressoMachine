#include <Arduino.h>
#include "tempControl.h"
#include <PID_v1.h> // PID library
#include "driver/mcpwm.h" // ESP32 PWM library
#include "soc/mcpwm_periph.h"
#include "pins.h"

#define PWM_FREQ 1 // Frequency of PWM pin (1Hz)

// Static variables to hold current temp and target
static double currentTemp = 0.0;
static double setpoint = 0.0;
static double outputDuty= 0.0;

// PID varibles
static double Kp = 7.0, Ki = 1.5, Kd = 4.0;

// Delay between updates (1 sec)
unsigned long prevMillis = 0;
const unsigned long interval = 1000;

// PID object from library
static PID tempPID(&currentTemp, &outputDuty, &setpoint, Kp, Ki, Kd, DIRECT);

/*
Setup to allow for the temperature control to change the temperature
*/
void tempControl::init() {

  // PID setup
  tempPID.SetMode(AUTOMATIC);
  tempPID.SetOutputLimits(0, 100);  // clamp output

  // PWM setup
  mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_1, 60000);

  // Config to setup the PWM for the ESP32
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PIN_HEATER);
  mcpwm_config_t pwm_config;
  pwm_config.frequency = PWM_FREQ;
  pwm_config.cmpr_a = 0.0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
}

/*
Setter for the desired setpoint
*/
void tempControl::setSetpoint(float temp){
  setpoint = temp;
}

/*
Setter for the current temperature
*/ 
void tempControl::setCurrentTemp(float temp){
  currentTemp = temp;
}

/*
Controls and updates the duty cycle to reach the setpoint
*/
void tempControl::setTemperature(float tempSetpoint) {
  // PID controller
  if(tempPID.Compute()) {
    // Changes the duty cycle
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, outputDuty);
  }
}

/*
Getter for temperature
*/
float tempControl::getTemperature() {
  return currentTemp;
}

/*
Getter for setpoint
*/ 
float tempControl::getSetpoint(){
  return setpoint;
}

/*
At given intervals gets the current temperature and updates it until the 
set temperature is reached.
*/
void tempControl::update() {

    unsigned long currMillis = millis();

    if(currMillis - prevMillis >= interval) { // Delay 
      prevMillis = currMillis;

      currentTemp = TemperatureSensor::getTemperature(); // Grab the current temperature
      
      tempControl::setTemperature(setpoint); // Reach that temperature

    }
}
