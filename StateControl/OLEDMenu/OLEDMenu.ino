#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h"
#include "pressure_control.h" 
#include "tempControl.h" 


#define TEMPERATURE_OFFSET 7 // stock calibration offset

TemperatureSensor tempSensor;
PressureControl pc(5, 0.0, 0.1); //TODO try setting the pid loop delay to 16 or 17ms
// also test in steam mode

TaskHandle_t mainTask;
TaskHandle_t uiTask;

void mainLoop(void * pvParameters);
void uiLoop(void * pvParameters);

static volatile float targetPressure = 0;
static volatile float targetTemperature = 70;
static volatile float currentTemperature = 0;
static volatile float currentPressure = 0;

static volatile float preinfusePressure = 20;
static volatile unsigned long preinfuseTime = 5;

enum BrewPhase { BREW_PREINFUSE, BREW_HOLD };
static BrewPhase brewPhase = BREW_PREINFUSE;
static unsigned long brewPhaseStartMs = 0;

enum MachineState {
  IDLE_STATE,
  BREW_STATE,
  STEAM_STATE,
  HOT_WATER_STATE
};

static volatile MachineState machineState = IDLE_STATE;

static MachineState lastState = IDLE_STATE;


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
  pc.setAlwaysOff();

  menu.beginInput(PIN_KNOB_ROTATE_A, PIN_KNOB_ROTATE_B, PIN_KNOB_BUTTON);

  pinMode(PIN_SWITCH_BREW, INPUT_PULLUP);
  pinMode(PIN_SWITCH_STEAM, INPUT_PULLUP);

  pinMode(PIN_SOLENOID, OUTPUT);
  digitalWrite(PIN_SOLENOID, LOW);

  xTaskCreate(uiLoop, "uiLoop", 10000, NULL, 0, &uiTask);
  xTaskCreate(mainLoop, "mainLoop", 10000, NULL, 0, &mainTask);

}


void uiLoop(void * pvParameters) {
  // --- debounce state (no helpers, minimal) ---
  static bool     lastBrewRaw  = false,  brewStable  = false;  // raw last & debounced stable
  static bool     lastSteamRaw = false,  steamStable = false;
  static uint32_t brewEdgeMs   = 0,      steamEdgeMs = 0;
  const  uint16_t DEBOUNCE_MS  = 30;

  for(;;) {
    // ===================== UI input & rendering =====================
    menu.pollInput();
    menu.show();

    int step = menu.consumeStep();
    if (step != 0) menu.moveSelection(step > 0);
    bool clicked = menu.consumeClick();
    Serial.print(clicked);
    if (clicked) {
      Serial.print(clicked);
      if (machineState == BREW_STATE || machineState == STEAM_STATE) {
        machineState = IDLE_STATE;
        menu.setState(MAIN_MENU);   // back to home
      } else {
        menu.select();              // normal UI selection on other pages
      }
    }

    // Targets from UI
    targetTemperature = menu.getTargetTemperature() + TEMPERATURE_OFFSET;
    targetPressure    = menu.getTargetPressure();
    preinfusePressure = menu.getPreinfPressure(); 
    preinfuseTime     = menu.getPreinfTimeSec();

    // Live values back to UI
    menu.setCurrentTemperature(currentTemperature - TEMPERATURE_OFFSET);
    menu.setCurrentPressure(currentPressure);

    // ===================== Switch handling (level-based, both edges) =====================
    // Active-low hardware switches: LOW = "ON / pressed", HIGH = "OFF / released".
    bool rawBrew  = !digitalRead(PIN_SWITCH_BREW);
    bool rawSteam = !digitalRead(PIN_SWITCH_STEAM);
    uint32_t now  = millis();

    // --- Brew: debounce to get a stable logical state (brewStable) ---
    if (rawBrew != lastBrewRaw) { lastBrewRaw = rawBrew; brewEdgeMs = now; }
    if ((now - brewEdgeMs) >= DEBOUNCE_MS && rawBrew != brewStable) {
      brewStable = rawBrew;  // stable state changed

      if (brewStable) {
        // Brew switch turned ON  → enter BREW (and leave other modes)
        if (machineState != BREW_STATE) {
          machineState = BREW_STATE;
          menu.setState(BREW_PAGE);      // reset brew timer inside setState
          menu.resetBrewAnimation();
        }
      } else {
        // Brew switch turned OFF → if we are in BREW, exit to MAIN
        if (machineState == BREW_STATE) {
          machineState = IDLE_STATE;
          menu.setState(MAIN_MENU);
        }
      }
    }

    // --- Steam: debounce to get a stable logical state (steamStable) ---
    if (rawSteam != lastSteamRaw) { lastSteamRaw = rawSteam; steamEdgeMs = now; }
    if ((now - steamEdgeMs) >= DEBOUNCE_MS && rawSteam != steamStable) {
      steamStable = rawSteam;  // stable state changed

      if (steamStable) {
        // Steam switch turned ON → enter STEAM (and leave other modes)
        if (machineState != STEAM_STATE) {
          machineState = STEAM_STATE;
          menu.setState(STEAM_PAGE);     // reset steam timer inside setState
        }
      } else {
        // Steam switch turned OFF → if we are in STEAM, exit to MAIN
        if (machineState == STEAM_STATE) {
          machineState = IDLE_STATE;
          menu.setState(MAIN_MENU);
        }
      }
    }

    // (Mutual exclusion tip)
    // If you want Brew & Steam to be strictly exclusive (when both are ON),
    // you can add priority rules here. For most toggle-switch panels, users
    // won’t put both ON at once, so this can be omitted.

    yield();
  }
}





// void mainLoop(void * pvParameters) {
//   for(;;) {

//   if (machineState != lastState) {
//         if (machineState == BREW_STATE) {
//           // 进入 Brew：从预浸阶段开始
//           brewPhase = BREW_PREINFUSE;
//           brewPhaseStartMs = millis();

//           pc.setSetpoint(preinfusePressure);   // 来自 UI 的 preinfusePressure
//           digitalWrite(PIN_SOLENOID, HIGH);
//         }
//         if (lastState == BREW_STATE && machineState != BREW_STATE) {

//           pc.setSetpoint(0);
//           digitalWrite(PIN_SOLENOID, LOW);
//         }
//         lastState = machineState;
//       }

//       switch (machineState) {
//         case IDLE_STATE:
//           pc.setAlwaysOff();
//           digitalWrite(PIN_SOLENOID, LOW);
//           break;

//         case BREW_STATE: {
//           digitalWrite(PIN_SOLENOID, HIGH);
//           // prefinfuse 
//           if (brewPhase == BREW_PREINFUSE) {
//             unsigned long elapsed = millis() - brewPhaseStartMs;
//             if (elapsed >= (unsigned long)preinfuseTime * 1000UL) {
//               brewPhase = BREW_HOLD;
//               brewPhaseStartMs = millis();
//               pc.setSetpoint(targetPressure);
//             }
//           }
//           break;
//         }

//         case STEAM_STATE:
//           pc.setSetpoint(targetPressure);
//           digitalWrite(PIN_SOLENOID, LOW);
//           break;

//         case HOT_WATER_STATE:
//           pc.setAlwaysOn();
//           digitalWrite(PIN_SOLENOID, LOW);
//           break;
//       }
//     pc.update();

//     tempControl::setSetpoint(targetTemperature); // screen returns a different temperature for each mode
//     currentTemperature = tempControl::getTemperature();

//     currentPressure = pc.getPressure();


//     tempControl::update();

    
//     // TIME_END = millis();
    
//     static unsigned long TIME_LAST_PRINT = 0;
//     if (millis() - TIME_LAST_PRINT >= 210) {
//       TIME_LAST_PRINT = millis();
//       // Serial.print("last time:");
//       // Serial.println(TIME_END - TIME_START);

//       // Serial.printf("temp:%.1f, tempset:%.1f, pres:%.1f, pressSet:%.1f\r\n", currentTemperature, targetTemperature, currentPressure, targetPressure);


//       Serial.print(">temp:");
//       Serial.print(currentTemperature, 2);
//       Serial.print(",tempset:");
//       Serial.print(targetTemperature, 2);
//       Serial.print(",pres:");
//       Serial.print(currentPressure, 2);
//       Serial.print(",pressSet:");
//       Serial.println(targetPressure, 2);

//       // Serial.flush();
//     }

    

//     // taskYIELD(); // do we need this or is yield() fine?
//     yield();
//   }
// }

void mainLoop(void * pvParameters) {

  static unsigned long brewStart = 0;
  static bool brewing = false;
  for(;;) {

    switch (machineState) {
    case IDLE_STATE:
      brewing = false;
      pc.setAlwaysOff();
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    case BREW_STATE:
      if (!brewing) {
        brewStart = millis();
        brewing = true;
      }

      // // handle preinfuse
      // if (millis() - brewStart < (10 * 1000)) {
      //   pc.setSetpoint(10.0);
      // } else {  
      //   pc.setSetpoint(targetPressure);
      // }

      pc.setSetpoint(targetPressure);
      digitalWrite(PIN_SOLENOID, HIGH);
      break;
    case STEAM_STATE:
      brewing = false;
      pc.setSetpoint(targetPressure);
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    case HOT_WATER_STATE:
      brewing = false;
      pc.setAlwaysOn();
      digitalWrite(PIN_SOLENOID, LOW);
    }

    tempControl::setSetpoint(targetTemperature); // screen returns a different temperature for each mode
    currentTemperature = tempControl::getTemperature();

    currentPressure = pc.getPressure();

    pc.update();

    tempControl::update();

    
    // TIME_END = millis();
    
    static unsigned long TIME_LAST_PRINT = 0;
    if (millis() - TIME_LAST_PRINT >= 210) {
      TIME_LAST_PRINT = millis();
      // Serial.print("last time:");
      // Serial.println(TIME_END - TIME_START);

      // Serial.printf("temp:%.1f, tempset:%.1f, pres:%.1f, pressSet:%.1f\r\n", currentTemperature, targetTemperature, currentPressure, targetPressure);


      Serial.print(">temp:");
      Serial.print(currentTemperature, 2);
      Serial.print(",tempset:");
      Serial.print(targetTemperature, 2);
      Serial.print(",pres:");
      Serial.print(currentPressure, 2);
      Serial.print(",pressSet:");
      Serial.println(targetPressure, 2);

      // Serial.flush();
    }

    

    // taskYIELD(); // do we need this or is yield() fine?
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