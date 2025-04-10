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
#define MAX_SETTING_ITEMS 2 

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
    const char* settingMenuItems[MAX_SETTING_ITEMS] = {"Temperature","Back"}; 
    uint8_t temperature = 70;
    float currentTemperature = 0; 
    float currentPressurePsi = 0.0;
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
    void setCurrentTemperature(float temp);
    void setCurrentPressure(float psi);

private:
    void showSidebarInfo();
    void showMainMenu();
    void showModePage();
    void showSettingPage();
    void showTemperatureSetting();
    void showBrewPage();
};

extern Menu menu;

#endif
