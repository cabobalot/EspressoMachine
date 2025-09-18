#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h"
#include "pressure_control.h" 
#include "tempControl.h" 


TemperatureSensor tempSensor;
PressureControl pc(1.1, 0.1, 0.1);

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
}

void loop() {
  static unsigned long TIME_START = 0;
  static unsigned long TIME_END = 0;

  menu.pollInput();
  
  TIME_START = millis();
  menu.show();   //TODO this takes too long and blocks... :(
  TIME_END = millis();
  

  int step = menu.consumeStep();
  if (step != 0) menu.moveSelection(step > 0);
  if (menu.consumeClick()) menu.select();
  //temperature 
  float spC = menu.getTargetTemperature();
  tempControl::setSetpoint(spC);
  //pressure
  double sp = (double)menu.getTargetPressure();

  // pc.setSetpoint(sp);
  if (digitalRead(PIN_SWITCH_BREW)) {
    pc.setSetpoint(sp);
  }
  else {
    pc.setSetpoint(0);
  }
  

  pc.update();
  menu.setCurrentTemperature(tempControl::getTemperature());  
  menu.setCurrentPressure(pc.getPressure());

  tempControl::update();

  
  
  static unsigned long TIME_LAST_PRINT = 0;
  if (millis() - TIME_LAST_PRINT >= 1000) {
    TIME_LAST_PRINT = millis();
    Serial.print("last time:");
    Serial.println(TIME_END - TIME_START);
  }

}
