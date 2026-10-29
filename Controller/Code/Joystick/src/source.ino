#include "pins.h"
#include "joystick.h"
#include "lcd.h"

void setup() {
    // put your setup code here, to run once:
    pinMode(DI_ENABLE,INPUT);
    pinMode(DI_ESTOP,INPUT_PULLUP);
    pinMode(AI_VJOYSTICK1,INPUT);
    pinMode(AI_HJOYSTICK1,INPUT);
    pinMode(DO_ENABLE,OUTPUT);
    analogReadResolution(10);
    setup_lcd();
}

void loop() {
    // put your main code here, to run repeatedly:
    if(digitalRead(DI_ENABLE)) {
        digitalWrite(DO_ENABLE,1);
    } else {
        digitalWrite(DO_ENABLE,0);
    }

    JoystickOutput current = readJoystick(AI_HJOYSTICK1,AI_VJOYSTICK1);

    update_joystick(current);
    update_moving(State::STOP);
}
