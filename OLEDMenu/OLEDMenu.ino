#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h" 



TemperatureSensor tempSensor;

void setup() {
  Serial.begin(115200);

  Wire.begin(PIN_SCREEN_SDA, PIN_SCREEN_SCL);
  Wire.setClock(400000); 
  if (!menu.begin()) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  menu.beginInput(PIN_KNOB_ROTATE_A, PIN_KNOB_ROTATE_B, PIN_KNOB_BUTTON);
  pinMode(PIN_PRESSURE_SENSE, INPUT);
}

void loop() {
  menu.pollInput();
  menu.show();

  int step = menu.consumeStep();    
  if (step != 0) menu.moveSelection(step > 0);
  if (menu.consumeClick()) menu.select();

  static unsigned long lastTemp = 0;
  if (millis() - lastTemp > 500) {
    float t = tempSensor.readTemperature();
    Serial.println(t);
    menu.setCurrentTemperature(t);
    lastTemp = millis();
  }
  float psiValue = calculatePressure();
  menu.setCurrentPressure(psiValue);
}
