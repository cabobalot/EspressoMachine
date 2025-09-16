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
#define MAX_SETTING_ITEMS 3

enum MenuState {
    MAIN_MENU,
    MODE_PAGE,
    SETTING_PAGE,
    BREW_PAGE
};

class Menu {
private:
    //Adafruit_SSD1306 display;
    Adafruit_SH1106G display;
    int8_t listSelection = 0;  // 当前选中的菜单项
    int8_t scrollOffset = 0;   // 滚动偏移量
    MenuState currentState = MAIN_MENU;
    const char* menuItems[MAX_MENU_ITEMS] = {"Mode", "Setting"};
    const char* modeMenuItems[MAX_MODE_ITEMS] = {"Steam", "Brew", "Back"};
    const char* settingMenuItems[MAX_SETTING_ITEMS] = {"Temperature","Pressure","Back"}; 
    //setter for pressure and temp
    uint8_t temperature = 70;
    float currentTemperature = 0; 
    float currentPressurePsi = 0.0;
    float targetPressurePsi = 40.0f;
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

    // select
    unsigned long _debounceDeadline=0;
    bool _pressed=false;
    bool _clicked=false;
};

extern Menu menu;

#endif
