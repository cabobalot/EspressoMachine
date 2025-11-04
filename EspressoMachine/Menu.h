#ifndef MENU_H
#define MENU_H

#include <Wire.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "bitmaps.h"
#include <Preferences.h>  // 用于持久化存储设置

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64  // OLED 屏幕是 128x32
#define OLED_RESET -1

#define MAX_MENU_ITEMS 2  
#define MAX_VISIBLE_ITEMS 3  
#define MAX_MODE_ITEMS 4 // mode items
#define MAX_SETTING_ITEMS 6   // Temperature, Pressure, PreinfPress, PreinfTime, Back

enum MenuState {
    MAIN_MENU,
    MODE_PAGE,
    SETTING_PAGE,
    BREW_PAGE,//brew page
    STEAM_PAGE,//steam page
    WATER_PAGE
};

class Menu {
private:
    //Adafruit_SSD1306 display;
    Adafruit_SH1106G display;
    int8_t listSelection = 0;  // 当前选中的菜单项
    int8_t scrollOffset = 0;   // 滚动偏移量
    const char* menuItems[MAX_MENU_ITEMS] = {"Mode", "Setting"};
    const char* modeMenuItems[MAX_MODE_ITEMS] = {"Steam", "Brew", "Water", "Back"};
    const char* settingMenuItems[MAX_SETTING_ITEMS] = {"Temperature", "BrewPressure", "SteamPressure","PreinfPress", "PreinfTime", "Back"}; 
    MenuState currentState = MAIN_MENU; 
    //setter for pressure and temp
    uint8_t temperature = 70;
    float currentTemperature = 0; 
    float currentPressurePsi = 0.0;
    //Default Value
    float targetPressurePsiBrew  = 40.0f;  // Default for brew pressure
    float targetPressurePsiSteam = 20.0f;  // Default for steam pressure
    float   preinfPressurePsi = 20.0f;   // default 20 PSI
    uint16_t preinfTimeSec    = 5;      // default 5
    bool isEditingPreinfPressure = false;
    bool isEditingPreinfTime     = false;
    //Editing pressure
    bool  isEditingPressureBrew  = false;
    bool  isEditingPressureSteam = false;
    bool isEditingTemperature = false;
    //count for brew screen
    unsigned long brewStartTime = 0;
    unsigned long lastFrameTime = 0;
    // count for steam screen
    unsigned long steamStartTime = 0;
    //count for water page
    unsigned long waterStartTime = 0; 
    int currentFrame = 0;
    const unsigned long frameInterval = 500;

public:
    Menu();
    bool begin();
    void show();
    void moveSelection(bool up);
    void select();
    //prefinpressure and prefintime
    float   getPreinfPressure() const { return preinfPressurePsi; }
    uint16_t getPreinfTimeSec() const { return preinfTimeSec; }
    void    setPreinfPressure(float psi) { preinfPressurePsi = psi; }
    void    setPreinfTimeSec(uint16_t s) { preinfTimeSec = s; }
    //Temp
    void setCurrentTemperature(float temp);
    float getTargetTemperature() const;
    //pressure
    void setCurrentPressure(float psi);
    // Brew & Steam target pressure
    void  setTargetPressureBrew(float psi)  { targetPressurePsiBrew  = psi; }
    void  setTargetPressureSteam(float psi) { targetPressurePsiSteam = psi; }
    float getTargetPressureBrew() const     { return targetPressurePsiBrew; }
    float getTargetPressureSteam() const    { return targetPressurePsiSteam; }
    //Encoder Function
    bool beginInput(int pinA, int pinB, int pinBtn);
    void pollInput();
    bool consumeClick();  
    int  consumeStep(); 
    //State machine
    void showIdlePage();          // back to home page
    void showBrewPagePublic();    // brew page start pressure control
    void startBrewAnimation();    // start animation 
    void stopBrewAnimation(); 
    void showSteamPage();
    //Steam page
    void startSteamTimer() { steamStartTime = millis(); }
    //state control
    void setState(MenuState s);     // 仅切换页面状态，不直接绘制
    void resetBrewAnimation();
    // 设置持久化存储
    void saveSettings();            // 保存当前设置到NVS
    void loadSettings();             // 从NVS加载设置
    // 用户档案管理（为第二步功能预留）
    void saveProfile(uint8_t profileId);  // 保存当前设置到指定档案
    void loadProfile(uint8_t profileId);  // 从指定档案加载设置
    uint8_t getCurrentProfileId() const { return currentProfileId; }
    void setCurrentProfileId(uint8_t id) { currentProfileId = id; }
      
private:
    void showSidebarInfo();
    void showMainMenu();
    void showModePage();
    void showSettingPage();
    void showTemperatureSetting();
    void showBrewPage();
    void showWaterPage();
    // encoder
    int _pinA=-1, _pinB=-1, _pinBtn=-1;

    // ISR and numb
    static void IRAM_ATTR isrAWrapper();
    static Menu* _self;
    void IRAM_ATTR onAChangeISR();

    volatile bool _encFired=false;
    volatile bool _encUp=false;
    volatile int  _stepAccum=0;

    // Debounce state (new)
    bool     _btnPrevRaw    = false;
    bool     _btnStable     = false;
    bool     _btnLatched    = false;
    uint32_t _btnEdgeTimeMs = 0;
    uint32_t _pressCount    = 0;
    static constexpr uint16_t DEBOUNCE_MS = 15;
    bool _clicked=false;
    
    // Preferences存储
    Preferences preferences;
    static const char* NVS_NAMESPACE;  // NVS命名空间
    uint8_t currentProfileId = 0;      // 当前使用的档案ID（0=默认设置，1-9=用户档案）
};

extern Menu menu;

#endif
