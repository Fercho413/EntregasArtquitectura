
//LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
const int LCD1 = 12;
const int LCD2 = 11;
const int LCD3 = 10;
const int LCD4 = 9;
const int LCD5 = 8;
const int LCD6 = 7;

//DHT
const int DHTPIN = 3;
const int photocellPin = A5;

//led
const int redPin = 22; 
const int bluePin = 24;
const int greenPin = 26;


const int pinBuzzer= 30;
const int intPinHall = 2;
const int intPinMetal = 20;
const int intPinTracking = 19;

//KEYPAD
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {28, 4, 5, 6};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A4};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
