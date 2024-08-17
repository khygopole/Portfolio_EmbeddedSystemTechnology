/*
REAL-TIME GARAGE PARKING SYSTEM USING ARDUINO WITH COUNTER AND KEYPAD
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This project demonstrates the utilization of complex actuator like the 4x4 keypad then sensor for
// an efficient garage parking system. The garage can sense if there is an incoming vehicle
// based on its headlights. Entry and Exits are permitted when specific conditions are satisfied.


//Include Necessary Libraries for LCD, 4x4 keypad module, and Servo Motor
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>
#include <Servo.h>

//Set the LCD address to 0x27 for 16 characters and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Declare Servo Object
Servo servo_obj;

//Declare Keypad Variables
const byte ROWS = 4;
const byte COLS = 4;
//Declare Each Character of the Keypads in a 4x4 Array
char keyMap [ROWS] [COLS] = 
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
//Declare Keypad Pins (for each rows and column)
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 0}; //4TH COLUMN IS NOT USED SO USE PIN 0 AS PLACEHOLDER
//Declare Keypad Object
Keypad myKey = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);

//DECLARE SINGLE-DIGIT SEVEN-SEGMENT PINS - COUNTER
const int A = 9;
const int B = 10;
const int C = 11;
const int D = 12;
const int E = 13;
const int F = 14;
const int G = 15;

//Declare Pin Variable for Photoresistor
const int LIGHTSENSOR = A2;
const int LIGHT_THRESHOLD = 103;

//Declare PIN Code variables
char* pinCode = "041188";
int pos = 0; //To check the current position of the driver's PIN
//Declare Character Key to enable Exit
const char EXIT = '*';
//Declare Character Key to cancel entry
const char CANCEL = '#';
//Declare Alternate Key to Enter (if car does not have light)
const char ALT = '#';

//Declare Variable for Counter
int vehicles = 0;
//Declare MAX Counter
const int MAX = 9;
//Declare bool for exit
bool isExit = false;

//Adjust Delay for UI
const int del = 1000;
const int delDoor = 5000;

// SETUP
void setup() 
{
  //Set pinmodes for single-digit seven-segment display pin connections
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);

  //Declare Servo Motor Pin
  servo_obj.attach(17);
  //Set the initial angular position of the servo motor's arm as a reference angle point for rotation
  servo_obj.write(0);

  lcd.begin(); //Start the LCD
  lcd.backlight(); //Turn ON the backlight of LCD
  lcd.clear();
}

// MAIN LOOP
void loop() 
{
  Preamble();
  bool isEntry = CheckLight(&vehicles, &isExit);
  EnterKey(isEntry, &pos, &vehicles);
  Exit(&vehicles, &isExit);
  DisplayCounter(&vehicles);
}


//VOID FUNCTIONS FOR EACH OUTPUT OF 7-SEGMENT DISPLAY COUNTER
void zero()
{
  //7 SEGMENT OUTPUT = 0
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, HIGH);
  digitalWrite(F, HIGH);
  digitalWrite(G, LOW);
}

void one()
{
  //7 SEGMENT OUTPUT = 1
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, LOW);
}

void two()
{
  //7 SEGMENT OUTPUT = 2
  digitalWrite(A,HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, LOW);
  digitalWrite(D, HIGH);
  digitalWrite(E, HIGH);
  digitalWrite(F, LOW);
  digitalWrite(G, HIGH);
}

void three()
{
  //7 SEGMENT OUTPUT = 3
  digitalWrite(A,HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, HIGH);
}

void four()
{
  //7 SEGMENT OUTPUT = 4
  digitalWrite(A,LOW);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, HIGH);
  digitalWrite(G, HIGH);
}

void five()
{
  //7 SEGMENT OUTPUT = 5
  digitalWrite(A,HIGH);
  digitalWrite(B, LOW);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, LOW);
  digitalWrite(F, HIGH);
  digitalWrite(G, HIGH);
}

void six()
{
  //7 SEGMENT OUTPUT = 6
  digitalWrite(A,HIGH);
  digitalWrite(B, LOW);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, HIGH);
  digitalWrite(F, HIGH);
  digitalWrite(G, HIGH);
}

void seven()
{
  //7 SEGMENT OUTPUT = 7
  digitalWrite(A,HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
  digitalWrite(F, LOW);
  digitalWrite(G, LOW);
}

void eight()
{
  //7 SEGMENT OUTPUT = 8
  digitalWrite(A,HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, HIGH);
  digitalWrite(F, HIGH);
  digitalWrite(G, HIGH);
}

void nine()
{
  //7 SEGMENT OUTPUT = 9
  digitalWrite(A,HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, HIGH);
  digitalWrite(E, LOW);
  digitalWrite(F, HIGH);
  digitalWrite(G, HIGH);
}

//Function to display number on the 7-segment display
void DisplayCounter(int *vehicles)
{
  switch(*vehicles)
  {
    case 0:
      zero();
      break;
    case 1:
      one();
      break;
    case 2:
      two();
      break;
    case 3:
      three();
      break;
    case 4:
      four();
      break;
    case 5:
      five();
      break;
    case 6:
      six();
      break;
    case 7:
      seven();
      break;
    case 8:
      eight();
      break;
    case 9:
      nine();
      break;
  }
}

// WELCOME MESSAGE BY DEFAULT
void Preamble()
{
  lcd.setCursor(0,0);
  lcd.print("   WELCOME TO   ");
  lcd.setCursor(0,1);
  lcd.print(" GARAGE PARKING ");
}

// FUNCTION TO CHECK IF A VEHICLE IS TRYING TO ENTER BASED ON ITS LIGHT
bool CheckLight(int *vehicles, bool *isExit)
{
  // READ PHOTORESISTOR
  int carLight = analogRead(LIGHTSENSOR);
  // READ FROM THE 4X4 KEYPAD
  char newKey = myKey.getKey();
  if (carLight >= LIGHT_THRESHOLD || newKey == ALT)
  {
    // SEND FLAG FOR POSSIBLE ENTRY
    if (*vehicles < MAX)
      return true;
    else
    {
      // DENY ENTRY IF GARAGE IS MAX
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("APOLOGIES GARAGE");
      lcd.setCursor(0,1);
      lcd.print("PARKING IS FULL");
      delay(del);
      lcd.clear();
      return false;
    }
  }
  else if (newKey == EXIT)
  {
    // PROCT EXIT
    *isExit = true;
    return false;
  }
  else
    return false;
}

// FUNCTION TO EXTRACT PIN ENTERED FROM THE 4X4 KEYPAD
void EnterKey(bool isEntry, int *pos, int *vehicles)
{
  // PROMPT ENTRY
  if (isEntry)
  {
    while (true)
    {
      lcd.setCursor(0,0);
      lcd.print("ENTRY PROCESSING");
      lcd.setCursor(0,1);
      lcd.print(" ENTER PIN CODE ");
      // READ KEY BY KEY FROM 4X4 KEYPAD
      char key = myKey.getKey();
      if (key != pinCode[*pos] && key != NO_KEY && key != CANCEL)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  INVALID KEY!  ");
        lcd.setCursor(0,1);
        lcd.print("ENTER PIN AGAIN!");
        *pos = 0;
        delay(del);
        lcd.clear();
      }
      else if (key == CANCEL)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  OPERATION HAS ");
        lcd.setCursor(0,1);
        lcd.print(" BEEN CANCELLED ");
        delay(del);
        lcd.clear();
        *pos = 0;
        break;
      }
      else if (key == pinCode[*pos])
      {
        // INCREASE POS IF CHARACTER BY CHARACTER FOR PIN IS CORRECT
        *pos += 1;
      }
      // IF POS IS 6, IT MEANS ALL CHARACTERS FOR THE ENTERED PIN ARE CORRECT, PERMIT ENTRY
      if (*pos == 6)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(" ENTRY VERIFIED ");
        lcd.setCursor(0,1);
        lcd.print("OPENING THE GATE");
        // OPEN THE GATE THEN CLOSE AFTER ENTRY
        GateAccess();
        // INCREASE THE NUMBER OF PARKED CAR BY 1
        *vehicles += 1;
        // RESET THE POS TO 0 FOR THE NEXT OPERATION
        *pos = 0;
        lcd.clear();
        break;
      }
    }
  }
}

// FUNCTION FOR THE OPENING AND CLOSING OF THE GATE
void GateAccess()
{
  int rotDis = 180;
  for (int i = 0; i < rotDis; i++)
  {
    servo_obj.write(i);
    delay(1);
  }
  delay(delDoor);
  for (int i = rotDis; i > 0; i--)
  {
    servo_obj.write(i);
    delay(1);
  }
  delay(del/2);
}

// FUNCTION TO EXECUTE IF EXIT IS ACTIVATED IN THE GARAGE
void Exit(int *vehicles, bool *isExit)
{
  if (*isExit)
  {
    // IF VEHICLE IS NOT EMPTY, PROCEED WITH THE EXIT
    if (*vehicles > 0)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" EXIT VERIFIED ");
      lcd.setCursor(0,1);
      lcd.print("OPENING THE GATE");
      GateAccess();
      // DECREMENT THE NUMBER OF PARKED VEHICLE IN THE GARAGE BY 1
      *vehicles -= 1;
      *isExit = false;
      lcd.clear();
    }
    else
    {
      // IF GARAGE IS CURRENTLY EMPTY, DO NOT ALLOW EXIT AS IT IS INVALID
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("GARAGE IS EMPTY!");
      lcd.setCursor(0,1);
      lcd.print("    INVALID!    ");
      *isExit = false;
      delay(del);
      lcd.clear();
    }
  }
}
