//External Libraries
#include <Arduino.h>
#include <DHT.h>

//Internal Libraries
#include "../lib/Screen.h"

//Pin Assignments
  //LCD
  const int RS = 19;
  const int EN = 23;
  const int D4 = 18;
  const int D5 = 17;
  const int D6 = 16;
  const int D7 = 15;

  const int HumidAndTemp = 4;

//GlobalVariables
Screen Display(RS, EN, D4, D5, D6, D7);
DHT TandH(HumidAndTemp, DHT11);

//Cloud Variables
float Humidity;
float Temperature;

//Functions

void setup() 
{
  Serial.begin(9600);

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
  Display.Display("T: " + String(Temperature) + "C", 0);
  Display.Display("H: " + String(Humidity) + "%", 1);

  delay(2000);
}

