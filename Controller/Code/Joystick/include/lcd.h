#pragma once
#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include "joystick.h"

enum class State {
    STOP,
    UP,
    DOWN
};

LiquidCrystal_AIP31068_I2C lcd;

uint8_t bell[8];
uint8_t note[8];
uint8_t clock[8];
uint8_t heart[8];
uint8_t duck[8];
uint8_t check[8];
uint8_t cross[8];
uint8_t retarrow[8];

void setup_lcd();

void update_joystick(JoystickOutput);
void update_moving(State);

#endif