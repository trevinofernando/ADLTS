#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

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

    //std::cout << "Sending pulse" << std::endl;
    while (i < length)
    {
        i++;
        while (o < IRpulses)
        {
            o++;
            digitalWrite(pin, HIGH);
            delayMicroseconds(IRt);
            digitalWrite(pin, LOW);
            delayMicroseconds(IRt);
        }
    }
}

void IRLaser::Shoot(int color)
{
    // color = 0 = red
    // color = 1 = blue
    SendPulse(4);
    //std::cout << "Shooting" << std::endl;
    delayMicroseconds(IRpulse);

    for (int i = 0; i < 8; i++)
    {
        if (color == 0)
        {
            SendPulse(1);
        }

        SendPulse(1);
        delayMicroseconds(IRpulse);
    }

    for (int i = 0; i < 8; i++)
    {
        if (color == 0)
        {
            SendPulse(1);
        }

        SendPulse(1);
        delayMicroseconds(IRpulse);
    }

    SendPulse(1);
    delayMicroseconds(IRpulse);
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

