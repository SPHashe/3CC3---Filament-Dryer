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
#include <AccelStepper.h>

//Internal Libraries
#include <Alert.h>
#include <Screen.h>
#include <IoT.h>

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
  const int HeaterPin = 13;

  //Armed pin
  const int ActualArmedPin = 32;

  //Roller Stepper
  const int StepsPerRevolution = 2048;
  const int StepperIn1 = 33;
  const int StepperIn2 = 25;
  const int StepperIn3 = 26;
  const int StepperIn4 = 27;

//GlobalVariables
  //Temp Setpoint
  double PreviousSetpoint = 0;

  //Errors
  bool HumidityError = false;
  bool TempError = false;
  bool TimeOutError = false;

  const int BlinkingPeriod = 1000;

//Cloud Variables
  //Temperature and Humidity
  double Humidity = 0;
  double Temperature = 0;

  //Temperature Setpoint
  double TempSetpoint = -10000;
  double HumidityLimit = -10000;

  //Timeout
  int Timeout = 100000;

  double TempTolerance = -10000; // in c and +/-
  bool TempSetpointReached = false;

  //If spinning or not
  bool ComandedArmed = false; //What server said
  bool ActualArmed = false; //Acctual state

  bool HeaterStatus = false; //Have to add this to the cloud still
  bool Error = false;
  String ErrorMessage = "";


//Objects
Alert Alarm(AlarmPin, &TempSetpointReached, Timeout, BlinkingPeriod, &TempTolerance); 
Screen Display(RS, EN, D4, D5, D6, D7);
DHT TandH(HumidAndTemp, DHT11);
IoT Cloud(&TempSetpoint, &TempTolerance, &HumidityLimit, &ComandedArmed, &Timeout);

AccelStepper StepperRoller(4, StepperIn1, StepperIn2, StepperIn3, StepperIn4);

//Functions
void DisconnectHandler();
void AlertHandler();
void SetpointHandler();
void HeaterControl();
void StepperControl();

void setup() 
{
  Serial.begin(115200 );

  //Steppr Starting
  pinMode(ActualArmedPin, INPUT);
  StepperRoller.setMaxSpeed(1000);
  StepperRoller.setSpeed(300); 

  //Heater Starting
  pinMode(HeaterPin, OUTPUT); 
  digitalWrite(HeaterPin, LOW);

  //Alarm 
  Alarm.Begin();

  //Humidity and Temperature
  TandH.begin();

  //LCD
  Display.Begin();

  Cloud.Begin();

  Serial.println("Attempting innitial connect");

  while (!Cloud.connectWiFi())
  {

    Serial.print(".");

    //Humidity and Temperature
    Humidity = TandH.readHumidity();
    Temperature = TandH.readTemperature();

    //LCD
    Display.Display("Temp: " + String(Temperature) + "C NW", 0);
    Display.Display("Humidity: " + String(Humidity) + "%", 1);

    delay(500);
  }

  Serial.println("Innitial connection successful");

  Serial.println("Attempting to sync data");

  while (TempSetpoint == -10000 || HumidityLimit == -10000 || TempTolerance == -10000)
  {
    //Humidity and Temperature
    Humidity = TandH.readHumidity();
    Temperature = TandH.readTemperature();

    Serial.println("TSP = " + String(TempSetpoint) + " HL = " + String(HumidityLimit) + " TT = " + String(TempTolerance));

    //LCD
    Display.Display("Temp: " + String(Temperature) + "C NC", 0);
    Display.Display("Humidity: " + String(Humidity) + "%", 1);
    
    Cloud.Loop(Temperature, Humidity, TempSetpointReached, ActualArmed, HeaterStatus, Error, ErrorMessage);
    delay(500);
  }

  Serial.println("Data synced! Proceeding to normal operations");
}

void loop() 
{
  bool ConnectionStatus = Cloud.Loop(Temperature, Humidity, TempSetpointReached, ActualArmed, HeaterStatus, Error, ErrorMessage);

  if (!ConnectionStatus)
  {
    DisconnectHandler();
  }

  //Serial.println("WERE IN FOLKS");
  
  //Humidity and Temperature
  Humidity = TandH.readHumidity();
  Temperature = TandH.readTemperature();

  Humidity = round(Humidity * 100)/100;
  Temperature = round(Temperature * 100)/100; //rounds to two

  //LCD
  Display.Display("Temp: " + String(Temperature) + "C       ", 0);
  Display.Display("Humidity: " + String(Humidity) + "%", 1);

  AlertHandler(); //Detects errors

  SetpointHandler(); //Detects updates, and if it reaches
      
  HeaterControl(); // Controls the heater

  StepperControl();

  //delay(1000);
}

void DisconnectHandler()
{
  Serial.println("Wifi Connection lost");

    while (!Cloud.connectWiFi())
    {
      Serial.println("Attempting reconnection");

      //Humidity and Temperature
      Humidity = TandH.readHumidity();
      Temperature = TandH.readTemperature();

      Serial.println("TSP = " + String(TempSetpoint) + " HL = " + String(HumidityLimit) + " TT = " + String(TempTolerance));

      //LCD
      Display.Display("Temp: " + String(Temperature) + "C NW", 0);
      Display.Display("Humidity: " + String(Humidity) + "%", 1);

      delay(500);
    }

    Serial.println("Wifi reconnected");
}

void AlertHandler()
{
  //Reset EVERYTHING
  String ErrorCompiler = "";
  Error = false;
  HumidityError = false;
  TempError = false;
  TimeOutError = false;

  //If changed reset it
  if (TempSetpoint != PreviousSetpoint)
  {
    PreviousSetpoint = TempSetpoint;

    TempSetpointReached = false;
    Alarm.SetReachedSetpoint(false);
  }
  
  //Serial Alerts
  if (Alarm.OverHumidityAlert(Humidity, HumidityLimit))
  {
    HumidityError = true;
    Serial.println("HUMIDITY ALERT");
    ErrorCompiler = "Over Humidity Alert!";
  }

  if (Alarm.OverTempAlert(Temperature, TempSetpoint))
  {
    TempError = true;
    Serial.println("TEMP ALERT");

    if (HumidityError)
    {
      ErrorCompiler = "Over Humidity & Temp Alert!";
    }
    else
    {
      ErrorCompiler = "Temp Alert!";
    }
  }

  if (Alarm.TimeoutCheck())//Check if this is reseting whenevr //Make a function to reset the timer later
  {
    Serial.println("TIMEOUT ALERT");
    TimeOutError = true;

    if (HumidityError) //Cause I dont think you can have a temp error and a timeout...
    {
      ErrorCompiler = "Over Humidity & Time Out Alert!";
    }
    else
    {
      ErrorCompiler = "Time Out Alert!";
    }
  }

  if (ErrorCompiler == "")
  {
    ErrorMessage = "No errors";
  }
  else
  {
    ErrorMessage = ErrorCompiler;
  }

  if (!HumidityError && !TempError && !TimeOutError)
  {
    Alarm.Silence();
  }
  else
  {
    Error = true; 
  }
}

void SetpointHandler()
{
  if (!TempSetpointReached)
  {
    if (Temperature >= TempSetpoint - 0.30 && Temperature <= TempSetpoint + 0.30)
    {
      TempSetpointReached = true;
      Alarm.SetReachedSetpoint(TempSetpointReached);
    }
  }
}

void HeaterControl()
{
  if (!(TempError || TimeOutError) && (Temperature <= TempSetpoint - 0.5))
  {
    HeaterStatus = true;
    //Serial.println("True");
  }
  else
  {
    HeaterStatus = false;
    //Serial.println("False");
  }

  digitalWrite(HeaterPin, HeaterStatus);
}

void StepperControl()
{
  ActualArmed = digitalRead(ActualArmedPin);
  Serial.println(ActualArmed);
  if (!(ActualArmed))
  {
    StepperRoller.enableOutputs();  
    Serial.println ("Moving roller");
    StepperRoller.runSpeed(); 
   }
  else
  {
    StepperRoller.disableOutputs(); 
  }
}
