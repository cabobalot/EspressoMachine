#include "Menu.h"
#include "pins.h" 

Menu menu;
Menu* Menu::_self = nullptr;
const char* Menu::NVS_NAMESPACE = "espresso";  // NVS命名空间

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
    }else if (s == WATER_PAGE) {               
        waterStartTime = millis();
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
    } else if (currentState == PROFILE_PAGE) {
        showProfilePage();
    } else if (currentState == BREW_PAGE){
        showBrewPage();
    } else if (currentState == STEAM_PAGE){
        showSteamPage();
    }else if (currentState == WATER_PAGE){
        showWaterPage();
    }

    display.display();
}

void Menu::moveSelection(bool up) {
    if (currentState == SETTING_PAGE && isEditingBrewTemperature) {
            if (up && brewTemperature < 100) brewTemperature++;
            if (!up && brewTemperature > 30) brewTemperature--;
            // 编辑过程中不保存，只在退出编辑模式时保存
            return;
        }
    
    if (currentState == SETTING_PAGE && isEditingSteamTemperature) {
            if (up && steamTemperature < 150) steamTemperature++;
            if (!up && steamTemperature > 50) steamTemperature--;
            // 编辑过程中不保存，只在退出编辑模式时保存
            return;
        }

    // Editing Brew Pressure
    if (currentState == SETTING_PAGE && isEditingPressureBrew) {
        if (up  && targetPressurePsiBrew < 150.0f)  targetPressurePsiBrew += 1.0f;
        if (!up && targetPressurePsiBrew >   0.0f)  targetPressurePsiBrew -= 1.0f;
        // 编辑过程中不保存，只在退出编辑模式时保存
        return;
    }
    // Editing Steam Pressure
    if (currentState == SETTING_PAGE && isEditingPressureSteam) {
        if (up  && targetPressurePsiSteam < 150.0f) targetPressurePsiSteam += 1.0f;
        if (!up && targetPressurePsiSteam >  0.0f)  targetPressurePsiSteam -= 1.0f;
        // 编辑过程中不保存，只在退出编辑模式时保存
        return;
    }
    // Editing: Preinf Pressure
    if (currentState == SETTING_PAGE && isEditingPreinfPressure) {
        if (up  && preinfPressurePsi < 150.0f) preinfPressurePsi += 0.5f;
        if (!up && preinfPressurePsi >   0.0f) preinfPressurePsi -= 0.5f;
        // 编辑过程中不保存，只在退出编辑模式时保存
        return;
    }
    // Editing: Preinf Time
    if (currentState == SETTING_PAGE && isEditingPreinfTime) {
        if (up  && preinfTimeSec < 60) preinfTimeSec += 1;
        if (!up && preinfTimeSec >  0) preinfTimeSec -= 1;
        // 编辑过程中不保存，只在退出编辑模式时保存
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
    //profile page
      else if (currentState == PROFILE_PAGE) {
        if (up) {
            if (listSelection > 0) {
                listSelection--;
            }
        } else {
            if (listSelection < MAX_PROFILE_ITEMS - 1) {
                listSelection++;
            }
        }
      }
 
    // 滚动逻辑（只对需要滚动的页面有效）
    // MODE_PAGE和SETTING_PAGE、PROFILE_PAGE不使用滚动（所有项目都能显示）
    if (currentState == MAIN_MENU) {
        if (listSelection < scrollOffset) {
            scrollOffset--;  // 向上滚动
        } else if (listSelection >= scrollOffset + MAX_VISIBLE_ITEMS) {
            scrollOffset++;  // 向下滚动
        }
    }
}
float Menu::getTargetTemperature() const {
    // 向后兼容：返回brew温度（默认）
    return (float)brewTemperature;
}
void Menu::setCurrentPressure(float psi) {
    currentPressurePsi = psi;
}
void Menu::setCurrentTemperature(float temp) {
    currentTemperature = temp;
}
void Menu::select() {
    if (currentState == MAIN_MENU) {
        if (listSelection == 2) {
            currentState = MODE_PAGE;
            listSelection = 0;
        } else if (listSelection == 0) {
            currentState = SETTING_PAGE;
            listSelection = 0;
        } else if (listSelection == 1) {
            currentState = PROFILE_PAGE;
            listSelection = 0;
        }
    } 
    else if (currentState == SETTING_PAGE) {
        if (listSelection == 0) {  // BrewTemp
            bool wasEditing = isEditingBrewTemperature;
            isEditingBrewTemperature = !isEditingBrewTemperature;
            if (isEditingBrewTemperature) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingSteamTemperature || isEditingPressureBrew || 
                    isEditingPressureSteam || isEditingPreinfPressure || 
                    isEditingPreinfTime) {
                    saveSettings();
                }
                isEditingSteamTemperature = false;
                isEditingPressureBrew  = false;
                isEditingPressureSteam = false;
                isEditingPreinfPressure = false;
                isEditingPreinfTime     = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 1) { // SteamTemp
            bool wasEditing = isEditingSteamTemperature;
            isEditingSteamTemperature = !isEditingSteamTemperature;
            if (isEditingSteamTemperature) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingBrewTemperature || isEditingPressureBrew || 
                    isEditingPressureSteam || isEditingPreinfPressure || 
                    isEditingPreinfTime) {
                    saveSettings();
                }
                isEditingBrewTemperature = false;
                isEditingPressureBrew  = false;
                isEditingPressureSteam = false;
                isEditingPreinfPressure = false;
                isEditingPreinfTime     = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 2) { // BrewPressure
            bool wasEditing = isEditingPressureBrew;
            isEditingPressureBrew = !isEditingPressureBrew;
            if (isEditingPressureBrew) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingBrewTemperature || isEditingSteamTemperature || 
                    isEditingPressureSteam || isEditingPreinfPressure || 
                    isEditingPreinfTime) {
                    saveSettings();
                }
                isEditingBrewTemperature = false;
                isEditingSteamTemperature = false;
                isEditingPressureSteam  = false;
                isEditingPreinfPressure = false;
                isEditingPreinfTime     = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 3) { // SteamPressure
            bool wasEditing = isEditingPressureSteam;
            isEditingPressureSteam = !isEditingPressureSteam;
            if (isEditingPressureSteam) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingBrewTemperature || isEditingSteamTemperature || 
                    isEditingPressureBrew || isEditingPreinfPressure || 
                    isEditingPreinfTime) {
                    saveSettings();
                }
                isEditingBrewTemperature = false;
                isEditingSteamTemperature = false;
                isEditingPressureBrew   = false;
                isEditingPreinfPressure = false;
                isEditingPreinfTime     = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 4) { // PreinfPress
            bool wasEditing = isEditingPreinfPressure;
            isEditingPreinfPressure = !isEditingPreinfPressure;
            if (isEditingPreinfPressure) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingBrewTemperature || isEditingSteamTemperature || 
                    isEditingPressureBrew || isEditingPressureSteam || 
                    isEditingPreinfTime) {
                    saveSettings();
                }
                isEditingBrewTemperature = false;
                isEditingSteamTemperature = false;
                isEditingPressureBrew   = false;
                isEditingPressureSteam  = false;
                isEditingPreinfTime     = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 5) { // PreinfTime
            bool wasEditing = isEditingPreinfTime;
            isEditingPreinfTime = !isEditingPreinfTime;
            if (isEditingPreinfTime) {
                // 进入编辑模式，先保存之前可能正在编辑的其他项
                if (isEditingBrewTemperature || isEditingSteamTemperature || 
                    isEditingPressureBrew || isEditingPressureSteam || 
                    isEditingPreinfPressure) {
                    saveSettings();
                }
                isEditingBrewTemperature = false;
                isEditingSteamTemperature = false;
                isEditingPressureBrew   = false;
                isEditingPressureSteam  = false;
                isEditingPreinfPressure = false;
            } else if (wasEditing) {
                // 退出编辑模式，保存设置
                saveSettings();
            }
        } else if (listSelection == 6) { // Default - 只重置当前profile为默认值
            // 只重置当前profile的值，不影响其他profile
            brewTemperature = 70;
            steamTemperature = 140;
            targetPressurePsiBrew = 40.0f;
            targetPressurePsiSteam = 25.0f;
            preinfPressurePsi = 20.0f;
            preinfTimeSec = 5;
            // 不改变currentProfileId，保持当前选中的profile
            // 保存重置后的值到当前profile
            saveSettings();
            Serial.println("Current profile reset to default values");
        } else if (listSelection == 7) { // Back
            // 退出设置页面时，如果还在编辑模式，先保存
            if (isEditingBrewTemperature || isEditingSteamTemperature || 
                isEditingPressureBrew || isEditingPressureSteam || 
                isEditingPreinfPressure || isEditingPreinfTime) {
                saveSettings();
            }
            currentState = MAIN_MENU;
            isEditingBrewTemperature = false;
            isEditingSteamTemperature = false;
            isEditingPressureBrew   = false;
            isEditingPressureSteam  = false;
            isEditingPreinfPressure = false;
            isEditingPreinfTime     = false;
            listSelection = 0;
            scrollOffset  = 0;
        }
    }
    else if (currentState == PROFILE_PAGE) {
        if (listSelection >= 0 && listSelection <= 3) {  // Profile 1-4
            uint8_t profileId = listSelection + 1;  // 1-4
            
            // 检查profile是否存在
            char key[32];
            snprintf(key, sizeof(key), "p%d_exists", profileId);
            bool profileExists = false;
            if (preferences.begin(NVS_NAMESPACE, true)) {
                profileExists = preferences.getBool(key, false);
                preferences.end();
            }
            
            // 如果profile不存在，这是第一次选中，需要创建它
            if (!profileExists) {
                // 使用默认值创建新profile
                brewTemperature = 70;
                steamTemperature = 140;
                targetPressurePsiBrew = 40.0f;
                targetPressurePsiSteam = 25.0f;
                preinfPressurePsi = 20.0f;
                preinfTimeSec = 5;
                // 设置currentProfileId为新profile
                currentProfileId = profileId;
                // 保存默认设置到新profile（创建profile）
                saveProfile(profileId);
                Serial.print("Profile ");
                Serial.print(profileId);
                Serial.println(" created for the first time with default values");
            } else {
                // Profile已存在，加载它
                currentProfileId = profileId;  // 先设置currentProfileId
                loadProfile(profileId);
            }
            
            saveCurrentProfileId();  // 保存当前profile ID
            currentState = MAIN_MENU;
            listSelection = 0;
            scrollOffset = 0;
        } else if (listSelection == 4) {  // Back
            currentState = MAIN_MENU;
            listSelection = 0;
            scrollOffset = 0;
        }
    }
    else if (currentState == MODE_PAGE) {
        if (listSelection == 0) {          // Steam
            currentState = STEAM_PAGE;     // ← 切到 Steam 页面
        } else if (listSelection == 1) {  // Brew
            currentState = BREW_PAGE;    // ← 切换状态
            brewStartTime = millis();
        } else if (listSelection == 2) {   // Water
            currentState = WATER_PAGE;     // <--- 新增
            waterStartTime = millis();
        } else if (listSelection == 3) {  
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
    // 显示所有4个mode选项，不使用滚动
    for (int i = 0; i < MAX_MODE_ITEMS; i++) {
        if (i == listSelection) {
            display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        } else {
            display.setTextColor(SH110X_WHITE);
        }
        display.println(modeMenuItems[i]);
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

        if (i == 0) {  // BrewTemp 
            display.print("BrewT: ");
            display.print(brewTemperature);
            display.println("C");
        } else if (i == 1) {       // SteamTemp
            display.print("SteamT: ");
            display.print(steamTemperature);
            display.println("C");
        } else if (i == 2) {       // BrewPressure
            display.print("BrewP: ");
            display.print(targetPressurePsiBrew, 0); //decimal
            display.println(" P");
        } else if (i == 3) {       // SteamPressure
            display.print("SteamP: ");
            display.print(targetPressurePsiSteam, 0);
            display.println(" P");
        } else if (i == 4) {  // PreinfPress
            display.print("PreP: ");
            display.print(preinfPressurePsi, 1);
            display.println(" P");
        } else if (i == 5) {  // PreinfTime
            display.print("PreT: ");
            display.print(preinfTimeSec);
            display.println(" S");
        } else if (i == 6) {  // Default
            display.println("Default");
        } else if (i == 7) {  // Back
            display.println("Back");
        }
    }
}

void Menu::showProfilePage() {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    
    for (int i = 0; i < MAX_PROFILE_ITEMS; i++) {
        if (i == listSelection) {
            display.setTextColor(SH110X_BLACK, SH110X_WHITE);  // 反色显示选中项
        } else {
            display.setTextColor(SH110X_WHITE);
        }
        
        // *标识表示当前选中的profile（currentProfileId）
        if (i >= 0 && i <= 3) {
            uint8_t profileId = i + 1;
            
            display.print(profileMenuItems[i]);
            // 只有当前选中的profile才显示*标识
            if (currentProfileId == profileId) {
                display.print(" *");  // 标记当前选中的档案
            }
            display.println();
        } else {
            display.println(profileMenuItems[i]);
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
    display.print(brewTemperature);
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
    display.setCursor(70, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.println("BREW");

    uint32_t sec = (now - brewStartTime) / 1000;
    display.setCursor(70, 12);
    display.print("Time: ");
    display.print(sec);
    display.println("s");

    display.setCursor(70, 24);
    display.print("Tar: ");
    display.print(brewTemperature);
    display.println("C");

    display.setCursor(60, 36);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");

    display.setCursor(60, 48);
    display.print("Cur :");
    display.print(currentPressurePsi);
    display.println("P");
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
    display.setCursor(70, 12);
    display.print("Time: ");
    display.print(sec);
    display.println("s");

    display.setCursor(70, 24);
    display.print("Tar: ");
    display.print(steamTemperature);
    display.println("C");

    display.setCursor(60, 36);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");

    display.setCursor(60, 48);
    display.print("Cur :");
    display.print(currentPressurePsi);
    display.println("P");
}

void Menu::showWaterPage() {
    unsigned long now = millis();
    if (now - lastFrameTime >= frameInterval) {
        currentFrame = (currentFrame + 1) % 2;
        lastFrameTime = now;
    }

    display.drawBitmap(0, 0, water, 58, 64, SH110X_WHITE);

    // Side bar
    display.setCursor(70, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.println("WATER");

    uint32_t sec = (now - waterStartTime) / 1000;
    display.setCursor(70, 12);
    display.print("Time: ");
    display.print(sec);
    display.println("s");

    display.setCursor(70, 24);
    display.print("Tar: ");
    display.print(brewTemperature);
    display.println("C");

    display.setCursor(60, 36);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");

    display.setCursor(60, 48);
    display.print("Cur :");
    display.print(currentPressurePsi);
    display.println("P");
}

// ==================== 设置持久化存储 ====================

void Menu::saveSettings() {
    // 保存到当前profile（如果当前是profile 0，则保存到默认设置）
    Serial.print("saveSettings() called, currentProfileId=");
    Serial.println(currentProfileId);
    Serial.print("Current values to save: brewTemp=");
    Serial.print(brewTemperature);
    Serial.print(", steamTemp=");
    Serial.print(steamTemperature);
    Serial.print(", brewP=");
    Serial.print(targetPressurePsiBrew);
    Serial.print(", steamP=");
    Serial.print(targetPressurePsiSteam);
    Serial.print(", preinfP=");
    Serial.print(preinfPressurePsi);
    Serial.print(", preinfT=");
    Serial.println(preinfTimeSec);
    saveProfile(currentProfileId);
    Serial.println("saveSettings() completed");
}

void Menu::loadSettings() {
    // 先加载上次使用的profile ID
    loadCurrentProfileId();
    Serial.print("loadSettings() - Loaded currentProfileId: ");
    Serial.println(currentProfileId);
    
    // 如果是第一次打开（currentProfileId为0或无效），使用profile 1
    if (currentProfileId == 0 || currentProfileId > 4) {
        currentProfileId = 1;
        Serial.println("First time opening, using profile 1");
    }
    
    // 加载对应的profile（如果不存在则使用默认值）
    loadProfile(currentProfileId);
}

void Menu::saveProfile(uint8_t profileId) {
    // profileId必须为1-4，不再使用profile 0
    if (profileId == 0 || profileId > 4) {
        Serial.print("Invalid profileId: ");
        Serial.println(profileId);
        return;
    }
    
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("Failed to open preferences for saving!");
        return;
    }
    
    // 构建键名：使用"pX_xxx"格式
    char key[32];
    
    Serial.print("Saving to profile ID: ");
    Serial.println(profileId);
    Serial.print("Values: brewTemp=");
    Serial.print(brewTemperature);
    Serial.print(", steamTemp=");
    Serial.print(steamTemperature);
    Serial.print(", brewP=");
    Serial.print(targetPressurePsiBrew);
    Serial.print(", steamP=");
    Serial.print(targetPressurePsiSteam);
    Serial.print(", preinfP=");
    Serial.print(preinfPressurePsi);
    Serial.print(", preinfT=");
    Serial.println(preinfTimeSec);
    
    // 保存用户档案（profileId 1-4）
    bool allSuccess = true;
    bool result;
    
    snprintf(key, sizeof(key), "p%d_brewTemp", profileId);
    result = preferences.putUChar(key, brewTemperature);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    snprintf(key, sizeof(key), "p%d_steamTemp", profileId);
    result = preferences.putUChar(key, steamTemperature);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    snprintf(key, sizeof(key), "p%d_brewP", profileId);
    result = preferences.putFloat(key, targetPressurePsiBrew);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    snprintf(key, sizeof(key), "p%d_steamP", profileId);
    result = preferences.putFloat(key, targetPressurePsiSteam);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    snprintf(key, sizeof(key), "p%d_preinfP", profileId);
    result = preferences.putFloat(key, preinfPressurePsi);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    snprintf(key, sizeof(key), "p%d_preinfT", profileId);
    result = preferences.putUShort(key, preinfTimeSec);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    // 标记档案存在
    snprintf(key, sizeof(key), "p%d_exists", profileId);
    result = preferences.putBool(key, true);
    if (!result) {
        Serial.print("ERROR: Failed to save ");
        Serial.println(key);
        allSuccess = false;
    }
    
    if (allSuccess) {
        Serial.print("Settings saved to profile ");
        Serial.println(profileId);
    } else {
        Serial.print("WARNING: Some values may have failed to save to profile ");
        Serial.println(profileId);
    }
    
    preferences.end();
    Serial.println("Preferences closed");
}

void Menu::loadProfile(uint8_t profileId) {
    // profileId必须为1-4，不再使用profile 0
    if (profileId == 0 || profileId > 4) {
        Serial.print("Invalid profileId for loading: ");
        Serial.println(profileId);
        // 使用默认值
        brewTemperature = 70;
        steamTemperature = 140;
        targetPressurePsiBrew = 40.0f;
        targetPressurePsiSteam = 25.0f;
        preinfPressurePsi = 20.0f;
        preinfTimeSec = 5;
        return;
    }
    
    Serial.print("loadProfile() called with profileId=");
    Serial.println(profileId);
    
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        Serial.println("Failed to open preferences for loading!");
        return;
    }
    
    char key[32];
    
    // 加载用户档案（profileId 1-4）
    snprintf(key, sizeof(key), "p%d_exists", profileId);
    bool profileExists = preferences.getBool(key, false);
    
    if (!profileExists) {
        // Profile不存在，使用默认值，但保持currentProfileId不变
        Serial.print("Profile ");
        Serial.print(profileId);
        Serial.println(" does not exist, using default values");
        
        // 使用默认值
        brewTemperature = 70;
        steamTemperature = 140;
        targetPressurePsiBrew = 40.0f;
        targetPressurePsiSteam = 25.0f;
        preinfPressurePsi = 20.0f;
        preinfTimeSec = 5;
        
        // 保持currentProfileId不变（这样在setting page显示的是当前profile）
        preferences.end();
        return;
    }
        
    // Profile存在，加载其值
    // 兼容旧数据：如果存在"p%d_temp"键，加载为brewTemp
    snprintf(key, sizeof(key), "p%d_temp", profileId);
    if (preferences.isKey(key)) {
        brewTemperature = preferences.getUChar(key, 70);
    } else {
        snprintf(key, sizeof(key), "p%d_brewTemp", profileId);
        brewTemperature = preferences.getUChar(key, 70);
    }
    
    snprintf(key, sizeof(key), "p%d_steamTemp", profileId);
    steamTemperature = preferences.getUChar(key, 140);
    
    snprintf(key, sizeof(key), "p%d_brewP", profileId);
    targetPressurePsiBrew = preferences.getFloat(key, 40.0f);
    
    snprintf(key, sizeof(key), "p%d_steamP", profileId);
    targetPressurePsiSteam = preferences.getFloat(key, 25.0f);
    
    snprintf(key, sizeof(key), "p%d_preinfP", profileId);
    preinfPressurePsi = preferences.getFloat(key, 20.0f);
    
    snprintf(key, sizeof(key), "p%d_preinfT", profileId);
    preinfTimeSec = preferences.getUShort(key, 5);
    
    // 不在这里设置currentProfileId，因为调用者已经设置了
    
    Serial.print("Settings loaded from profile ");
    Serial.println(profileId);
    
    preferences.end();
}

void Menu::resetToDefaults() {
    // 恢复所有设置为默认值（不改变currentProfileId）
    brewTemperature = 70;
    steamTemperature = 140;
    targetPressurePsiBrew = 40.0f;
    targetPressurePsiSteam = 25.0f;
    preinfPressurePsi = 20.0f;
    preinfTimeSec = 5;
    // 不改变currentProfileId，保持当前选中的profile
    
    Serial.println("Settings reset to defaults");
}

void Menu::markSettingsDirty() {
    settingsDirty = true;
    lastSaveTime = millis();
}

void Menu::checkAndSaveSettings() {
    if (!settingsDirty) {
        return;
    }
    
    // 如果距离上次修改超过延迟时间，则保存
    unsigned long now = millis();
    if (now - lastSaveTime >= SAVE_DELAY_MS) {
        saveSettings();
        settingsDirty = false;
        Serial.println("Settings saved (delayed)");
    }
}

void Menu::saveCurrentProfileId() {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("Failed to open preferences for saving profile ID!");
        return;
    }
    
    preferences.putUChar("currentProfileId", currentProfileId);
    preferences.end();
    
    Serial.print("Current profile ID saved: ");
    Serial.println(currentProfileId);
}

void Menu::loadCurrentProfileId() {
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        Serial.println("Failed to open preferences for loading profile ID!");
        return;
    }
    
    currentProfileId = preferences.getUChar("currentProfileId", 0);
    preferences.end();
    
    Serial.print("Current profile ID loaded: ");
    Serial.println(currentProfileId);
}

