#include "motors.h"

#define StepsPerRevolution 1600
#define connectionMotor1 0
#define connectionMotor2 2
#define connectionMotor3 3
#define connectionMotor4 4


class StepperMotors {
  public:
    // constructors:
    StepperMotors(int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4);

    // mover method:
    void step(float angleX, float angleY);

  private:
    Stepper myStepper1;
    Stepper myStepper2;
};
StepperMotors::StepperMotors()
{
}

void StepperMotors::step(float angleX, float angleY)
{
}
