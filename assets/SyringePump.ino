#include <digitalWriteFast.h>

#include <AccelStepper.h>
#include <MultiStepper.h>

float flowRateMax = 30; // max flow rate in ml/min
int ms = 16; //up to 25.9 ml/min with 8ms and 20ml syringe or 13 ml/min with 16ms -- 10.3 ml/min with 8ms and 5ml syringe or 5.2 ml/min with 16ms
const float syringe_capacity = 20; // 5 or 20 

const int step_pin = 12; // driver step pin
const int dir_pin = 13; // driver direction pin
const int ms1_pin = 4; // microstepping pin 1
const int ms2_pin = 5; // microstepping pin 2
const int ms3_pin = 6; // microstepping pin 3
const int pot = A0;
const int reverse = 7;
const int forward = 8;
int ms_pins[] = {ms1_pin, ms2_pin, ms3_pin};

const int ledR = 9;
const int ledG = 10;
const int ledB = 11;
const int switch_pin = 2; // latching switch pin, must be pin 2 or 3
const int endstop = 3;

float flowRPS;
float syringeArea;
float volumePerRev;
float flowVolumeMax;
float flowRPSMax;



int currentPot;
int prevPot;
bool empty;
bool paused;
bool pumping;
float syringe_id; // syringe ID in cm
float syringeVolumePerStep;
float stepsPerSec;
const float lead = 0.2; //leadscrew distance per revolution in cm

float motorSpeed;

int ms_pinval[5][3] = {
  {1,1,1}, //16
  {1,1,0}, //8
  {0,1,0}, //4
  {1,0,0}, //2
  {0,0,0} //1
};

AccelStepper stepper(AccelStepper::DRIVER, step_pin, dir_pin);

void setup() {
//  Serial.begin(9600);

  stepper.setMaxSpeed(1200); // max speed in steps/sec

  pinMode(switch_pin, INPUT_PULLUP);
  
  pinMode(endstop, INPUT_PULLUP);
  pinMode(reverse, INPUT_PULLUP);
  pinMode(forward, INPUT_PULLUP);
  

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(step_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(ms1_pin, OUTPUT);
  pinMode(ms2_pin, OUTPUT);
  pinMode(ms3_pin, OUTPUT);

  if (syringe_capacity == 5) {
    syringe_id = 1.206; // 5mL syringe ID in cm
  }

  if (syringe_capacity == 20) {
    syringe_id = 1.913; // 20mL syringe ID in cm
  }

  syringeArea = sq(syringe_id/2)*PI;
  volumePerRev = syringeArea*lead; //mL per full revolution of leadscrew
  flowVolumeMax = (stepper.maxSpeed()/(200*ms))*volumePerRev; //in ml/sec
  flowRPSMax = flowVolumeMax/volumePerRev; //removes a float calculation from main function loop

  if (ms == 16) {
    for (int i = 0; i < 3; i++) {
     digitalWrite(ms_pins[i], ms_pinval[0][i]);
    }
  }
  if (ms == 8) {
    for (int i = 0; i < 3; i++) {
     digitalWrite(ms_pins[i], ms_pinval[1][i]);
    }
  }
  attachInterrupt(digitalPinToInterrupt(endstop), done, HIGH); // endstop wired normally closed

  empty = false;
  paused = true;

  digitalWrite(ledR,255);
  digitalWrite(ledG,5);
  delay(2000);
}

void done() {
  stepper.setSpeed(0);
  stepper.stop();
  digitalWrite(ledR, 255);
  digitalWrite(ledG, 0);
  digitalWrite(ledB, 0);
  empty = true;
}

void loop() {
  if (digitalRead(switch_pin)==1) {
    paused = true;
  }
  else {
    paused = false;
  }  
  if (digitalRead(reverse) == 1 || digitalRead(forward) == 1) {
    if (!empty && !paused) {
      currentPot = analogRead(pot);
      if (currentPot > prevPot + 3 || currentPot < prevPot - 3) {
        prevPot = currentPot;
        flowRPS = (currentPot/1023.0) * flowRPSMax;
//        Serial.println((flowRPS*volumePerRev)*60.0);
      }
    stepper.setSpeed(flowRPS*200.0*ms);
    stepper.runSpeed();
    digitalWriteFast(ledG, 255);
    digitalWriteFast(ledR, 0);
    }
    if (paused == true && empty == false) {
      stepper.setSpeed(0);
      stepper.stop();
      digitalWriteFast(ledG, 5);
      digitalWriteFast(ledR, 255);
    }
    if (empty == true){
      stepper.setSpeed(0);
      stepper.stop();
      digitalWrite(ledR, 255);
      digitalWrite(ledG, 0);
      digitalWrite(ledB, 0);
    }
  }
  if (digitalRead(reverse)==0) {
    stepper.setSpeed(-1000);
    stepper.runSpeed();
  }
  if (digitalRead(forward)==0) {
    stepper.setSpeed(1000);
    stepper.runSpeed();   
     
  }
}
