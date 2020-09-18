#include <wiringPi.h>
#include "motor.h"

#include "Vectors.h"

using namespace std;

int main()
{
	wiringPiSetup();
	StepperMotors stp;
	Vector2 v(20.44, 46.4);
	//step(angleX, angleY)
	stp.step(v);
	return 0;
}
