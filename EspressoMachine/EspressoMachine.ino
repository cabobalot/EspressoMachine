#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h"
#include "pressure_control.h" 
#include "tempControl.h" 


#define TEMPERATURE_OFFSET 7 // stock calibration offset

TemperatureSensor tempSensor;
PressureControl pc(1.1, 0.1, 0.1);

TaskHandle_t mainTask;
TaskHandle_t uiTask;

void mainLoop(void * pvParameters);
void uiLoop(void * pvParameters);

static volatile float targetPressure = 0;
static volatile float targetTemperature = 70;
static volatile float currentTemperature = 0;
static volatile float currentPressure = 0;

static volatile float preinfusePressure = 20;
static volatile unsigned long preinfuseTime = 10;



enum MachineState {
  IDLE_STATE,
  BREW_STATE,
  STEAM_STATE,
  HOT_WATER_STATE
}

static volatile MachineState machineState = IDLE_STATE;


void setup() {
  Serial.begin(115200);

  Wire.begin(PIN_SCREEN_SDA, PIN_SCREEN_SCL);
  Wire.setClock(400000); 
  if (!menu.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  //temp
  tempControl::init();
  //presure
  pinMode(PIN_PRESSURE_SENSE, INPUT);
  pc.init(PIN_DIMMER_CONTROL, PIN_DIMMER_ZERO_CROSS);
  //temporary init
  pc.setSetpoint(0.0);

  menu.beginInput(PIN_KNOB_ROTATE_A, PIN_KNOB_ROTATE_B, PIN_KNOB_BUTTON);

  pinMode(PIN_SWITCH_BREW, INPUT_PULLDOWN);
  pinMode(PIN_SWITCH_STEAM, INPUT_PULLDOWN);

  pinMode(PIN_SOLENOID, OUTPUT);
  digitalWrite(PIN_SOLENOID, LOW);

  xTaskCreate(uiLoop, "uiLoop", 10000, NULL, 2, &uiTask);
  xTaskCreate(mainLoop, "mainLoop", 10000, NULL, 2, &uiTask);

}


void uiLoop(void * pvParameters) {
   for(;;) {
    menu.pollInput();
    
    menu.show();

    int step = menu.consumeStep();
    if (step != 0) menu.moveSelection(step > 0);
    if (menu.consumeClick()) menu.select();
    

    targetTemperature = menu.getTargetTemperature();
    targetPressure = menu.getTargetPressure();
    // preinfusePressure = menu.getPreinfusePressure();
    // preinfuseTime = menu.getPreinfuseTime();

    menu.setCurrentTemperature(currentTemperature);
    menu.setCurrentPressure(currentPressure);


    // state switch
    bool brewSwitch = digitalRead(PIN_SWITCH_BREW);
    bool steamSwitch = digitalRead(PIN_SWITCH_STEAM);
    if (!brewSwitch && !steamSwitch) {
      machineState = IDLE_STATE;
      if (machineState != IDLE_STATE) {
        //menu.stopAll(); // or go to home screen or something
      }
    }
    else if (brewSwitch && !steamSwitch) {
      if (machineState != BREW_STATE) {
        //menu.startBrew();
        //TODO preinfuse and pressure profile
      }
      machineState = BREW_STATE;
    }
    else if (steamSwitch && !brewSwitch) {
      machineState = STEAM_STATE;
      if (machineState != STEAM_STATE) {
        //menu.startSteam();
      }
    }
    else if (brewSwitch && steamSwitch) {
      machineState = HOT_WATER_STATE;
      if (machineState != HOT_WATER_STATE) {
        //menu.startHotWater();
      }
    }
    else {
      // https://www.youtube.com/watch?v=5IsSpAOD6K8
    }


    yield();
  }
}


void mainLoop(void * pvParameters) {
  for(;;) {

    switch (MachineState) {
    case IDLE_STATE:
      pc.setAlwaysOff();
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    case BREW_STATE:
      pc.setSetpoint(targetPressure);
      digitalWrite(PIN_SOLENOID, HIGH);
      break;
    case STEAM_STATE:
      pc.setSetpoint(targetPressure);
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    case HOT_WATER_STATE:
      pc.setAlwaysOff();
      digitalWrite(PIN_SOLENOID, LOW);
    }

    tempControl::setSetpoint(targetTemperature + TEMPERATURE_OFFSET); // screen returns a different temperature for each mode
    currentTemperature = tempControl::getTemperature();

    currentPressure = pc.getPressure();

    pc.update();

    tempControl::update();

    
    // static unsigned long TIME_START = 0;
    // static unsigned long TIME_END = 0;
    // TIME_START = millis();
    // TIME_END = millis();
    // static unsigned long TIME_LAST_PRINT = 0;
    // if (millis() - TIME_LAST_PRINT >= 1000) {
    //   TIME_LAST_PRINT = millis();
    //   Serial.print("last time:");
    //   Serial.println(TIME_END - TIME_START);
    // }

    yield();
  }
}

void loop() {
  // static unsigned long TIME_START = 0;
  // static unsigned long TIME_END = 0;

  
  // static unsigned long TIME_LAST_PRINT = 0;
  // if (millis() - TIME_LAST_PRINT >= 1000) {
  //   TIME_LAST_PRINT = millis();
  //   Serial.print("last time:");
  //   Serial.println(TIME_END - TIME_START);
  // }

}
