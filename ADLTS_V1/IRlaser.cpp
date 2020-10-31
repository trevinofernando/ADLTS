

#include <wiringPi.h>
#include <stdlib.h>

#include "IRlaser.h"

IRLaser::IRLaser(int GPIO)
{
	pin = GPIO;	
	pinMode(pin, OUTPUT);
}


void IRLaser::SendPulse(int length) 
{

	int i = 0;
	int o = 0;

	while (i < length) {
		i++;
		while (o < IRpulses) {
			o++;
			digitalWrite(pin, HIGH);
			usleep(IRt);
			digitalWrite(pin, LOW);
			usleep(IRt);
		}
	}
}

void IRLaser::Shoot(int color)
{
	// color = 0 = red
	// color = 1 = blu
	IRLaser::sendPulse(4);

	usleep(IRpulse);

	for (int i = 0; i < 8; i++)
	{
		if (color == 0)
		{
			IRLaser::sendPulse(1);
		}

		IRLaser::sendPulse(1);
		usleep(IRpulse);

	}

	for (int i = 0; i < 8; i++) {

		if (color == 0)
		{
			IRLaser::sendPulse(1);
		}

		IRLaser::sendPulse(1);
		usleep(IRpulse);

	}

	IRLaser::sendPulse(1);

	usleep(IRpulse);

}

// void Loop() 
// {
// 	// put your main code here, to run repeatedly:
// 	if (Serial.available() > 0) {
// 		// read the incoming byte:
// 		incomingByte = Serial.read();

// 		if (incomingByte == 114)
// 		{
// 			Serial.println("RED");
// 			shoot(0);
// 		}
// 		else if (incomingByte == 98)
// 		{
// 			Serial.println("BLUE");
// 			shoot(1);
// 		}
// 	}

// 	//shoot(1);
// 	//delay(5000);
// }
