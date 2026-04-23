#pragma once

#include "Arduino.h"

#define LEFT_BUTTON 12
#define RIGHT_BUTTON 10
#define CENTER_BUTTON 11

enum class ButtonState : int {
  None,
  Down,
  Up,
  Hold
};

class Button {
  private:
    bool lastState = false;
    uint32_t timer = 0;
    uint32_t holdTimer;
    byte pin;
  public:
    uint32_t downDelay = 30; 
    uint32_t holdDelay = 300;

    Button(byte pin)
    {
      this->pin = pin;
      pinMode(pin, INPUT);
    }

    ButtonState FastUpdate(void)
    {
      bool state = digitalRead(pin);

      if (state && !lastState) {
        holdTimer = millis();
      }

      if (state && (millis() - holdTimer >= holdDelay)) {
          return ButtonState::Hold;
      }

      if (millis() - timer >= downDelay && lastState != state)
      {
        timer = millis();
        lastState = state;
        if (state) return ButtonState::Down;
        else return ButtonState::Up;
      }

      return ButtonState::None;
    }

    ButtonState Update(void)
    {
      bool state = digitalRead(pin);
    
      if (state && !lastState) {
        holdTimer = millis();
      }
    
      if (state) {
        if (millis() - holdTimer >= holdDelay) {
          lastState = state;
          return ButtonState::Hold;
        }
      
        lastState = state;
        return ButtonState::None;
      }
    
      if (!state && lastState) {
        if (millis() - holdTimer < holdDelay) {
          lastState = state;
          return ButtonState::Down;
        } else {
          lastState = state;
          return ButtonState::Up;
        }
      }
    
      lastState = state;
      return ButtonState::None;
    }
};

struct Input {
  ButtonState Left;
  ButtonState Right;
  ButtonState Center;

  Input(ButtonState left, ButtonState right, ButtonState center) : Left(left), Right(right), Center(center) {}
};

extern Button LeftButton;
extern Button CenterButton;
extern Button RightButton;

extern Input GetInput(void);
extern Input GetFastInput(void);