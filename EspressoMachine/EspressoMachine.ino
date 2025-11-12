#include "Menu.h"
#include "bitmaps.h"
#include "pressure_sensor.h"
#include "temp_sensor.h"
#include "pins.h"
#include "pressure_control.h" 
#include "tempControl.h" 
#include "dataWebPage.h"
#include "EspressoMachine.h"

#include "timingTestDebug.h"


#define TEMPERATURE_OFFSET 7 // stock calibration offset

TemperatureSensor tempSensor;
PressureControl pc(5, 0.0, 0.1);
// also test in steam mode

TaskHandle_t mainTask;
TaskHandle_t uiTask;
TaskHandle_t screenShowTask;

void mainLoop(void * pvParameters);
void uiLoop(void * pvParameters);
void screenShowLoop(void * pvParameters);

static volatile float targetTemperature = 95;
static volatile float targetPressureBrew  = 120.0f;
static volatile float targetPressureSteam = 25.0f;
static volatile float currentTemperature = 0;
static volatile float currentPressure = 0;

static volatile float preinfusePressure = 20;
static volatile unsigned long preinfuseTime = 10;
static volatile float preinfusePressure = 20.0f;  // 与Menu.h中的preinfPressurePsi默认值一致
static volatile unsigned long preinfuseTime = 5;  // 与Menu.h中的preinfTimeSec默认值一致

// Brew模式压力控制：记录brew开始时间，用于切换预浸泡和正常压力
static volatile unsigned long brewStartTimeMs = 0;
static volatile float currentTargetPressure = 0.0f;  // 当前实际使用的目标压力（用于串口输出）


static volatile MachineState machineState = IDLE_STATE;


void setup() {
  Serial.begin(115200);

  if (!menu.begin()) { // doing this here so we have serial if it fails
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  dataWebPage::init();

  xTaskCreatePinnedToCore(uiLoop, "uiLoop", 10000, NULL, 1, &uiTask, 1);
  xTaskCreatePinnedToCore(mainLoop, "mainLoop", 10000, NULL, 1, &mainTask, 1);
  xTaskCreatePinnedToCore(screenShowLoop, "screenShowLoop", 10000, NULL, 1, &screenShowTask, 0);
}

void screenShowLoop(void * pvParameters) {
  for(;;) {
    menu.show();
    vTaskDelay(50 / portTICK_PERIOD_MS); // 20 FPS max
  }
}

void uiLoop(void * pvParameters) {
  // --- debounce state ---
  static bool     lastBrewRaw  = false,  brewStable  = false;
  static bool     lastSteamRaw = false,  steamStable = false;
  static uint32_t brewEdgeMs   = 0,      steamEdgeMs = 0;
  const  uint16_t DEBOUNCE_MS  = 30;

  menu.beginInput(PIN_KNOB_ROTATE_A, PIN_KNOB_ROTATE_B, PIN_KNOB_BUTTON); // pins an interrupt on this core

  pinMode(PIN_SWITCH_BREW, INPUT_PULLUP);
  pinMode(PIN_SWITCH_STEAM, INPUT_PULLUP);

  for(;;) {
    // ===================== UI input & rendering =====================

    menu.pollInput();
    
    int step = menu.consumeStep();
    if (step != 0) menu.moveSelection(step > 0);
    if (menu.consumeClick()) menu.select();

    // Targets from UI
    targetTemperature = menu.getTargetTemperature() + TEMPERATURE_OFFSET;
    targetPressureBrew  = menu.getTargetPressureBrew();
    targetPressureSteam = menu.getTargetPressureSteam();
    preinfusePressure   = menu.getPreinfPressure(); 
    preinfuseTime       = menu.getPreinfTimeSec();

    // Live values back to UI
    menu.setCurrentTemperature(currentTemperature - TEMPERATURE_OFFSET);
    menu.setCurrentPressure(currentPressure);

    dataWebPage::update(currentTemperature - TEMPERATURE_OFFSET, currentPressure, targetTemperature, targetPressure, machineState);
    // ===================== Switch handling (level-based, both edges) =====================
    // Active-low hardware switches: LOW = pressed
    bool rawBrew  = !digitalRead(PIN_SWITCH_BREW);
    bool rawSteam = !digitalRead(PIN_SWITCH_STEAM);
    uint32_t now  = millis();

    // --- Brew debounce ---
    if (rawBrew != lastBrewRaw) { lastBrewRaw = rawBrew; brewEdgeMs = now; }
    if ((now - brewEdgeMs) >= DEBOUNCE_MS && rawBrew != brewStable) {
      brewStable = rawBrew;
    }

    // --- Steam debounce ---
    if (rawSteam != lastSteamRaw) { lastSteamRaw = rawSteam; steamEdgeMs = now; }
    if ((now - steamEdgeMs) >= DEBOUNCE_MS && rawSteam != steamStable) {
      steamStable = rawSteam;
    }

    // --- Unified decision (priority: BOTH -> WATER, else single) ---
    MachineState nextState = machineState;
    if (brewStable && steamStable) {
      nextState = HOT_WATER_STATE;            // Water 模式：两键同按
    } else if (brewStable) {
      nextState = BREW_STATE;                 // 仅 Brew 键按下
    } else if (steamStable) {
      nextState = STEAM_STATE;                // 仅 Steam 键按下
    } else {
      nextState = IDLE_STATE;                 // 都没按
    }

    // --- Apply transition once (and set the correct page) ---
    if (nextState != machineState) {
      machineState = nextState;

      switch (machineState) {
        case BREW_STATE:
          brewStartTimeMs = millis();  // 记录brew开始时间
          menu.setState(BREW_PAGE);
          menu.resetBrewAnimation();
          break;
        case STEAM_STATE:
          menu.setState(STEAM_PAGE);
          break;
        case HOT_WATER_STATE:
          menu.setState(WATER_PAGE);          // 两键同按 -> 进 Water 页面
          break;
        case IDLE_STATE:
        default:
          menu.setState(MAIN_MENU);
          break;
      }
    }

    // yield();
    vTaskDelay(20 / portTICK_PERIOD_MS); // 

  }
}

// void uiLoop(void * pvParameters) {
//   // --- debounce state (no helpers, minimal) ---
//   static bool     lastBrewRaw  = false,  brewStable  = false;  // raw last & debounced stable
//   static bool     lastSteamRaw = false,  steamStable = false;
//   static uint32_t brewEdgeMs   = 0,      steamEdgeMs = 0;
//   const  uint16_t DEBOUNCE_MS  = 30;

//   for(;;) {
//     // ===================== UI input & rendering =====================
//     menu.pollInput();
//     menu.show();

//     int step = menu.consumeStep();
//     if (step != 0) menu.moveSelection(step > 0);
//     if (menu.consumeClick()) menu.select();

//     // Targets from UI
//     targetTemperature = menu.getTargetTemperature() + TEMPERATURE_OFFSET;
//     //TODO
//     targetPressureBrew  = menu.getTargetPressureBrew();
//     targetPressureSteam = menu.getTargetPressureSteam();
//     preinfusePressure = menu.getPreinfPressure(); 
//     preinfuseTime     = menu.getPreinfTimeSec();

//     // Live values back to UI
//     menu.setCurrentTemperature(currentTemperature - TEMPERATURE_OFFSET);
//     menu.setCurrentPressure(currentPressure);

//     // ===================== Switch handling (level-based, both edges) =====================
//     // Active-low hardware switches: LOW = "ON / pressed", HIGH = "OFF / released".
//     bool rawBrew  = !digitalRead(PIN_SWITCH_BREW);
//     bool rawSteam = !digitalRead(PIN_SWITCH_STEAM);
//     uint32_t now  = millis();

//     // --- Brew: debounce to get a stable logical state (brewStable) ---
//     if (rawBrew != lastBrewRaw) { lastBrewRaw = rawBrew; brewEdgeMs = now; }
//     if ((now - brewEdgeMs) >= DEBOUNCE_MS && rawBrew != brewStable) {
//       brewStable = rawBrew;  // stable state changed

//       if (brewStable) {
//         // Brew switch turned ON  → enter BREW (and leave other modes)
//         if (machineState != BREW_STATE) {
//           machineState = BREW_STATE;
//           menu.setState(BREW_PAGE);      // reset brew timer inside setState
//           menu.resetBrewAnimation();
//         }
//       } else {
//         // Brew switch turned OFF → if we are in BREW, exit to MAIN
//         if (machineState == BREW_STATE) {
//           machineState = IDLE_STATE;
//           menu.setState(MAIN_MENU);
//         }
//       }
//     }

//     // --- Steam: debounce to get a stable logical state (steamStable) ---
//     if (rawSteam != lastSteamRaw) { lastSteamRaw = rawSteam; steamEdgeMs = now; }
//     if ((now - steamEdgeMs) >= DEBOUNCE_MS && rawSteam != steamStable) {
//       steamStable = rawSteam;  // stable state changed

//       if (steamStable) {
//         // Steam switch turned ON → enter STEAM (and leave other modes)
//         if (machineState != STEAM_STATE) {
//           machineState = STEAM_STATE;
//           menu.setState(STEAM_PAGE);     // reset steam timer inside setState
//         }
//       } else {
//         // Steam switch turned OFF → if we are in STEAM, exit to MAIN
//         if (machineState == STEAM_STATE) {
//           machineState = IDLE_STATE;
//           menu.setState(MAIN_MENU);
//         }
//       }
//     }

//     yield();
//   }
// }

void mainLoop(void * pvParameters) {
  //temperature
  tempControl::init();
  //presure
  pinMode(PIN_PRESSURE_SENSE, INPUT);
  pc.init(PIN_DIMMER_CONTROL, PIN_DIMMER_ZERO_CROSS); // this pins an interrupt to this core

  // init
  pc.setAlwaysOff();

  pinMode(PIN_SOLENOID, OUTPUT);
  digitalWrite(PIN_SOLENOID, LOW);


  for(;;) {
    switch (machineState) {
    case IDLE_STATE:
      pc.setAlwaysOff();
      digitalWrite(PIN_SOLENOID, LOW);
      currentTargetPressure = 0.0f;
      break;
    case BREW_STATE:
      {
        // 计算brew已运行时间（毫秒）
        unsigned long brewElapsedMs = millis() - brewStartTimeMs;
        unsigned long preinfuseTimeMs = preinfuseTime * 1000;  // 转换为毫秒
        
        // 前5秒使用预浸泡压力（20 PSI），之后使用70 PSI
        if (brewElapsedMs < preinfuseTimeMs) {
          currentTargetPressure = preinfusePressure;  // 20 PSI
        } else {
          currentTargetPressure = targetPressureBrew;  // 5秒后切换到70 PSI
        }
        
        pc.setSetpoint(currentTargetPressure);
        digitalWrite(PIN_SOLENOID, HIGH);
      }
      break;
    case STEAM_STATE:
      currentTargetPressure = targetPressureSteam;
      pc.setSetpoint(targetPressureSteam);
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    case HOT_WATER_STATE:
      pc.setAlwaysOn();
      digitalWrite(PIN_SOLENOID, LOW);
      break;
    }

    tempControl::setSetpoint(targetTemperature); // screen returns a different temperature for each mode
    currentTemperature = tempControl::getTemperature();

    currentPressure = pc.getPressure();

    pc.update();

    tempControl::update();

    
    static unsigned long timeLastPrint = 0;
    if (millis() - timeLastPrint >= 1000) {
      timeLastPrint = millis();

      // Serial.printf("temp:%.1f, tempset:%.1f, pres:%.1f, pressSet:%.1f\r\n", currentTemperature, targetTemperature, currentPressure, targetPressure);


      Serial.print(">temp:");
      Serial.print(currentTemperature, 2);
      Serial.print(",tempset:");
      Serial.print(targetTemperature, 2);
      Serial.print(",pres:");
      Serial.print(currentPressure, 2);
      Serial.print(",pressSet:");
      Serial.print(targetPressureBrew, 2);
      Serial.print(",targetPress:");
      Serial.println(currentTargetPressure, 2);

      // Serial.print("UI core: ");
      // Serial.print(xTaskGetCoreID(uiTask));
      // Serial.print(" main core: ");
      // Serial.println(xTaskGetCoreID(mainTask));

    }

    // pretty sure we dont need a taskDelay here but maybe
    yield();
  }
}

void loop() {

}
