#pragma once
#include "Arduino.h"

#define BAT_PIN 4

enum class Location {
  FileSelecter,
  Server,
  File,
  Game
};

extern Location UserLocation;
extern void processBat(void);