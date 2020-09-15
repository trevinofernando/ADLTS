#include "motors.h"

using namespace std;

int main()
{
	wiringPiSetup();
	StepperMotors stp();
	//step(angleX, angleY)
	stp.step(20.44, 46.4);
	return 0;
}
