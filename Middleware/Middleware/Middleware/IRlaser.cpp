#define LED 13

int incomingByte = 0;
int IRfrequency = 38;
int IRt = 9;
int IRpulse = 600;
int IRpulses = 23;

void Setup() 
{
	// put your setup code here, to run once:
	Serial.begin(9600);
	/*
	IRt = (int) (500/IRfrequency);

	IRpulses = (int) (IRpulse / (2*IRt));

	IRt = IRt - 4;

	Serial.print("IRt = ");
	Serial.println(IRt, DEC);

	Serial.print("IRpulse = ");
	Serial.println(IRpulse, DEC);

	Serial.print("IRpulses = ");
	Serial.println(IRpulses, DEC);
	*/
	pinMode(LED, OUTPUT);
}


void SendPulse(int pin, int length) 
{

	int i = 0;
	int o = 0;

	while (i < length) {
		i++;
		while (o < IRpulses) {
			o++;
			digitalWrite(pin, HIGH);
			delayMicroseconds(IRt);
			digitalWrite(pin, LOW);
			delayMicroseconds(IRt);
		}
	}
}

void Shoot(int color)
{
	// color = 0 = red
	// color = 1 = blu
	sendPulse(LED, 4);

	delayMicroseconds(IRpulse);

	for (int i = 0; i < 8; i++)
	{
		if (color == 0)
		{
			sendPulse(LED, 1);
		}

		sendPulse(LED, 1);
		delayMicroseconds(IRpulse);

	}

	for (int i = 0; i < 8; i++) {

		if (color == 0)
		{
			sendPulse(LED, 1);
		}

		sendPulse(LED, 1);
		delayMicroseconds(IRpulse);

	}

	sendPulse(LED, 1);

	delayMicroseconds(IRpulse);

}

void Loop() 
{
	// put your main code here, to run repeatedly:
	if (Serial.available() > 0) {
		// read the incoming byte:
		incomingByte = Serial.read();

		if (incomingByte == 114)
		{
			Serial.println("RED");
			shoot(0);
		}
		else if (incomingByte == 98)
		{
			Serial.println("BLUE");
			shoot(1);
		}
	}

	//shoot(1);
	//delay(5000);
}