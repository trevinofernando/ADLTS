#include <wiringPi.h>
#include <stdlib.h>
#include "motor.h"
#include "Vectors.h"
#include <iostream>

using namespace std;

Stepper::Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2)
{
  this->step_number = 0;    // which step the motor is on
  this->direction = 0;      // motor direction
  this->last_step_time = 0; // time stamp in us of the last step taken
  this->number_of_steps = number_of_steps; // total number of steps for this motor
    std::cout << "Num of steps: " << number_of_steps << std::endl;
  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);

  //setSpeed
  this->setSpeed(50);
  std::cout << "Step delay: " << this->step_delay << std::endl;
}


/*
 * Sets the speed in revs per minute
 */
void Stepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / whatSpeed;
}

void Stepper::stepAngle(float angles_to_move, int stepsPerRevolution) {
  int step_num = (int) (angles_to_move*stepsPerRevolution/360);
  std:cout << "Total steps: " << step_num << std::endl;
  step(step_num);
}

Stepper::~Stepper() {
}

/*
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 */
void Stepper::step(int steps_to_move)
{
  int steps_left = abs(steps_to_move);  // how many steps to take

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) { this->direction = 1; }
  if (steps_to_move < 0) { this->direction = 0; }


  // decrement the number of steps, moving one step each time:
  while (steps_left > 0)
  {
    //std:cout << "Steps left: " << steps_left << std::endl;

    unsigned long now = micros();
    // move only if the appropriate delay has passed:
    if (now - this->last_step_time >= this->step_delay)
    {
      // get the timeStamp of when you stepped:
      this->last_step_time = now;
      // increment or decrement the step number,
      // depending on direction:
      if (this->direction == 1)
      {
        this->step_number++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
      }
      else
      {
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        this->step_number--;
      }
      // decrement the steps left:
      steps_left--;
      // step the motor to step number 0, 1, ..., 3
      // cout<<"\nwriting "<<this->step_number % 4<<"\n";
      stepMotor(this->step_number % 4);
    }
  }
    //completeRotation = true;
}

/*
 * Moves the motor forward or backwards.
 */
void Stepper::stepMotor(int thisStep)
{
    //std::cout << "StepNum: " << thisStep << std::endl;
    switch (thisStep) {
      case 0:  // 01
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
      break;
      case 1:  // 11
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, HIGH);
      break;
      case 2:  // 10
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
      break;
      case 3:  // 00
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW );
      break;
    }
}



StepperMotors::StepperMotors(): myStepper1(StepsPerRevolution, connectionMotor1, connectionMotor2), myStepper2(StepsPerRevolution, connectionMotor3, connectionMotor4)
{
}

void StepperMotors::RotateMotors(Vector2 v)
{
  float angleX, angleY;
  angleX = v.x;
  angleY = v.y;
  this->myStepper1.stepAngle(angleX, StepsPerRevolution);
  this->myStepper2.stepAngle(angleY, StepsPerRevolution);
}


StepperMotors::~StepperMotors()
{
}
