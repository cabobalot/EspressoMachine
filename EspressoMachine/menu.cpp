#include "Menu.h"
#include "pins.h" 

Menu menu;
Menu* Menu::_self = nullptr;

Menu::Menu() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 800000UL, 800000UL) {}

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
    // 旋转事件累计
    if (_encFired) {
        _stepAccum += (_encUp ? +1 : -1);
        _encFired = false;
    }
    // 按键去抖（短按）
    if (!digitalRead(_pinBtn)) {           // 按下
        if (!_pressed) {
            _pressed = true;
            _debounceDeadline = millis() + 50;
        } else if (millis() > _debounceDeadline) {
            _clicked = true;               // 记一次点击
        }
    } else {
        _pressed = false;
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
    }
    
    display.display();
}
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
         else if (listSelection == 2) {  
            // 选中 Back 返回主菜单
            currentState = MAIN_MENU;
            isEditingTemperature = false;
            isEditingPressure    = false;
            listSelection = 0;
            scrollOffset = 0;
        }
    }
    else if (currentState == MODE_PAGE) {
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

        if (i == 0) {  //Temperature 
            display.print("Temp: ");
            display.print(temperature);
            display.println("C");
        } else if (i == 1) {  // Pressure
            display.print("Press: ");
            display.print(targetPressurePsi, 0);
            display.println(" PSI");
        } else if (i == 2) {  // Back
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
    display.print("Curr:");
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
    display.clearDisplay();
    // 显示图像（居中）
    display.drawBitmap(0, 0, frame, 58, 64, SH110X_WHITE);

    // 倒计时计算
    int secondsLeft = 30 - (now - brewStartTime) / 1000;
    if (secondsLeft <= 0) {
        currentState = MAIN_MENU;
        return;
    }

    // 显示倒计时 & 温度
    display.setCursor(70, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("Time: ");
    display.print(secondsLeft);
    display.println("s");

    display.setCursor(70, 15);
    display.print("Tar: ");
    display.print(temperature);
    display.println("C");

    display.setCursor(70, 30);
    display.print("Cur: ");
    display.print(currentTemperature);
    display.println("C");
}

