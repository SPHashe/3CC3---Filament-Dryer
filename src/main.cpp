/////////////////////////////////////////////////////////////////////////
//////                            3CC3 Project                      /////
////// By: Parsa Hashemi (400516453), and Vic Cuatico (400507885)   /////
//////                        Smart Filament dryer                  /////
/////////////////////////////////////////////////////////////////////////

/*To Do
1. Debug
2. Connect to cloud (only the main code left)
3. Program the stepper motor movement
4. Get the heater logic
*/

//External Libraries
#include <Arduino.h>
#include <DHT.h>
#include <IoT.h>

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

  float PreviousSetpoint = 0;

//Cloud Variables
  //Temperature and Humidity
  float Humidity = 0;
  float Temperature = 0;

  //Temperature Setpoint
  float TempSetpoint = -10000;
  float HumidityLimit = -10000;

  float TempTolerance = -10000; // in c and +/-
  bool TempSetpointReached = false;

  //If spinning or not
  bool ComandedArmed = false; //What server said
  bool ActualArmed = false; //Acctual state


//Objects
Alert Alarm(AlarmPin, &TempSetpointReached, Timeout, BlinkingPeriod, TempTolerance);
Screen Display(RS, EN, D4, D5, D6, D7);
DHT TandH(HumidAndTemp, DHT11);
IoT Cloud(&TempSetpoint, &TempTolerance, &HumidityLimit, &ComandedArmed, &ActualArmed);

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

  //Cloud Init
  Cloud.Begin();

  //while (TempSetpoint == -10000 || HumidityLimit == -10000 || TempTolerance == -10000)
  //{
  //}
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

  
  Cloud.Loop(Temperature, Humidity, TempSetpointReached, ActualArmed);

  delay(100);
}

