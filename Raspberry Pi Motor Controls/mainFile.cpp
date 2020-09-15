#include <iostream>
#include "motor.h"
/*
#define totalStepsPerRevolution 1600
#define connectionMotor1 0
#define connectionMotor2 2
#define connectionMotor3 3
#define connectionMotor4 4
*/

using namespace std;


int main()
{
    float angle;
    
    wiringPiSetup();
    Stepper myStepper(totalStepsPerRevolution, connectionMotor1, connectionMotor2);
    myStepper.stepAngle(angle, stepsPerRevolution);
    
    return 0;
}

