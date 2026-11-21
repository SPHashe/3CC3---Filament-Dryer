//External Libraries
#include <Arduino.h>
#include <DHT.h>


//Internal Libraries
#include <Alert.h>
#include <Screen.h>

//Pin Assignments
  //LCD
  const int RS = 19;
  const int EN = 23;
  const int D4 = 18;
  const int D5 = 17;
  const int D6 = 16;
  const int D7 = 15;

  //DHT11 temp and humidity
  const int HumidAndTemp = 4;

  //Buzzer and LED Alert
  const int AlarmPin = 21; 

  //Heating Unit
  //const int Heater = x;

//GlobalVariables
  const int Timeout = 100000;
  const int BlinkingPeriod = 1000;

//Cloud Variables
  //Temperature and Humidity
  float Humidity = 0;
  float Temperature = 0;

  //Temperature Setpoint
  float TempSetpoint = 28.00;
  float HumidityLimit = 40.00;

  float TempTolerance = 3; // in c and +/-
  bool TempSetpointReached = false;

  //If spinning or not
  bool ComandedArmed = false; //What server said
  bool ActualArmed = false; //Acctual state


//Objects
Alert Alarm(AlarmPin, &TempSetpointReached, Timeout, BlinkingPeriod, TempTolerance);
Screen Display(RS, EN, D4, D5, D6, D7);
DHT TandH(HumidAndTemp, DHT11);




//Functions

void setup() 
{
  Serial.begin(9600);

  //Alarm 
  Alarm.Begin();

  //Humidity and Temperature
  TandH.begin();

  //LCD
  Display.Begin();
}

void loop() 
{
  //Humidity and Temperature
  Humidity = TandH.readHumidity();
  Temperature = TandH.readTemperature();

  //LCD
  Display.Display("Temp: " + String(Temperature) + "C", 0);
  Display.Display("Humidity: " + String(Humidity) + "%", 1);

  //Serial Alerts
  if (Alarm.OverHumidityAlert(Humidity, HumidityLimit))
  {
    Serial.println("HUMIDITY ALERT");
  }

  if (Alarm.OverTempAlert(Temperature, TempSetpoint))
  {
    Serial.println("TEMP ALERT");
  }

  if (!TempSetpointReached)
  {
    if (Temperature == TempSetpoint)
    {
      TempSetpointReached = true;
      Alarm.SetReachedSetpoint(TempSetpointReached);
    }
    else 
    {
      Alarm.TimeoutCheck();
    }
  }
  else
  {

  }

  Serial.println(TempSetpointReached);
  
  delay(100);
}

