/***************************************************
Katherin alexandra zuñiga morales
David santiago fernandez dejoy
Jose david chilito

Codigo que muestra en el LCD la temperatura, humedad y luz del entorno
por medio de tareas asincronicas.
****************************************************/

#include "AsyncTaskLib.h"
#include "DHTStable.h"
#include <LiquidCrystal.h>

DHTStable DHT;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

#define DHT11_PIN 5

#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);

void readtemperatureandhumedity();
void readLight();

const int photocellPin = A0;
int outputValue = 0;
//Tarea asincronica para leer temperatura y humedad
AsyncTask asyncTaskTemp(2000, true,readtemperatureandhumedity);
//Tarea asincronica para leer la luz
AsyncTask asyncTaskLight(4000, true,readLight);

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting");
  lcd.begin(16, 2);
  asyncTaskTemp.Start();
  asyncTaskLight.Start();
}

void loop()
{

  asyncTaskTemp.Update();
  asyncTaskLight.Update();
}

//Procedimiento que lee la temperatura y la humedad
void readtemperatureandhumedity(){
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.print(DHT.getHumidity());
  lcd.setCursor(8,0);
  lcd.print("T:");
  lcd.print(DHT.getTemperature());

}

//Procedimiento que lee la luz
void readLight(){
  outputValue = analogRead(photocellPin);
  lcd.setCursor(0, 1);
  lcd.print("Photocell:");
  lcd.setCursor(11, 1);
  lcd.print(outputValue);
  Serial.println(outputValue);

}
