#include <LiquidCrystal.h>

#define SENSOR 2
#define LED 13
#define KILLTIME 2000

LiquidCrystal lcd(6,7,8,9,10,11);

int startTime;  
int  currentTime;   
int  maxTime;   
int  finalTime;
bool time_started;

void setup() {
  lcd.begin(16,2);

  lcd.setCursor(0,1);
  lcd.print("c_time 0");

  lcd.setCursor(0,0);
  lcd.print("m_time 0");

  pinMode(SENSOR, INPUT);
  pinMode(LED, OUTPUT);

}

void loop() {
    
  int detected = digitalRead(SENSOR);

  // laser detected
  if(detected == LOW) 
  {
    if (time_started == false)
    {
      time_started = true;
      startTime = millis();
    }

    currentTime = millis() - startTime;
    lcd.setCursor(7,1);
    lcd.print("        ");
    lcd.setCursor(7,1);
    lcd.print(currentTime);
   
  }
  
  // laser not detected
  if(detected == HIGH)
  {
    if (time_started == true)
    {
      finalTime = millis();
      currentTime = finalTime - startTime;
      
      if (currentTime > maxTime)
      {
        maxTime = currentTime;
      }
      
      lcd.setCursor(7,0);
      lcd.print(maxTime);
      currentTime = 0;
    }
    time_started = false;
  }

  // Kill Time reached, turn on LED. 
  if ((currentTime >= KILLTIME) || (maxTime >= KILLTIME))
  {
      digitalWrite(LED, HIGH);
  }
}


