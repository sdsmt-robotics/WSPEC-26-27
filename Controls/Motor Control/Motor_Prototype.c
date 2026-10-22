#include <Arduino.h>
#include <ESP32Servo.h>

Servo vescControl;

const int VESC_PIN = 18; // Connected to 'SIN' on the receiver cable

void setup() {
  Serial.begin(115200);

  // Allocate hardware PWM timers for ESP32
  ESP32PWM::allocateTimer(0);
  vescControl.setPeriodHertz(50); // Standard 50Hz RC Servo frequency

  // Attach pin with standard RC pulse limits: 1000us (min) to 2000us (max)
  vescControl.attach(VESC_PIN, 1000, 2000);

  // Send neutral signal (1500us = center/stop) on boot
  vescControl.writeMicroseconds(1500);
  delay(2000); // Allow VESC to initialize
}

void loop() {
  Serial.println("Motor Neutral");
  vescControl.writeMicroseconds(1500);
  delay(3000);


  Serial.println("Low Throttle Backward");
  vescControl.writeMicroseconds(1600);
  delay(3000);

  Serial.println("Low Throttle Forward");
  vescControl.writeMicroseconds(1400);
  delay(3000);

  Serial.println("Half Throttle Forward");
  vescControl.writeMicroseconds(1750);
  delay(3000);

    Serial.println("Half Throttle Backward");
  vescControl.writeMicroseconds(1250);
  delay(3000);

  Serial.println("Stopping");
  vescControl.writeMicroseconds(1500);
  delay(3000);
}




//#include <Arduino.h>
//#include <VescUart.h>
//
//// Instantiate the VESC UART object
//VescUart VESC;
//
//// Define hardware serial pins for ESP32 (UART2)
//#define RXD2 16 // Connected to VESC TX
//#define TXD2 17 // Connected to VESC RX
//
//// REV NEO 1.1 Motor Specs
//const int MOTOR_POLE_PAIRS = 7; // 14 poles = 7 pole pairs
//
//void setup() {
//  // Debug Serial output to PC
//  Serial.begin(115200);
//
//  // Initialize Hardware Serial 2 on ESP32 at 115200 baud
//  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
//
//  // Pass the hardware serial reference to VescUart
//  VESC.setSerialPort(&Serial2);
//
//  Serial.println("VESC UART Communication Initialized.");
//  delay(1000);
//}
//
//void loop() {
//  // ----------------------------------------------------
//  // 1. CONTROL COMMANDS
//  // ----------------------------------------------------
//  
//  // Set target speed in Electrical RPM (ERPM)
//  // Target: 1000 Mechanical RPM -> (1000 * 7 = 7000 ERPM)
//  float targetMechanicalRPM = 1000.0;
//  float targetERPM = targetMechanicalRPM * MOTOR_POLE_PAIRS;
//  
//  VESC.setRPM(targetERPM);
//
//  // Alternative Control Methods:
//  // VESC.setDuty(0.25);       // 25% Duty Cycle
//  // VESC.setCurrent(5.0);     // 5 Amps motor current
//  // VESC.setBrakeCurrent(2.0); // 2 Amps regenerative braking
//
//  // ----------------------------------------------------
//  // 2. TELEMETRY READOUT
//  // ----------------------------------------------------
//  
//  // Request fresh packet from VESC (Returns true if successful)
//  if (VESC.getVescValues()) {
//    float currentERPM = VESC.data.rpm;
//    float mechanicalRPM = currentERPM / MOTOR_POLE_PAIRS;
//
//    Serial.println("--- VESC TELEMETRY ---");
//    Serial.print("Mechanical Speed : "); Serial.print(mechanicalRPM); Serial.println(" RPM");
//    Serial.print("Input Voltage    : "); Serial.print(VESC.data.inpVoltage); Serial.println(" V");
//    Serial.print("Motor Current    : "); Serial.print(VESC.data.avgMotorCurrent); Serial.println(" A");
//    Serial.print("Battery Current  : "); Serial.print(VESC.data.avgInputCurrent); Serial.println(" A");
//    Serial.print("MOSFET Temp      : "); Serial.print(VESC.data.tempMosfet); Serial.println(" °C");
//    Serial.print("Motor Temp       : "); Serial.print(VESC.data.tempMotor); Serial.println(" °C");
//    Serial.print("Energy Consumed  : "); Serial.print(VESC.data.ampHours); Serial.println(" Ah");
//    Serial.print("Fault Code       : "); Serial.println(VESC.data.faultCode);
//    Serial.println("----------------------\n");
//  } else {
//    Serial.println("Error: Failed to receive packet from VESC.");
//  }
//
//  delay(100); // 10 Hz refresh rate
//}