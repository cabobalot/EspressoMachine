#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"


// #define ENCODER_A 3
// #define ENCODER_B 10
// #define ENCODER_BUTTON 11

#define ENCODER_A 33          // CLK/A -> GPIO18
#define ENCODER_B 25          // DT/B -> GPIO19
#define ENCODER_BUTTON 32      // SW    -> GPIO5
#define PRESSURE_PIN 36  

volatile boolean encoderFired = false;
volatile boolean encoderUp = false;
unsigned long debounceTime;
bool buttonPressed = false;
bool buttonClicked = false;
//Fake Temp
float currentTemperature = 0;
TemperatureSensor tempSensor;


void IRAM_ATTR encoderInterrupt() {
    bool pina = digitalRead(ENCODER_A);
    bool pinb = digitalRead(ENCODER_B);

    if (pina) {
        encoderUp = pinb;
    } else {
        encoderUp = !pinb;
    }
    encoderFired = true;
}

void setup() {
    Serial.begin(115200);

    if (!menu.begin()) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    pinMode(ENCODER_A, INPUT);
    pinMode(ENCODER_B, INPUT);
    pinMode(PRESSURE_PIN, INPUT);
    pinMode(ENCODER_BUTTON, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderInterrupt, CHANGE);
}

void loop() {
    menu.show();
    currentTemperature = tempSensor.readTemperature();
    menu.setCurrentTemperature(currentTemperature);
    //Fake Pressure  
    float psiValue = calculatePressure();  // 来自 pressure_sensor.h
    menu.setCurrentPressure(psiValue);

    if (encoderFired) {
        menu.moveSelection(encoderUp);
        encoderFired = false;
    }

    if (!digitalRead(ENCODER_BUTTON)) {
        if (buttonPressed) {
            if (debounceTime < millis()) {
                buttonClicked = true;
            }
        } else {
            debounceTime = millis() + 50;
            buttonPressed = true;
        }
    } else {
        if (buttonClicked) {
            menu.select();  
            buttonClicked = false;
            buttonPressed = false;
        }
    }
}
