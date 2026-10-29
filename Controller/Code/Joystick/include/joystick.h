#pragma once
#ifndef JOYSTICK_H
#define JOYSTICK_H

struct JoystickOutput {
    float x;
    float y;
};

JoystickOutput readJoystick(int pinX, int pinY);

#endif