#include "motors.h"

/*
StepperMotors::StepperMotors()
{
  wiringPiSetup();
  this->myStepper1 = new Stepper(StepsPerRevolution, connectionMotor1, connectionMotor2);
  this->myStepper2 = new Stepper(StepsPerRevolution, connectionMotor3, connectionMotor4);
}
*/
StepperMotors::StepperMotors(): myStepper1(StepsPerRevolution, connectionMotor1, connectionMotor2), myStepper2(StepsPerRevolution, connectionMotor3, connectionMotor4)
{
}

void StepperMotors::step(float angleX, float angleY)
{
  this->myStepper1.stepAngle(angleX, StepsPerRevolution);
  this->myStepper2.stepAngle(angleY, StepsPerRevolution);
}


StepperMotors::~StepperMotors()
{
}
