#include "joystick.h"

JoystickOutput readJoystick(int pinX, int pinY) {
    const int resolution = 0b1111111111;

    // In range [0 - resolution]

    int readX = analogRead(pinX);
    int readY = analogRead(pinY);

    // In range [0.0f - 1.0f]

    float normalizedX = readX / resolution;
    float normalizedY = readY / resolution;

    // In range [-1.0f - 1.0f]

    float coordinateX = ((normalizedX / 2.0f) - 0.5f) * 2.0f;
    float coordinateY = ((normalizedY / 2.0f) - 0.5f) * 2.0f;

    JoystickOutput jo;
    jo.x = coordinateX;
    jo.y = coordinateY;
    
    return jo;
}