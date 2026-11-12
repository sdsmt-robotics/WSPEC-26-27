#include <Arduino.h>
#include <ESP32Servo.h>

Servo Motor_1;
Servo Motor_2;
Servo Motor_3;
Servo Motor_4;

const int Motor_1_PIN = 35; // Connected to 'SIN' on the receiver cable
const int Motor_2_PIN = 47; // Connected to 'SOUT' on the receiver cable
const int Motor_3_PIN = 3; // Connected to 'SOUT' on the receiver cable
const int Motor_4_PIN = 13; // Connected to 'SOUT' on the

void write_Motors(int motor_1_value, int motor_2_value, int motor_3_value, int motor_4_value, int duration_ms);

void write_Motors(int motor_1_value, int motor_2_value, int motor_3_value, int motor_4_value, int duration_ms) {
  // Use Arduino's built-in delay function

  Motor_1.writeMicroseconds(motor_1_value);
  Motor_2.writeMicroseconds(motor_2_value); 
  Motor_3.writeMicroseconds(motor_3_value);
  Motor_4.writeMicroseconds(motor_4_value);
  delay(duration_ms);
}

void setup() {
  Serial.begin(115200);

  // Allocate hardware PWM timers for ESP32
  ESP32PWM::allocateTimer(0);
  Motor_1.setPeriodHertz(50); // Standard 50Hz RC Servo frequency
  Motor_2.setPeriodHertz(50);
  Motor_3.setPeriodHertz(50);
  Motor_4.setPeriodHertz(50);

  // Attach pin with standard RC pulse limits: 1000us (min) to 2000us (max)
  Motor_1.attach(Motor_1_PIN, 1000, 2000);
  Motor_2.attach(Motor_2_PIN, 1000, 2000);
  Motor_3.attach(Motor_3_PIN, 1000, 2000);
  Motor_4.attach(Motor_4_PIN, 1000, 2000);

  // Send neutral signal (1500us = center/stop) on boot
  Motor_1.writeMicroseconds(1500);
  Motor_2.writeMicroseconds(1500);
  Motor_3.writeMicroseconds(1500);
  Motor_4.writeMicroseconds(1500);
  delay(2000); // Allow VESC to initialize
}

void loop() {

  Serial.println("Low Throttle forward");
  write_Motors(1650, 1650, 1650, 1650, 5000);

  Serial.println("Stopping");
  write_Motors(1500, 1500, 1500, 1500, 5000);

  Serial.println("Low Throttle Backward");
  //write_Motors(1400, 1400, 1400, 1400, 5000);


  Serial.println("Stopping");
  write_Motors(1500, 1500, 1500, 1500, 5000);

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
//  

//   VESC.setDuty(0.25);       // 25% Duty Cycle
//  // Alternative Options:
//  // VESC.setCurrent(5.0);     // 5 Amps motor current
//  // VESC.setBrakeCurrent(2.0); // 2 Amps regenerative braking
//
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