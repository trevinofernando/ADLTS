#ifndef MOTORS_H
#define MOTORS_H

#include "motor.h"

#define StepsPerRevolution 1600
#define connectionMotor1 0
#define connectionMotor2 2
#define connectionMotor3 3
#define connectionMotor4 4


class StepperMotors {
  public:
    // constructors:
    StepperMotors();
    ~StepperMotors();

    // mover method:
    void step(float angleX, float angleY);

  private:
    Stepper myStepper1;
    Stepper myStepper2;
};

#endif
