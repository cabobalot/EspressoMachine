#ifndef MENU_H
#define MENU_H

#include <Wire.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "bitmaps.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64  // OLED 屏幕是 128x32
#define OLED_RESET -1

#define MAX_MENU_ITEMS 2  
#define MAX_VISIBLE_ITEMS 3  
#define MAX_MODE_ITEMS 3 // mode items
#define MAX_SETTING_ITEMS 5   // Temperature, Pressure, PreinfPress, PreinfTime, Back

enum MenuState {
    MAIN_MENU,
    MODE_PAGE,
    SETTING_PAGE,
    BREW_PAGE,//brew page
    STEAM_PAGE//steam page
};

class Menu {
private:
    //Adafruit_SSD1306 display;
    Adafruit_SH1106G display;
    int8_t listSelection = 0;  // 当前选中的菜单项
    int8_t scrollOffset = 0;   // 滚动偏移量
    const char* menuItems[MAX_MENU_ITEMS] = {"Mode", "Setting"};
    const char* modeMenuItems[MAX_MODE_ITEMS] = {"Steam", "Brew", "Back"};
    const char* settingMenuItems[MAX_SETTING_ITEMS] = {"Temperature", "Pressure", "PreinfPress", "PreinfTime", "Back"}; 
    MenuState currentState = MAIN_MENU; 
    //setter for pressure and temp
    uint8_t temperature = 70;
    float currentTemperature = 0; 
    float currentPressurePsi = 0.0;
    float targetPressurePsi = 40.0f;
    float   preinfPressurePsi = 6.0f;   // default 6
    uint16_t preinfTimeSec    = 5;      // default 5
    bool isEditingPreinfPressure = false;
    bool isEditingPreinfTime     = false;
    bool isEditingPressure    = false;
    bool isEditingTemperature = false;
    //count for brew screen
    unsigned long brewStartTime = 0;
    unsigned long lastFrameTime = 0;
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
    void  setTargetPressure(float psi) { targetPressurePsi = psi; }
    float getTargetPressure() const    { return targetPressurePsi; }
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
    //state control
    void setState(MenuState s);     // 仅切换页面状态，不直接绘制
    void resetBrewAnimation();   
      
private:
    void showSidebarInfo();
    void showMainMenu();
    void showModePage();
    void showSettingPage();
    void showTemperatureSetting();
    void showBrewPage();
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
    static constexpr uint16_t DEBOUNCE_MS = 30;
    bool _clicked=false;
};

extern Menu menu;

#endif
