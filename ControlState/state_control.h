// state_control.h
#pragma once
#include <Arduino.h> 
#include "modes.h"

class Menu;  

class StateController {
public:
  explicit StateController(Menu& ui);

  void begin();                   // 只做按钮初始化 + 初始页面
  void update(uint32_t now);      // 只消费按钮事件；不做任何 update 调度
  void startBrew();               // 按下→进入 Brew（切页+复位动画）

  Mode mode() const { return mode_; }

private:
  void initButtons_();
  void handleButtons_(uint32_t now);

  Menu& ui_;
  Mode  mode_ = Mode::Idle;
};
