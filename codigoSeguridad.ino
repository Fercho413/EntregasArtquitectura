/***************************************************
Katherin alexandra zuñiga morales
David santiago fernandez dejoy
Jose david chilito

Codigo de seguridad con bombillo led y temporizador.
****************************************************/

#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

/* Display */
//RC-ENABLE  
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A4};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'.', '0', '=', '/'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

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
  0b00100,
  0b01010,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b00000
};

char clave[7] = "123456";
char ing_contrasenia[7];
int longitudCadena = 0;
int numIntentos = 0;
const int MAXIMO = 3;
int tempInicio = 0;
int tempFinal = 0;

void setup() {

  lcd.begin(16, 2);
  pinMode(22,OUTPUT); 
  pinMode(24,OUTPUT); 
  pinMode(26,OUTPUT); 
  lcd.createChar(1, heart);
  lcd.createChar(2, sadFace);
  lcd.createChar(3, candado);
  lcd.setCursor(0,0);
  lcd.print("Bienvenido");
  lcd.setCursor(0,1);
  lcd.write((byte)1);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ingrese Clave:");
  lcd.setCursor(0,1);
}
void loop() {
  char key = keypad.getKey();
  if (key){
    tempInicio=millis()/1000;    //Al ingresar '=' se valida si es o no la contraseña
    if(key == '='){
      if (strcmp(ing_contrasenia,clave) == 0){
        color(0,255, 0); // turn the RGB LED blue
        delay(1000);
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Clave Correcta");
        lcd.setCursor(1, 2);
        lcd.print(" Bienvenido ");
        lcd.write((byte)1);
        delay(2000);
        longitudCadena = 0;
        numIntentos = 0;
        exit(0);
      }else{
        color(255, 0, 0); // turn the RGB LED red
        delay(1000); 
        numIntentos++;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Clave Incorrecta");
        lcd.setCursor(8, 1);
        lcd.write((byte)2);
        delay(2000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Ingrese Clave:");
        lcd.setCursor(0,1);
        longitudCadena = 0;
        //Condicional para bloquear el sistema en caso de exceder los intentos(3)
        if(numIntentos >= MAXIMO){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("SistemaBloqueado");
          lcd.setCursor(0, 1);
          lcd.write((byte)3);
          longitudCadena = 0;
          numIntentos = 0;
          exit(0);
        }
      }
    }else if (longitudCadena < 6) {
      //Guarda lo escrito en 'ing_contrasenia'
      ing_contrasenia[longitudCadena++] = key;
      lcd.print("*");
    }
  }
  tempFinal=millis()/1000;    
    if((tempFinal-tempInicio)>=5){
      lcd.clear();
      lcd.print("Ingrese Clave:");
      delay(250);
      lcd.setCursor(0,1);
      tempInicio=0;
      tempFinal=0;
      longitudCadena = 0;
    }
}
void color (unsigned char red, unsigned char green, unsigned char blue) // the color generating function
{
analogWrite(22, red);
analogWrite(24, blue);
analogWrite(26, green);
}



  

