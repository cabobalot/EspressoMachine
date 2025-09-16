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
  pc.setSetpoint(40.0);

  pc.setCurrentPressure(20.0);
  menu.beginInput(PIN_KNOB_ROTATE_A, PIN_KNOB_ROTATE_B, PIN_KNOB_BUTTON);
}

void loop() {
  menu.pollInput();
  menu.show();

  int step = menu.consumeStep();    
  if (step != 0) menu.moveSelection(step > 0);
  if (menu.consumeClick()) menu.select();
  //temperature 
  float spC = menu.getTargetTemperature();
  tempControl::setSetpoint(spC);
  //pressure
  double sp = (double)menu.getTargetPressure();
  pc.setSetpoint(sp);

  pc.update();
  menu.setCurrentTemperature(tempControl::getTemperature());  
  menu.setCurrentPressure(pc.getPressure());

}
