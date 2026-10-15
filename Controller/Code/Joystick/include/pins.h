
#pragma once
#ifndef PINS_H
#define PINS_H

// --------------------------------------------------
//                  INPUT PINS
// --------------------------------------------------

/*
    
    ENABLE_IN_PIN

    Description:
    Enables the robot when switch is high
    
    Type: Digitial
    Pull Behavior: None

*/
extern int ENABLE_IN_PIN;
/*
    
    ESTOP_IN_PIN

    Description:
    Emergency stops the robot when button is low
    
    Type: Digitial
    Pull Behavior: Pull-Up

*/
extern int ESTOP_IN_PIN;
/*
    
    JOYSTICK_IN_PIN

    Description:
    Controls the movement of the robot
    
    Type: Analog

*/
extern int JOYSTICK_IN_PIN;

// --------------------------------------------------
//                  OUTPUT PINS
// --------------------------------------------------

/*
    
    LED_OUT_PIN

    Description:
    Lights an LED when robot is enabled
    
    Type: Digital

*/
extern int LED_OUT_PIN;

#endif