/*
MULTI SECURITY SYSTEM USING ARDUINO WITH SENSORS
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This project demonstrates the basic use of sensors and actuators by applying it into a home lock security system

// INCLUDE NECESSARY LIBRARIES
#include <dht.h> // Library for DHT11 (Humidity and Temperature Sensor Module)
#include <Servo.h> // Library for SG90 (Servo Motor Module)
#include <LiquidCrystal_I2C.h> // Library for LCD 1602 Module
#include <Wire.h> // Library needed for LCD

// Set the LCD address to 0x27 for 16 characters and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// BUTTON PIN DECLARATIONS
const int SOUND_BUTTON = 2;
const int LIGHT_BUTTON = 3;
const int HUMIDITY_BUTTON = 4;
// BUTTON ACTIVATOR VARIABLE
int activateSound = 0;
int activateLight = 0;
int activateHumidity = 0;

// IDLE LEDS PIN DECLARATIONS
const int LEFT_LED = 5;
const int RIGHT_LED = 6;

// BOOLEAN VARIABLE FOR DOOR FLAG
bool isAccess = false;
// BOOLEAN VARIABLE FOR ROTARY DIRECTION FLAG
bool isClosing;

/* S E N S O R S */
// SOUND SENSOR
const int SOUND_PIN = A0;
// LIGHT SENSOR
int LIGHT_PIN = A1;
// HUMIDITY SENSOR
#define HUMIDITY_PIN 8
// SENSOR THRESHOLDS
const int SOUND_THRESHOLD = 90;
const int LIGHT_THRESHOLD = 150;
const int HUMIDITY_THRESHOLD = 70; //70 or 60 or 56 or 58

// Declare DHT object
dht DHT;
// Declare Servo object
Servo servo_obj;

// Define PIN Numbers for STEPPER PINS
#define STEPPER_PIN1 12
#define STEPPER_PIN2 11
#define STEPPER_PIN3 10
#define STEPPER_PIN4 9

// Declare step int variable for one step
int step_number = 0;

// Declare Delay Variables
int del = 500;
int del_door = 6000;

// SETUP
void setup() 
{
  //BUTTONS PIN MODE DECLARATIONS
  pinMode(SOUND_BUTTON, INPUT);
  pinMode(LIGHT_BUTTON, INPUT);
  pinMode(HUMIDITY_BUTTON, INPUT);

  //LEDS PIN MODE DECLARATIONS
  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);

  //SOUND SENSOR PIN MODE DECLARATION
  pinMode(SOUND_PIN, INPUT);

  //Set pinmodes for Stepper Motor
  pinMode(STEPPER_PIN1, OUTPUT);
  pinMode(STEPPER_PIN2, OUTPUT);
  pinMode(STEPPER_PIN3, OUTPUT);
  pinMode(STEPPER_PIN4, OUTPUT);

  //Declare Servo Motor DigitalPin
  servo_obj.attach(13);
  //Set the initial angular position of the servo motor's arm as a reference angle point for rotation
  servo_obj.write(0);

  lcd.begin(); //Start the LCD
  lcd.backlight(); //Turn ON the backlight of LCD
}

// MAIN LOOP
void loop() 
{
  //Clear LCD before execution
  lcd.clear();

  //Read Button Inputs
  ReadButtons();

  //PRESS LEFT BUTTON TO ACTIVATE SOUND SECURITY SYSTEM
  if (activateSound == HIGH && activateLight == LOW && activateHumidity == LOW)
  {
    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, LOW);
    SoundSecurity(isAccess);
  }
  //PRESS MIDDLE BUTTON TO ACTIVATE LIGHT SECURITY SYSTEM
  else if (activateSound == LOW && activateLight == HIGH && activateHumidity == LOW)
  {
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    LightSecurity(isAccess);
  }
  //PRESS RIGHT BUTTON TO ACTIVATE HUMIDITY SECURITY SYSTEM
  else if (activateSound == LOW && activateLight == LOW && activateHumidity == HIGH)
  {
    digitalWrite(RIGHT_LED, HIGH);
    digitalWrite(LEFT_LED, LOW);
    HumiditySecurity(isAccess);
  }
  //IDLE MODE WHICH LIGHTS THE LED TO SIGNIFY READY
  else if (activateSound == LOW && activateLight == LOW && activateHumidity == LOW)
  {
    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, HIGH);
    //LCD OUTPUT FOR IDLE
    lcd.setCursor(0,0);
    lcd.print("MULTI SECURITY");
    lcd.setCursor(0,1);
    lcd.print("SYSTEM");
  }
}

//FUNCTION TO READ BUTTONS
void ReadButtons()
{
  activateSound = digitalRead(SOUND_BUTTON);
  activateLight = digitalRead(LIGHT_BUTTON);
  activateHumidity = digitalRead(HUMIDITY_BUTTON);
}


//FUNCTION TO OPERATE SOUND SECURITY SYSTEM
void SoundSecurity(bool isAccess)
{
  bool verifySound = isAccess;

  while (verifySound == false)
  {
    lcd.clear();
    int soundValue = analogRead(SOUND_PIN);
    if (soundValue >= SOUND_THRESHOLD)
    {
      //LCD OUTPUT FOR VERIFIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Sound = ");
      lcd.print(soundValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS VERIFIED");
      //GATE FUNCTION TO OPEN AND CLOSE AFTER A DELAY
      //Open the Gate
      isClosing = false;
      AccessGate(isClosing);
      delay(del_door);
      //Close the Gate
      isClosing = true;
      AccessGate(isClosing);
      verifySound = true;
      delay(del_door);
    }
    else
    {
      //LCD OUTPUT FOR DENIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Sound = ");
      lcd.print(soundValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS DENIED");
      delay(del);
    }
  }
}


//FUNCTION TO OPERATE LIGHT SECURITY SYSTEM
void LightSecurity(bool isAccess)
{
  bool verifyLight = isAccess;
  while (verifyLight == false)
  {
    lcd.clear();
    int lightValue = analogRead(LIGHT_PIN);
    if (lightValue >= LIGHT_THRESHOLD)
    {
      //LCD OUTPUT FOR VERIFIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Light = ");
      lcd.print(lightValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS VERIFIED");
      //GATE FUNCTION TO OPEN AND CLOSE AFTER A DELAY
      //Open the Gate
      isClosing = false;
      AccessGate(isClosing);
      delay(del_door);
      //Close the Gate
      isClosing = true;
      AccessGate(isClosing);
      verifyLight = true;
      delay(del_door);
    }
    else
    {
      //LCD OUTPUT FOR DENIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Light = ");
      lcd.print(lightValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS DENIED");
      delay(del);
    }
  }
}


//FUNCTION TO OPERATE HUMIDITY SECURITY SYSTEM
void HumiditySecurity(bool isAccess)
{
  bool verifyHumidity = isAccess;
  while (verifyHumidity == false)
  {
    lcd.clear();
    int humtemp = DHT.read11(HUMIDITY_PIN);
    int humidityValue = DHT.humidity;
    if (humidityValue >= HUMIDITY_THRESHOLD)
    {
      //LCD OUTPUT FOR VERIFIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Humidity = ");
      lcd.print(humidityValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS VERIFIED");
      //GATE FUNCTION TO OPEN AND CLOSE AFTER A DELAY
      //Open the Gate
      isClosing = false;
      AccessGate(isClosing);
      delay(del_door);
      //Close the Gate
      isClosing = true;
      AccessGate(isClosing);
      verifyHumidity = true;
      delay(del_door);
    }
    else
    {
      //LCD OUTPUT FOR DENIED ACCESS
      lcd.setCursor(0,0);
      lcd.print("Humidity = ");
      lcd.print(humidityValue);
      lcd.setCursor(0,1);
      lcd.print("ACCESS DENIED");
      delay(del);
    }
  }
}

//FUNCTION FOR OPENING AND CLOSING THE GATE
void AccessGate(bool isClosing)
{
  float rotDis;
  bool isItClose = isClosing;
  if (isItClose == false)
    rotDis = 0;
  else
    rotDis = 90;
  for (int i = 0; i < 600; i++)
  {
    OneStep(isItClose);
    if (isItClose == false)
    {
      rotDis += 0.15;
      servo_obj.write(rotDis);
    }
    else
    {
      rotDis -= 0.15;
      servo_obj.write(rotDis);
    }
    delay(2);
  }
}


//FUNCTION FOR STEPPER MOTOR, ONE STEP AT A TIME
void OneStep(bool isClosing)
{
  if(isClosing == false)
  {
    switch(step_number)
    {
      case 0:
        digitalWrite(STEPPER_PIN1, HIGH);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, HIGH);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, HIGH);
        digitalWrite(STEPPER_PIN4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, HIGH);
        break;
    }
  }
  else
  {
    switch(step_number)
    {
      case 0:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, HIGH);
        digitalWrite(STEPPER_PIN4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN1, LOW);
        digitalWrite(STEPPER_PIN2, HIGH);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN1, HIGH);
        digitalWrite(STEPPER_PIN2, LOW);
        digitalWrite(STEPPER_PIN3, LOW);
        digitalWrite(STEPPER_PIN4, LOW); 
    }
  }
  step_number++;
  if(step_number > 3)
    step_number = 0;
}
