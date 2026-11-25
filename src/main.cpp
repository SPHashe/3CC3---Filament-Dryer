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

  double PreviousSetpoint = 0;

//Cloud Variables
  //Temperature and Humidity
  double Humidity = 0;
  double Temperature = 0;

  //Temperature Setpoint
  double TempSetpoint = -10000;
  double HumidityLimit = -10000;

  double TempTolerance = -10000; // in c and +/-
  bool TempSetpointReached = false;

  //If spinning or not
  bool ComandedArmed = false; //What server said
  bool ActualArmed = false; //Acctual state


//Objects
Alert Alarm(AlarmPin, &TempSetpointReached, Timeout, BlinkingPeriod, &TempTolerance); 
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

  while (TempSetpoint == -10000 || HumidityLimit == -10000 || TempTolerance == -10000)
  {
    //Humidity and Temperature
  Humidity = TandH.readHumidity();
  Temperature = TandH.readTemperature();

  Serial.println("TSP = " + String(TempSetpoint) + " HL = " + String(HumidityLimit) + " TT = " + String(TempTolerance));

  //LCD
  Display.Display("Temp: " + String(Temperature) + "C NC", 0);
  Display.Display("Humidity: " + String(Humidity) + "%", 1);
  
  Cloud.Loop(Temperature, Humidity, TempSetpointReached, ActualArmed);
  delay(100);
}
}

void loop() 
{
  bool HumidityError = false;
  bool TempError = false;
  bool TimeOutError = false;

  //Humidity and Temperature
  Humidity = TandH.readHumidity();
  Temperature = TandH.readTemperature();

  //Serial.println("WERE IN FOLKS");

  //LCD
  Display.Display("Temp: " + String(Temperature) + "C       ", 0);
  Display.Display("Humidity: " + String(Humidity) + "%", 1);

  //Serial Alerts
  if (Alarm.OverHumidityAlert(Humidity, HumidityLimit))
  {
    HumidityError = true;
    Serial.println("HUMIDITY ALERT");
  }
  if (Alarm.OverTempAlert(Temperature, TempSetpoint))
  {
    TempError = true;
    Serial.println("TEMP ALERT");
  }
  if (!TempSetpointReached)
  {
    if (Temperature >= TempSetpoint - 0.30 && Temperature <= TempSetpoint + 0.30)
    {
      TempSetpointReached = true;
      Alarm.SetReachedSetpoint(TempSetpointReached);
    }
    else 
    {
      if (Alarm.TimeoutCheck())//Check if this is reseting whenevr 
      {
        TimeOutError = true;
      } 
    }
  }
  else
  {

  }

  if (!HumidityError && !TempError && !TimeOutError)
  {
    Alarm.Silence();
  }

  if (TempSetpoint != PreviousSetpoint)
  {
    PreviousSetpoint = TempSetpoint;

    TempSetpointReached = false;
    Alarm.SetReachedSetpoint(false);
  }

  
  Cloud.Loop(Temperature, Humidity, TempSetpointReached, ActualArmed);

  //delay(1000);
}

