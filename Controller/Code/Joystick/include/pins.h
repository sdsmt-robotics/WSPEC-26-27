#pragma once
#ifndef PINS_H
#define PINS_H

// --------------------------------------------------
//                  INPUT PINS
// --------------------------------------------------

/*
    ENABLE

    Description:
    Enables the robot when switch is high

    Type: Digitial
    Pull Behavior: None

*/
int DI_ENABLE;

/*
    ESTOP

    Description:
    Sends emergency stop command to robot when low

    Type: Digitial
    Pull Behavior: Up

*/
int DI_ESTOP;

/*
    VJOYSTICK1

    Description:
    Provides vertical component for joystick 1

    Type: Analog

*/
int AI_VJOYSTICK1;

/*
    HJOYSTICK1

    Description:
    Provides horizontal component for joystick 1

    Type: Analog

*/
int AI_HJOYSTICK1;

// --------------------------------------------------
//                  OUTPUT PINS
// --------------------------------------------------

/*
    ENABLE

    Description:
    Lights the led when robot is enabled

    Type: Digitial
    Pull Behavior: None

*/
int DO_ENABLE;

// --------------------------------------------------
//                  PROTOCOL
// --------------------------------------------------

/*
    16X2

    Description:
    LCD output of the controller

    Protocol: I2C
    Type: SCL
*/
int I2C_SCL_16X2;

/*
    16X2

    Description:
    LCD output of the controller

    Protocol: I2C
    Type: SDA
*/
int I2C_SDA_16X2;

#endif