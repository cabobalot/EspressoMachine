#include "Menu.h"
#include "pins.h" 

Menu menu;
Menu* Menu::_self = nullptr;

Menu::Menu() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

bool Menu::begin() {
    if (!display.begin(0x3C, true)) {
        return false;
    }
    display.cp437(true);
    return true;
}
bool Menu::beginInput(int pinA, int pinB, int pinBtn) {
    _self = this;
    _pinA = pinA; _pinB = pinB; _pinBtn = pinBtn;

    pinMode(_pinA, INPUT);
    pinMode(_pinB, INPUT);
    pinMode(_pinBtn, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(_pinA), Menu::isrAWrapper, CHANGE);
    return true;
}

void IRAM_ATTR Menu::isrAWrapper() {
    if (_self) _self->onAChangeISR();
}

void IRAM_ATTR Menu::onAChangeISR() {
    bool a = digitalRead(_pinA);
    bool b = digitalRead(_pinB);
    _encUp   = a ? b : !b;
    _encFired = true;
}

void Menu::pollInput() {
    // === 旋钮累计（保留原逻辑） ===
    if (_encFired) {
        _stepAccum += (_encUp ? +1 : -1);
        _encFired = false;
    }

    // === 按钮：边沿 + 去抖，只在稳定“抬起->按下”瞬间计一次 ===
    unsigned long now = millis();
    bool rawPressed = !digitalRead(_pinBtn);   // 上拉输入：低电平=按下

    // 记录原始读数发生边沿的时刻
    if (rawPressed != _btnPrevRaw) {
        _btnPrevRaw    = rawPressed;
        _btnEdgeTimeMs = now;
    }

    // 原始状态稳定超过去抖时间，才更新“稳定状态”
    if ((now - _btnEdgeTimeMs) >= DEBOUNCE_MS ) {
        if (rawPressed != _btnStable) {
            _btnStable = rawPressed;

            if (_btnStable) {               // 稳定地从“未按”->“按下”
                if (!_btnLatched) {         // 未计过数才计数
                    _clicked = true;        // 一次性点击事件（供上层 consumeClick() 读取）
                    _btnLatched = true;     // 锁存，直到松开前不再重复计数
                    _pressCount++;

                    // 可选：串口统计
                    Serial.print("Button press #");
                    Serial.println(_pressCount);
                }
            } else {                         // 稳定地从“按下”->“松开”
                _btnLatched = false;        // 解锁，允许下一次按下再计数
            }
        }
    }
}

int Menu::consumeStep() {
    int s = _stepAccum;
    _stepAccum = 0;
    return s;
}

bool Menu::consumeClick() {
    bool c = _clicked;
    _clicked = false;
    return c;
}
//State control
//press the button to change state
void Menu::setState(MenuState s) {
    currentState = s;
    if (s == STEAM_PAGE) {
        steamStartTime = millis();
        lastFrameTime = 0;
        currentFrame = 0;
    }else if (s == BREW_PAGE) {
    brewStartTime = millis();
    lastFrameTime = 0;
    currentFrame  = 0;
  }
}

void Menu::resetBrewAnimation() {
    brewStartTime  = millis();
    lastFrameTime  = 0;
    currentFrame   = 0;
}
void Menu::show() {
    display.clearDisplay();
    if (currentState == MAIN_MENU) {
        showMainMenu();
        showSidebarInfo();
    } else if (currentState == MODE_PAGE) {
        showModePage();
    } else if (currentState == SETTING_PAGE) {
        showSettingPage();
        showSidebarInfo();
    } else if (currentState == BREW_PAGE){
        showBrewPage();
    } else if (currentState == STEAM_PAGE){
        showSteamPage();
    }

    display.display();
}

void Menu::moveSelection(bool up) {
    if (currentState == SETTING_PAGE && isEditingTemperature) {
            if (up && temperature < 100) temperature++;
            if (!up && temperature > 30) temperature--;
            return;
        }

    if (currentState == SETTING_PAGE && isEditingPressure) {
        if (up  && targetPressurePsi < 150.0f) targetPressurePsi += 1.0f;
        if (!up && targetPressurePsi >   0.0f) targetPressurePsi -= 1.0f;
        return;
    }
    // Editing: Preinf Pressure
    if (currentState == SETTING_PAGE && isEditingPreinfPressure) {
        if (up  && preinfPressurePsi < 150.0f) preinfPressurePsi += 0.5f;
        if (!up && preinfPressurePsi >   0.0f) preinfPressurePsi -= 0.5f;
        return;
    }
    // Editing: Preinf Time
    if (currentState == SETTING_PAGE && isEditingPreinfTime) {
        if (up  && preinfTimeSec < 60) preinfTimeSec += 1;
        if (!up && preinfTimeSec >  0) preinfTimeSec -= 1;
        return;
    }

  //main page
    if (currentState == MAIN_MENU) {
        if (up) {
            if (listSelection > 0) {
                listSelection--;
            }
        } else {
            if (listSelection < MAX_MENU_ITEMS - 1) {
                listSelection++;
            }
        }
    }
  //mode page
    else if (currentState == MODE_PAGE) { 
        if (up) {
            if (listSelection > 0) {
                listSelection--;
            }
        } else {
            if (listSelection < MAX_MODE_ITEMS - 1) {
                listSelection++;
            }
        }
    }
    //setting page
      else if (currentState == SETTING_PAGE) {  
        if (up) {
            if (listSelection > 0) {
                listSelection--;
            }
        } else {
            if (listSelection < MAX_SETTING_ITEMS - 1) {
                listSelection++;
            }
        }
      }
 
    if (listSelection < scrollOffset) {
        scrollOffset--;  // 向上滚动
    } else if (listSelection >= scrollOffset + MAX_VISIBLE_ITEMS) {
        scrollOffset++;  // 向下滚动
    }
}
float Menu::getTargetTemperature() const {
    return (float)temperature;
}
void Menu::setCurrentPressure(float psi) {
    currentPressurePsi = psi;
}
void Menu::setCurrentTemperature(float temp) {
    currentTemperature = temp;
}
void Menu::select() {
    if (currentState == MAIN_MENU) {
        if (listSelection == 0) {
            currentState = MODE_PAGE;
            listSelection = 0;
        } else if (listSelection == 1) {
            currentState = SETTING_PAGE;
            listSelection = 0;
        }
    } 
    else if (currentState == SETTING_PAGE) {
        if (listSelection == 0) {  
            // 选中 Temperature 进入温度调整模式
            isEditingTemperature = !isEditingTemperature;  // 进入/退出编辑模式
            if (isEditingTemperature) isEditingPressure = false; 
        }else if (listSelection == 1){
            isEditingPressure = !isEditingPressure;
            if (isEditingPressure) isEditingTemperature = false; // 互斥
        }
         else if (listSelection == 2) { // PreinfPress
        isEditingPreinfPressure = !isEditingPreinfPressure;
        if (isEditingPreinfPressure) {
            isEditingTemperature = false;
            isEditingPressure = false;
            isEditingPreinfTime = false;
        }
        } else if (listSelection == 3) { // PreinfTime
            isEditingPreinfTime = !isEditingPreinfTime;
            if (isEditingPreinfTime) {
                isEditingTemperature = false;
                isEditingPressure = false;
                isEditingPreinfPressure = false;
            }
        } 
         else if (listSelection == 4) {  
            // 选中 Back 返回主菜单
            currentState = MAIN_MENU;
            isEditingTemperature = false;
            isEditingPressure    = false;
            listSelection = 0;
            scrollOffset = 0;
        }
    }
    else if (currentState == MODE_PAGE) {
        if (listSelection == 0) {          // Steam
        currentState = STEAM_PAGE;     // ← 切到 Steam 页面
        // lastFrameTime = 0;
        // currentFrame  = 0;
        }
        if (listSelection == 1) {  // Brew
        currentState = BREW_PAGE;    // ← 切换状态
        brewStartTime = millis();
        } 
        else if (listSelection == 2) {  
            // 选中 Back 返回主菜单
            currentState = MAIN_MENU;
            listSelection = 0;
            scrollOffset = 0;
        }
    }
}
//Show child page
void Menu::showMainMenu() {
    display.setCursor(0, 0);
    display.setTextSize(1);
    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        int itemIndex = scrollOffset + i;  // 计算当前要显示的菜单项索引

        if (itemIndex < MAX_MENU_ITEMS) {
            if (itemIndex == listSelection) {
                display.setTextColor(SH110X_BLACK, SH110X_WHITE);  // 选中项反色显示
            } else {
                display.setTextColor(SH110X_WHITE);
            }
            display.println(menuItems[itemIndex]);
        }
    }
}
void Menu::showModePage() {
    display.setCursor(0, 0);
    display.setTextSize(1);
    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {  
        int itemIndex = scrollOffset + i;  
        if (itemIndex < MAX_MODE_ITEMS) {
            if (itemIndex == listSelection) {
                display.setTextColor(SH110X_BLACK, SH110X_WHITE);
            } else {
                display.setTextColor(SH110X_WHITE);
            }
            display.println(modeMenuItems[itemIndex]);
        }
    }
}

void Menu::showSettingPage() {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    for (int i = 0; i < MAX_SETTING_ITEMS; i++) {
        if (i == listSelection) {
            display.setTextColor(SH110X_BLACK, SH110X_WHITE);  // 反色显示选中项
        } else {
            display.setTextColor(SH110X_WHITE);
        }

        if (i == 0) {  // Temperature 
            display.print("Temp: ");
            display.print(temperature);
            display.println("C");
        } else if (i == 1) {  // Pressure
            display.print("Press: ");
            display.print(targetPressurePsi, 0);
            display.println(" P");
        } else if (i == 2) {  // PreinfPress
            display.print("PreP: ");
            display.print(preinfPressurePsi, 1);
            display.println(" p");
        } else if (i == 3) {  // PreinfTime
            display.print("PreT: ");
            display.print(preinfTimeSec);
            display.println(" s");
        } else if (i == 4) {  // Back
            display.println("Back");
        }
    }
}

void Menu::showSidebarInfo() {
    display.setTextColor(SH110X_WHITE); 
    display.drawLine(70, 0, 70, SCREEN_HEIGHT, SH110X_WHITE);
    display.setCursor(73, 0);
    display.print("Temp");

    display.setCursor(73, 10);
    display.print("Tar:");
    display.print(temperature);
    display.println("C");

    display.setCursor(73, 20);
    display.print("C:");
    display.print(currentTemperature);
    display.println("C");

    display.drawLine(70, 30, SCREEN_WIDTH, 30, SH110X_WHITE);

    display.setTextColor(SH110X_WHITE);
    display.setCursor(73, 40);
    display.print("PSI:");
    display.print(currentPressurePsi, 1);
}
void Menu::showBrewPage() {
    unsigned long now = millis();
    if (now - lastFrameTime > frameInterval) {
        currentFrame = (currentFrame + 1) % 2;  // 两帧循环切换
        lastFrameTime = now;
    }

    const unsigned char* frame = (currentFrame == 0) ? brew_coffee__ : brew_coffee__1;
    // 显示图像（居中）
    display.drawBitmap(0, 0, frame, 58, 64, SH110X_WHITE);

    // 倒计时计算
    uint32_t sec = (now - brewStartTime) / 1000;

    // 显示倒计时 & 温度
    display.setCursor(70, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("Time: ");
    display.print(sec);
    display.println("s");

    display.setCursor(70, 15);
    display.print("Tar: ");
    display.print(temperature);
    display.println("C");

    display.setCursor(60, 30);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");
}

void Menu::showSteamPage() {
    unsigned long now = millis();
    if (now - lastFrameTime >= frameInterval) {
        currentFrame = (currentFrame + 1) % 2;
        lastFrameTime = now;
    }

    const unsigned char* frame = (currentFrame == 0) ? steam_1 : steam_2;
    display.drawBitmap(0, 0, frame, 58, 64, SH110X_WHITE);

    display.setCursor(70, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.println("STEAM");

    uint32_t sec = (now - steamStartTime) / 1000;
    display.setCursor(70, 15);
    display.print("Time: ");
    display.print(sec);
    display.println("s");

    display.setCursor(70, 30);
    display.print("Tar: ");
    display.print(temperature);
    display.println("C");

    display.setCursor(60, 45);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");
}


