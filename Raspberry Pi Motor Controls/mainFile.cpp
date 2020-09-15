#include <iostream>
#include "motor.h"
/*
Done in motor.h in case it produces error we can remove these comments

#define totalStepsPerRevolution 1600
#define connectionMotor1 0
#define connectionMotor2 2
#define connectionMotor3 3
#define connectionMotor4 4
*/

using namespace std;


int main()
{
    
    // required only once
    wiringPiSetup();
    Stepper myStepper(totalStepsPerRevolution, connectionMotor1, connectionMotor2);
    Stepper myStepper2(totalStepsPerRevolution, connectionMotor3, connectionMotor4);
    
    // requires as many times you want motor to move
    float angle;
    // moves x motor
    myStepper.stepAngle(angle, totalStepsPerRevolution);
    // moves y motor
    myStepper2.stepAngle(angle, totalStepsPerRevolution);
    
    return 0;
}

