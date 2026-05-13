/*
  NOTE: Unplug pin 0 (RX) when uploading. Reconnect to ESP32 GPIO17 after.
*/

#include <Servo.h>

Servo leftDrive;
Servo rightDrive;
Servo panServo;


const int STOP_US = 1500;
const int FAST_US = 200;
const int SLOW_US = 80;
const int LEFT_PIN  = 5;
const int RIGHT_PIN = 6;
const int PAN_PIN   = 3;
const int PAN_CENTER = 90;
const int PAN_LEFT   = 50;
const int PAN_RIGHT  = 130;

char currentCmd = 'S';

void driveMotors(int leftOffset, int rightOffset) {
  leftDrive.writeMicroseconds(STOP_US + leftOffset);
  rightDrive.writeMicroseconds(STOP_US - rightOffset);
}

void stopMotors() {
  leftDrive.writeMicroseconds(STOP_US);
  rightDrive.writeMicroseconds(STOP_US);
}

void updatePan(char cmd) {
  switch (cmd) {
    case 'L': panServo.write(PAN_LEFT);   break;
    case 'R': panServo.write(PAN_RIGHT);  break;
    default:  panServo.write(PAN_CENTER); break;
  }
}

void setup() {
  Serial.begin(9600);

  leftDrive.attach(LEFT_PIN);
  rightDrive.attach(RIGHT_PIN);
  panServo.attach(PAN_PIN);

  stopMotors();
  panServo.write(PAN_CENTER);
  delay(500);
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'F' || c == 'B' || c == 'L' ||
        c == 'R' || c == 'S' || c == 'A') {
      currentCmd = c;
      updatePan(c);
    }
  }

  switch (currentCmd) {
    case 'F': driveMotors( FAST_US,  FAST_US); break;
    case 'B': driveMotors(-FAST_US, -FAST_US); break;
    case 'L': driveMotors(-SLOW_US,  SLOW_US); break;
    case 'R': driveMotors( SLOW_US, -SLOW_US); break;
    case 'A': driveMotors( SLOW_US,  SLOW_US); break;
    case 'S':
    default:  stopMotors(); break;
  }
}
