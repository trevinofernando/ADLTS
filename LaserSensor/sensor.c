
#include <LiquidCrystal.h>
#include <IRremote.h>

#define SENSOR 2
#define RED 13
#define GREEN 12
#define BLUE 11

LiquidCrystal lcd(4,5,6,7,8,9);

  int  BlueHP = 5;   
  int  RedHP = 5;
  int  NearMisses = 0;

  IRrecv irrecv(SENSOR);
  decode_results results;

  int incomingByte = 0;

void setup() {

  Serial.begin(9600);
  irrecv.enableIRIn();
  
  // put your setup code here, to run once:
  lcd.begin(16,2);

  lcd.setCursor(0,0);
  lcd.print("HP BLU:20 RED:20");

  lcd.setCursor(0,1);
  lcd.print("NEARMISS: 0");

  pinMode(SENSOR, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

}

void blinkLED(int red, int green, int blue)
{
  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);

  delay(200);
}

void receiveIR()
{
  if (irrecv.decode(&results)) 
  {
    Serial.println(results.value);
    irrecv.resume(); // Receive the next value

    // Red irShoot(0) || remote(1)
    if ((results.value == 462217468) || (results.value == 16724175))
    {
      RedHP--;
      lcd.setCursor(14,0);
      lcd.print("  ");
      lcd.setCursor(14,0);
      lcd.print(RedHP);

      blinkLED(255, 0, 0);
      blinkLED(0, 0, 0);
    }
    
    // Blue irShoot(1) || remote(2)
    else if ((results.value == 3389625022) || (results.value == 16718055))
    {
      BlueHP--;
      lcd.setCursor(7,0);
      lcd.print("  ");
      lcd.setCursor(7,0);
      lcd.print(BlueHP);

      blinkLED(0, 0, 255);
      blinkLED(0, 0, 0);
    }
    
    // Near misses, but saw a signal...
    else 
    {
      if(results.value != 4294967295)
      {
     
        NearMisses++;
        lcd.setCursor(10,1);
        lcd.print(NearMisses);
  
        blinkLED(0, 255, 0);
        blinkLED(0, 0, 0);
      }
    }
  }
}

void loop() 
{
  if (BlueHP <= 0)
  {
    blinkLED(0, 0, 255);
  }
  else if (RedHP <= 0)
  {
    blinkLED(255, 0, 0);
  }
  else 
  {
    receiveIR();
  }

//  blinkLED(255, 0, 0);
//  blinkLED(0, 255, 0);

//  digitalWrite(RED, HIGH);
//  digitalWrite(GREEN, HIGH);
//  digitalWrite(BLUE, HIGH);

  //shoot(0);
  //delay(250);
}
