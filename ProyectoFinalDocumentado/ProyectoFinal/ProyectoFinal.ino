/*!
\file   ProyectoFinal.ino
\date   2023-06-28
\author Katherin Alexandra Zuñiga Morales <kzunigam@unicauca.edu.co>
\author David Santiago Fernandez Dejoy <dfernandezd@unicauca.edu.co>
\author Jose David Chilito Cometa <jdchilito@unicauca.edu.co>
\brief  Proyecto Final.

\par Copyright
Information contained herein is proprietary to and constitutes valuable
confidential trade secrets of Unicauca, and
is subject to restrictions on use and disclosure.

\par
Copyright (c) Unicauca 2023. All rights reserved.

\par
The copyright notices above do not evidence any actual or
intended publication of this material.
******************************************************************************
*/

/********************************************//**
 *  Definiciones de pines
 ***********************************************/
#include "pins.h"

/********************************************//**
 *  Biblioteca para administrar tareas asíncronas
 ***********************************************/
#include "StateMachineLib.h"


/********************************************//**
 *  Funciones de control del sensor de temperatura
 *  y humedad
 ***********************************************/
#include "DHTStable.h"
DHTStable DHT;

/********************************************//**
 *  Biblioteca para administrar tareas asíncronas
 ***********************************************/
#include "AsyncTaskLib.h"

/********************************************//**
 *  Biblioteca para controlar una pantalla LCD
 ***********************************************/
#include <LiquidCrystal.h>
LiquidCrystal lcd(LCD1,LCD2,LCD3,LCD4,LCD5,LCD6);


/********************************************//**
 *  Biblioteca para manejar un teclado numérico
 ***********************************************/
#include <Keypad.h>
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);


/********************************************//**
 *  Definición de patrones de caracteres personalizados
 ***********************************************/
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte sadFace[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b01110,
  0b10001,
  0b10001
};

byte candado[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b00000
};

/********************************************//**
 *   Estados de la máquina de estados
 ***********************************************/
enum State
{
	Inicio = 0,
	MonitTHL = 1,
	Alarma = 2,
	SysBloq = 3,
	MonPV = 4
};

/********************************************//**
 *   Entradas de la máquina de estados
 ***********************************************/
enum Input
{
	contraCorrecta = 0,
	sistemaBloqueado = 1,
  tempover = 2,
  timeout = 3,
  tempoverandtimeout = 4,
  inAsterisco = 5,
  event = 6 ,
  Unknown = 7
};

/********************************************//**
 *   Constantes y variables globales
 ***********************************************/
int tempInicio = 0;
int tempFinal = 0;
int outputValue = 0;
char clave[7] = "123456";
char ing_contrasenia[7];
int longitudCadena = 0;
int numIntentos = 0;
const int MAXIMO = 3;
bool isLogin=false;
int contadorAlarma=0;
StateMachine stateMachine(5,11);
Input input;
volatile int eventDetected = LOW;

/*F**************************************************************************
* NAME: time20sec
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define time20sec funcion para redirigir a SysBloq
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void time20sec()
{
  
  input = Input::tempoverandtimeout;
  if(stateMachine.GetState()==SysBloq){
    input = Input::timeout;
  }
}

/*F**************************************************************************
* NAME: timeoutgeneral
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define timeoutgeneral funcion para cambios de estado 
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void timeoutgeneral()
{
  input = Input::timeout;
}

/*F**************************************************************************
* NAME: temporizador
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define temporizador procedimiento que resetea en caso de que la contrasena
* este incompleta
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void temporizador(){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clave Incorrecta");
      lcd.setCursor(8, 1);
      lcd.write((byte)2);
      delay(2000);

      inputSecuritySystem();      
      tempInicio=0;
      tempFinal=0;
      numIntentos++;
}

/********************************************//**
 *   Creación de las tareas asíncronas
 ***********************************************/
AsyncTask ATTemporizador(5000, false, temporizador);
AsyncTask asyncTaskTemp(1000, true, readtemperatureandhumedity);
AsyncTask ATtime5sec(5000, true,timeoutgeneral);
AsyncTask ATtime20sec(10000, false,time20sec);
AsyncTask ATtime2coma5sec(2500,false,timeoutgeneral);
AsyncTask ATtime1coma5sec(1500,false,timeoutgeneral);

/*F**************************************************************************
* NAME: readtemperatureandhumedity
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define readtemperatureandhumedity procedimiento para leer temperatura y humedad
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void readtemperatureandhumedity(){
  // Leer datos de temperatura y humedad del sensor DHT
  int chk = DHT.read11(DHTPIN);
  float valorTem=DHT.getTemperature();
  float valorHum=DHT.getHumidity();

  // Leer valor de la fotoresistor
  outputValue = analogRead(photocellPin);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.print(valorHum);
  lcd.setCursor(8,0);  
  lcd.print("Luz:");
  lcd.print(outputValue);
  lcd.setCursor(0,1);
  lcd.print("Temp:");
  lcd.print(valorTem);
  
  // Verificar si la temperatura supera el umbral establecido (30.0 grados Celsius)
  if(valorTem>30.0f){
      input = Input::tempover;
  }else{
    ATtime20sec.Stop();
    noTone(pinBuzzer);
    contadorAlarma=0;
  }

}

/*F**************************************************************************
* NAME: inputSecuritySystem
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define inputSecuritySystem procedimiento para mostrar mensaje del ingreso de clave
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void inputSecuritySystem(){
  eventDetected=LOW;
  isLogin=false;
  longitudCadena = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ingrese Clave:");
  lcd.setCursor(0,1);
  color(0,0,0);
}


/*F**************************************************************************
* NAME: inputAlarma
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define inputAlarma procedimiento para manejar los tiempos de la alarma
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void inputAlarma(){
  ATtime5sec.Start();
  if(contadorAlarma==0){
    ATtime20sec.Start();

  }
  contadorAlarma++;  
  
  tone(pinBuzzer,800);
  color(255,0, 0);
}

/*F**************************************************************************
* NAME: inputDHT
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define inputDHT procedimiento que empieza la tarea para mostrar temperatura, humedad y luz.
* Tambien empieza la tarea de tiempo para el cambio de estado
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void inputDHT(){
  Serial.println("Entra a DHT");
  asyncTaskTemp.Start();
  ATtime2coma5sec.Start();
}

/*F**************************************************************************
* NAME: inputSysBloq
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define inputSysBloq procedimiento para las tareas e indica que el sistema esta bloqueado
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void inputSysBloq(){
  color(255, 0,0);
  asyncTaskTemp.Stop();
  ATtime5sec.Stop();
  ATtime1coma5sec.Stop();
  ATtime2coma5sec.Stop();
  contadorAlarma=0;
  noTone(pinBuzzer);
  eventDetected=LOW;  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SistemaBloqueado");
  lcd.setCursor(0, 1);
  lcd.write((byte)3);
  ATtime20sec.Stop();
  ATtime20sec.Start();
}

/*F**************************************************************************
* NAME: inputMonPV
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define inputMonPV procedimiento para detectar movimiento en el estado monPV
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void inputMonPV(){
    Serial.println("Entra a PV");
    lcd.clear();
    lcd.setCursor(0,0);
    if(eventDetected==HIGH){  
      asyncTaskTemp.Stop();
      ATtime5sec.Stop();
      ATtime20sec.Stop();
      ATtime1coma5sec.Stop();
      ATtime2coma5sec.Stop();
      lcd.print("Movimiento");
      lcd.setCursor(0,1);      
      lcd.print("Detectado");
      delay(1000);      
          
    }else{
      ATtime1coma5sec.Start();
      lcd.print("Monitoreando");
    }
    
}

/*F**************************************************************************
* NAME: activarEvento
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define activarEvento procedimiento para activar el evento
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void activarEvento(){
  eventDetected=HIGH;
}


/*F**************************************************************************
* NAME: setupStateMachine
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define setupStateMachine procedimiento que se encarga de configurar la máquina de estados 
* con las transiciones y acciones correspondientes
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void setupStateMachine()
{
	// Agregar transiciones
	stateMachine.AddTransition(Inicio, MonitTHL, []() { return input == contraCorrecta; });
  stateMachine.AddTransition(Inicio,SysBloq , []() { return input == sistemaBloqueado; });


	stateMachine.AddTransition(MonitTHL, Alarma, []() { return input == tempover; });
	stateMachine.AddTransition(MonitTHL, MonPV, []() { return input ==timeout ; });
  stateMachine.AddTransition(MonitTHL, SysBloq, []() { return input == tempoverandtimeout; });

	stateMachine.AddTransition(Alarma, SysBloq, []() { return input == tempoverandtimeout; });
	stateMachine.AddTransition(Alarma, MonitTHL, []() { return input == timeout; });

	stateMachine.AddTransition(SysBloq,MonPV , []() { return input == timeout; });

	stateMachine.AddTransition(MonPV,Inicio , []() { return input ==inAsterisco ; });
  stateMachine.AddTransition(MonPV,MonitTHL , []() { return input == timeout; });
  stateMachine.AddTransition(MonPV,SysBloq , []() { return input ==event ; });

	// Agregar acciones
	stateMachine.SetOnEntering(Inicio, inputSecuritySystem);
	stateMachine.SetOnEntering(MonitTHL,inputDHT);
	stateMachine.SetOnEntering(Alarma, inputAlarma);
	stateMachine.SetOnEntering(SysBloq, inputSysBloq);
  stateMachine.SetOnEntering(MonPV, inputMonPV);

	stateMachine.SetOnLeaving(Inicio,[]() {Serial.println("Leaving Inicio"); });
	stateMachine.SetOnLeaving(MonitTHL, []() {Serial.println("Leaving MonitTHL"); });
	stateMachine.SetOnLeaving(Alarma, []() {Serial.println("Leaving Alarma"); });
  stateMachine.SetOnLeaving(MonPV, []() {Serial.println("Leaving MonPV"); });
	stateMachine.SetOnLeaving(SysBloq, []() {Serial.println("Leaving Sysbloq"); });
}


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.createChar(1, heart);
  lcd.createChar(2, sadFace);
  lcd.createChar(3, candado);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT); 
  pinMode(bluePin, OUTPUT); 
  pinMode(pinBuzzer, OUTPUT);

  
  pinMode(intPinHall, INPUT_PULLUP);
  pinMode(intPinTracking,INPUT_PULLUP);
  pinMode(intPinMetal, INPUT_PULLUP);
  
  // Configurar interrupciones
  attachInterrupt(digitalPinToInterrupt(intPinMetal), activarEvento, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPinTracking), activarEvento, FALLING);
  attachInterrupt(digitalPinToInterrupt(intPinHall), activarEvento, FALLING);
  eventDetected=LOW;

  lcd.setCursor(0,0);
  lcd.print("Bienvenido");
  lcd.setCursor(0,1);
  lcd.write((byte)1);
  delay(1000);

  // Configurar la máquina de estados
  setupStateMachine();
  stateMachine.SetState(Inicio, false, true);

  lcd.setCursor(0,1);
}

/*F**************************************************************************
* NAME: updateTasks
*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define updateTasks procedimiento que se encarga de actualizar todas las tareas
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
void updateTasks(){
  asyncTaskTemp.Update();
  ATtime5sec.Update();
  ATtime20sec.Update();
  ATtime2coma5sec.Update();
  ATtime1coma5sec.Update();
  ATTemporizador.Update();
}

void loop() {
  input = static_cast<Input>(readInput());
  updateTasks();
  stateMachine.Update();
}

/*F**************************************************************************
* NAME: readInput

*----------------------------------------------------------------------------
* PARAMS:
* return:   none
*----------------------------------------------------------------------------
* PURPOSE:
* Define readInput procedimiento que se encarga de leer las entradas del usuario
*----------------------------------------------------------------------------
* NOTE:
* 
*****************************************************************************/
int readInput(){
  Input currentInput=Input::Unknown;
  char key = keypad.getKey();
  if(stateMachine.GetState()==MonPV){
    if(key=='*'){
      asyncTaskTemp.Stop();
      ATtime5sec.Stop();
      ATtime20sec.Stop();
      ATtime1coma5sec.Stop();
      ATtime2coma5sec.Stop();
      numIntentos=0;
      eventDetected=LOW;
      currentInput=Input::inAsterisco;
    }
  }
  if(numIntentos>=3){
    currentInput= Input::sistemaBloqueado;
  }
  if(!isLogin){
    if (key){
        ATTemporizador.Start();
        if(key == '#'){
          ATTemporizador.Stop();          
          if (strcmp(ing_contrasenia,clave) == 0){
            color(0,255, 0); // turn the RGB LED blue
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Clave Correcta");
            lcd.setCursor(1, 2);
            lcd.print(" Bienvenido ");
            lcd.write((byte)1);
            delay(1000);
            
            currentInput= Input::contraCorrecta;
            isLogin=true;
            lcd.clear();
          }else{
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Clave Incorrecta");
            lcd.setCursor(8, 1);
            lcd.write((byte)2);
            delay(1000);
            numIntentos++;
            inputSecuritySystem();
          }
          for(int i=0;i<7;i++){
            ing_contrasenia[i]=NULL;
          }
          
        }else{
          ing_contrasenia[longitudCadena++] = key;
          lcd.print("*");
        }
    }

  }
  
  if(stateMachine.GetState()==MonPV && eventDetected==HIGH){
    currentInput=Input::event;
  }
  
  return static_cast<int>(currentInput);
}

void color (unsigned char red, unsigned char green, unsigned char blue) 
{
analogWrite(redPin, red);
analogWrite(bluePin, blue);
analogWrite(greenPin, green);
}










