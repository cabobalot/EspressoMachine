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

#define MAX_MENU_ITEMS 3  // Mode, Setting, Profile
#define MAX_VISIBLE_ITEMS 3  
#define MAX_MODE_ITEMS 4 // mode items
#define MAX_SETTING_ITEMS 8   // BrewTemp, SteamTemp, BrewPressure, SteamPressure, PreinfPress, PreinfTime, Default, Back
#define MAX_PROFILE_ITEMS 5   // Profile 1, Profile 2, Profile 3, Profile 4, Back

enum MenuState {
    MAIN_MENU,
    MODE_PAGE,
    SETTING_PAGE,
    PROFILE_PAGE,  // 档案选择页面
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
    const char* menuItems[MAX_MENU_ITEMS] = {"Setting", "Profile", "Mode"};
    const char* modeMenuItems[MAX_MODE_ITEMS] = {"Steam", "Brew", "Water", "Back"};
    const char* settingMenuItems[MAX_SETTING_ITEMS] = {"BrewTemp", "SteamTemp", "BrewPressure", "SteamPressure","PreinfPress", "PreinfTime", "Default", "Back"};
    const char* profileMenuItems[MAX_PROFILE_ITEMS] = {"Profile 1", "Profile 2", "Profile 3", "Profile 4", "Back"}; 
    MenuState currentState = MAIN_MENU; 
    //setter for pressure and temp
    uint8_t brewTemperature = 70;      // Brew模式的目标温度
    uint8_t steamTemperature = 140;     // Steam模式的目标温度（默认140度）
    float currentTemperature = 0; 
    float currentPressurePsi = 0.0;
    //Default Value
    float targetPressurePsiBrew  = 40.0f;  // Default for brew pressure
    float targetPressurePsiSteam = 25.0f;  // Default for steam pressure
    float   preinfPressurePsi = 20.0f;   // default 20 PSI
    uint16_t preinfTimeSec    = 5;      // default 5
    bool isEditingPreinfPressure = false;
    bool isEditingPreinfTime     = false;
    //Editing pressure
    bool  isEditingPressureBrew  = false;
    bool  isEditingPressureSteam = false;
    bool isEditingBrewTemperature = false;
    bool isEditingSteamTemperature = false;
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
    float getTargetTemperature() const;  // 根据当前模式返回对应温度（向后兼容）
    float getTargetTemperatureBrew() const { return (float)brewTemperature; }
    float getTargetTemperatureSteam() const { return (float)steamTemperature; }
    void setTargetTemperatureBrew(uint8_t temp) { brewTemperature = temp; }
    void setTargetTemperatureSteam(uint8_t temp) { steamTemperature = temp; }
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
    void saveSettings();            // 保存当前设置到NVS（立即保存）
    void markSettingsDirty();       // 标记设置已修改（延迟保存）
    void loadSettings();             // 从NVS加载设置
    void checkAndSaveSettings();    // 检查并保存（如果dirty）
    // 用户档案管理
    void saveProfile(uint8_t profileId);  // 保存当前设置到指定档案
    void loadProfile(uint8_t profileId);  // 从指定档案加载设置（不存在时使用默认值）
    void resetToDefaults();  // 恢复默认设置值
    uint8_t getCurrentProfileId() const { return currentProfileId; }
    void setCurrentProfileId(uint8_t id) { currentProfileId = id; }
    void saveCurrentProfileId();   // 保存当前profile ID
    void loadCurrentProfileId();    // 加载当前profile ID
      
private:
    void showSidebarInfo();
    void showMainMenu();
    void showModePage();
    void showSettingPage();
    void showTemperatureSetting();
    void showProfilePage();  // 显示档案选择页面
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
    bool settingsDirty = false;        // 标记设置是否被修改（用于延迟保存）
    unsigned long lastSaveTime = 0;    // 上次保存时间
    static constexpr unsigned long SAVE_DELAY_MS = 2000;  // 延迟保存时间（2秒无修改后保存）
};

extern Menu menu;

#endif
