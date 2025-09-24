#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h"
#include "pressure_control.h" 
#include "tempControl.h" 


TemperatureSensor tempSensor;
PressureControl pc(1.1, 0.1, 0.1);

TaskHandle_t mainTask;
TaskHandle_t uiTask;

void mainLoop(void * pvParameters);
void uiLoop(void * pvParameters);

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

  xTaskCreate(uiLoop, "uiLoop", 10000, NULL, 2, &uiTask);
  xTaskCreate(mainLoop, "mainLoop", 10000, NULL, 2, &uiTask);

}


volatile float targetPressure;
volatile float targetTemperature;
volatile float currentTemperature;
volatile float currentPressure;

void uiLoop(void * pvParameters) {
   for(;;) {
    menu.pollInput();
    
    menu.show();

    int step = menu.consumeStep();
    if (step != 0) menu.moveSelection(step > 0);
    if (menu.consumeClick()) menu.select();
    //temperature 
    targetTemperature = menu.getTargetTemperature();
    
    //pressure
    targetPressure = menu.getTargetPressure();

    menu.setCurrentTemperature(currentTemperature);
    menu.setCurrentPressure(currentPressure);

    yield();
  }
}


void mainLoop(void * pvParameters) {
  for(;;) {
    static unsigned long TIME_START = 0;
    static unsigned long TIME_END = 0;

    TIME_START = millis();
    // pc.setSetpoint(sp);
    if (digitalRead(PIN_SWITCH_BREW)) {
      pc.setSetpoint(targetPressure);
    }
    else {
      pc.setSetpoint(0);
    }
    
    tempControl::setSetpoint(targetTemperature);
    currentTemperature = tempControl::getTemperature();

    currentPressure = pc.getPressure();

    pc.update();

    tempControl::update();

    
    TIME_END = millis();
    
    static unsigned long TIME_LAST_PRINT = 0;
    if (millis() - TIME_LAST_PRINT >= 1000) {
      TIME_LAST_PRINT = millis();
      Serial.print("last time:");
      Serial.println(TIME_END - TIME_START);
    }

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
