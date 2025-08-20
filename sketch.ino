#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>

// Motor
const int m1_1 = 2;
const int m1_2 = 3;
// motor speed control
const int ena = 9;

// Front-facing sensor
const int trigPin1 = 9;
const int echoPin1 = 10;
// Left sensor
const int trigPin2 = 11;
const int echoPin2 = 12;
// Right sensor
const int trigPin3 = 13;
const int echoPin3 = 14;
// Front-left sensor
const int trigPin4 = 15;
const int echoPin4 = 16;
// Front-right sensor
const int trigPin5 = 17;
const int echoPin5 = 18;
// Back-facing sensor
// const int trigPin4 = 15;
// const int echoPin4 = 16;

// IR sensor
const int IR1 = 5;

// constant distance values
const int openDistForTurn = 1;
const int closedDistForTurn = 1;
const int carLength = 0;
const int safeDistForTurn = 0;
const int turnDegServo = 90;
const int turnThreshold = 5;

// Servo
Servo steer;
int servo_pwm = 3;
int servo_pos = 0;

// Compass
MPU6050 mpu;
float yaw = 0;
unsigned long lastTime;
float gyroZoffset = 0;

float distFront, distLeft, distRight, distFrontLeft, distFrontRight, initDistFront, initDistBack, distBack;


void calibrateGyro() {
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  long sum = 0;
  for (int i = 0; i < 1000; i++) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    sum += gz;
    delay(2);
  }
  gyroZoffset = sum / 1000.0;
  Serial.print("Gyro Z offset = "); Serial.println(gyroZoffset);

  lastTime = millis();
}

void setup() {
  pinMode(m1_1, OUTPUT);
  pinMode(m1_2, OUTPUT);
  pinMode(m2_1, OUTPUT);
  pinMode(m2_2, OUTPUT);
  pinMode(ena,  OUTPUT);
  pinMode(enb, OUTPUT);

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  // pinMode(trigPin4, OUTPUT);
  // pinMode(echoPin4, INPUT);
  pinMode(IR1, INPUT);

  steer.attach(servo_pwm);
  steer.write(90);
  
  Serial.begin(9600);

  delay(100); 
  calibrateGyro();
  // initDistBack = measureDist(trigPin4, echoPin4);
}

float measureDist(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  
	delayMicroseconds(2);  
	digitalWrite(trigPin, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(trigPin, LOW);  

  float duration = pulseIn(echoPin, HIGH);  
  return (duration*.0343)/2;
}

bool lineFlag = false;
bool isOnLine() {
  int sensorValue = digitalRead(IR1);
  if (sensorValue == LOW) {
    lineFlag = true;
    return true;
  } else {
    lineFlag = false;
    return false;
  }
}

void updateYaw() {
  int gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  float gyroZ = (gz - gyroZoffset) / 131.0; // dps
  yaw += gyroZ * dt;
}

void turnLeft() {
  for (int pos = 90; pos <= 90 + turnDegServo; pos++) {
    steer.write(pos); 
    delay(15);
  }
  delay(500);
  for (int pos = turnDegServo; pos >= 90; pos--) {
    steer.write(pos); 
    delay(15);
  }
}

void turnRight() {
  for (int pos = 90; pos >= 90 - turnDegServo; pos--) {
    steer.write(pos); 
    delay(15);
  }
  delay(500);
  for (int pos = 90 - turnDegServo; pos <= 90; pos++) {
    steer.write(pos); 
    delay(15);
  }
}

void turnSlightLeft() {

}

void turnSlightRight() {

}

void stopVehicle() {
  analogWrite(ena, 0);
  analogWrite(enb, 0);
  digitalWrite(m1_1, LOW);
  digitalWrite(m1_2, LOW);
  digitalWrite(m2_1, LOW);
  digitalWrite(m2_2, LOW);
}

void moveForward() {
  // move vehicle forward with variable speed depending on different situations
  analogWrite(ena, 200);

  digitalWrite(m1_1,  HIGH);
  digitalWrite(m1_2, LOW);
  steer.write(90);
}

int turns = 0;
void loop() {
  updateYaw();
  distFront = measureDist(trigPin1, echoPin1);
  distFrontLeft = measureDist(trigPin4, echoPin4);
  distFrontRight = measureDist(trigPin5, echoPin5);
  distLeft = measureDist(trigPin2, echoPin2);
  distRight = measureDist(trigPin3, echoPin3);
  // distBack = measureDist(trigPin4, echoPin4);

  // if (turns == 12) {
  //   if (distBack >= initDistBack) {
  //     stopVehicle()
  //     return;
  //   }
  // }

  // if (distLeft >= openDistForTurn && distRight <= closedDistForTurn) {
  //   passedDist = initDistFront - distFront;
  //   if (passedDist == 0) turnDistFront = distFront;
  //   if (passedDist >= carLength + safeDistForTurn) {
  //     turnLeft();
  //     // turns++;
  //     passedDist = 0;
  //     return;
  //   }
  // }

  if (distFront < safeDistForTurn) {
    Serial.println("Reached end of track");
    stopVehicle();
  }
  // TODO: handle two lines per turn
  if (!lineFlag && isOnLine()) {
    Serial.println("Crossing a line");
    turns++;
  }

  if (distFrontLeft > distFrontRight + turnThreshold) {
    Serial.println("Slight rotation to Left");
    turnSlightLeft(); 
    return;
  } else if (distFrontRight > distFrontLeft + turnThreshold) {
    Serial.println("Slight rotation to Right");
    turnSlightRight();
    return;
  } 
  else if (distLeft > distRight + turnThreshold) {
    Serial.println("Sharp rotation to Left");
    turnLeft();
    return;
  } else if (distRight > distLeft + turnThreshold) {
    Serial.println("Sharp rotation to Right");
    turnRight();
    return;
  }

  moveForward();

}
