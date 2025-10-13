#include "state_control.h"
#include "Menu.h"
#include "pins.h"
#include <Arduino.h>

#ifndef PIN_BTN_BREW
#define PIN_BTN_BREW PIN_SWITCH_BREW
#endif

static volatile bool s_brew_down_edge = false;
static uint32_t      s_last_isr_ms     = 0;

void IRAM_ATTR isrBrewBtn() {
  uint32_t now = millis();
  if (now - s_last_isr_ms < 30) return;   // 30ms 去抖
  s_last_isr_ms = now;
  if (digitalRead(PIN_BTN_BREW) == LOW) { // 上拉输入：LOW=按下
    s_brew_down_edge = true;
  }
}

StateController::StateController(Menu& ui) : ui_(ui) {}

void StateController::begin() {
  initButtons_();
  mode_ = Mode::Idle;
  ui_.setState(MAIN_MENU);
}

void StateController::initButtons_() {
  pinMode(PIN_BTN_BREW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_BREW), isrBrewBtn, CHANGE);
}

void StateController::update(uint32_t /*now*/) {
  handleButtons_(0);
}

void StateController::handleButtons_(uint32_t /*now*/) {
  if (s_brew_down_edge) {
    s_brew_down_edge = false;
    startBrew();
  }
}

void StateController::startBrew() {
  if (mode_ == Mode::Brew) return;
  mode_ = Mode::Brew;
  ui_.setState(BREW_PAGE);
  ui_.resetBrewAnimation();
}
