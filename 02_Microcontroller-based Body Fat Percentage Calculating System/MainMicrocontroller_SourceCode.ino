/*
MICROCONTROLLER-BASED AUTOMATED BODY FAT PERCENTAGE CALCULATING SYSTEM
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This is the main system which handles the height extraction, controls for sex selection and measurement traversing, and body fat percentage calculation.
// The body circumference measurements for neck, waist, and hips are received by the system from the secondary microcontroller.

//Main Microcontroller Program

// Include essential libraries (LCD, Math operations)
#include <Wire.h> 
#include <math.h> 
#include <LiquidCrystal_I2C.h>
//Setup nRF24L01 Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Set nRF24L01
RF24 radio(7, 8); //CE and CSN
const byte addresses[][6] = {"EDCBA", "ABCDE"};

const int MALE_BUTTON=6; //BUTTON PIN FOR MALES
const int FEMALE_BUTTON=5; //BUTTON PIN FOR FEMALES
//ULTRASONIC SENSOR PINS
const int trigPin = 10;
const int echoPin = 9;

//BUTTON STATE DECLARATIONS
boolean prevButtonState = LOW;    // Previous Button State
boolean currentButtonState = LOW; // Current Button State
int buttonMode = 0; // Mode Variable to Cycle Between Measurements

//HEIGHT VARIABLES
const int DISTANCE_FROM_GROUND = 197;
long duration;
int distance;
int height; //Variable used for a sample's height

//Placeholder for Body Circumference Measurement
float bodyCirc; 

// Body Circumference Measurement Variables
float *neck; //Variable used for a sample's neck circumference
float *waist; //Variable used for a sample's waist circumference
float *hip;   //Variable used for hip circumference measurement exclusive only for female samples

float bfp = 0; //BF% VARIABLE
const int reset_bodyCirc = 1; //Flag To Reset Circumference in Measuring Device 

bool isMale; //Boolean variable to check if sample is male or not depending on the pressed button (left = female or right = male)

void setup()
{ 
  //Setup Button and Ultrasonic Sensor Pins as Input/Output
  pinMode (MALE_BUTTON, INPUT);
  pinMode (FEMALE_BUTTON, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Setup lcd
  lcd.init();
  lcd.backlight();
  //Setup Transmission and Receival
  radio.begin();
  radio.openWritingPipe(addresses[1]); //ABCDE
  radio.openReadingPipe(0, addresses[0]); //EDCBA
  radio.setPALevel(RF24_PA_MIN);
}

//FUNCTION FOR ACCEPTING DATA FROM SECONDARY MICROCONTROLLER
void GetBodyCircumference(float *bodyCirc)
{
  delay(5);
  radio.startListening();
  if (radio.available()) 
  {
    float bodyCircumference = 0.00;
    radio.read(&bodyCircumference, sizeof(bodyCircumference));
    *bodyCirc = bodyCircumference;
  }
  delay(5);
  radio.stopListening();
}

//DEBOUNCE FUNCTION FOR MALES
boolean DebounceMale(boolean last)
{
  boolean current = digitalRead(MALE_BUTTON); //Read button state
  if (last != current) //If states are different = pressed
  {
    delay(5);
    current = digitalRead(MALE_BUTTON); //Read again
  }
  return current;
}

//DEBOUNCE FUNCTION FOR FEMALES
boolean DebounceFemale(boolean last)
{
  boolean current = digitalRead(FEMALE_BUTTON); // Read button state
  if (last != current) //If states are different = pressed
  {
    delay(5);
    current = digitalRead(FEMALE_BUTTON); // Read again
  }
  return current;
}

//FUNCTION TO EXTRACT HEIGHT USING ULTRASONIC SENSOR
void GetHeight()
{
  //Start Ultrasonic Sensor Operation
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure the response from the HC-SR04 Echo Pin in us (microseconds)
  duration = pulseIn(echoPin, HIGH);

  // Determine distance from duration
  // Use 0.034 cm/us as speed of sound
  distance = duration*0.034/2;
  height = DISTANCE_FROM_GROUND - distance;
  //PRINT THE HEIGHT TO THE LCD
  lcd.setCursor(0,0);
  lcd.print("HEIGHT:             ");
  lcd.setCursor(0, 1);
  lcd.print(height);
  lcd.print("  cm         ");
}

// FUNCTION TO EXTRACT NECK CIRCUMFERENCE
void GetNeck(float &bodyCirc)
{
  GetBodyCircumference(&bodyCirc);
  neck = &bodyCirc;
  lcd.setCursor(0,0);
  lcd.print("NECK:             ");
  lcd.setCursor(0, 1);
  lcd.print(*neck);
  lcd.print("  cm         ");
  delay(1);
}

// FUNCTION TO EXTRACT WAISR CIRCUMFERENCE
void GetWaist(float &bodyCirc)
{
  GetBodyCircumference(&bodyCirc);
  waist = &bodyCirc;
  lcd.setCursor(0,0);
  lcd.print("WAIST:            ");
  lcd.setCursor(0, 1);
  lcd.print(*waist);
  lcd.print("  cm         ");
  delay(1);
}
// FUNCTION TO EXTRACT HIP CIRCUMFERENCE
void GetHip(float &bodyCirc)
{
  GetBodyCircumference(&bodyCirc);
  hip = &bodyCirc;
  lcd.setCursor(0,0);
  lcd.print("HIP:              ");
  lcd.setCursor(0, 1);
  lcd.print(*hip);
  lcd.print("  cm         ");
  delay(1);
}

// FUNCTION TO CALCULATE BF% OF A SAMPLE
void CalculateBFP()
{
  if (isMale)
    bfp = (495.00/(1.0324-0.19077*log10(*waist-*neck)+0.15456*log10(height)))-450.00;
  else
    bfp = (495/(1.29579-0.35004*log10(*waist+*hip-*neck)+0.22100*log10(height)))-450.00;
  lcd.setCursor(0, 0);
  lcd.print("BF%:             ");
  lcd.setCursor(0, 1);
  lcd.print(bfp);
  lcd.print("  %        ");
}

// FUNCTION TO SELECT SEX (DEFAULT)
void SelectSex()
{
  lcd.setCursor(0, 0);
  lcd.print("LBtn: Female            "); //left button for female samples
  lcd.setCursor(0, 1);
  lcd.print("RBtn: Male              "); //right button for male samples
  if (digitalRead(MALE_BUTTON) == HIGH) //If right button is pressed, set isMale to true
    isMale = true;
  else if (digitalRead(FEMALE_BUTTON) == HIGH) //If left button is pressed, set isMale to false
    isMale = false;
  // RESET VARIABLES BEFORE PROCEEDING TO THE NEXT SAMPLE
  *neck = 0;
  *waist = 0;
  *hip = 0;
}

//FUNCTION FOR MALE SAMPLE BF% CALCULATION
void OperateMale(int maleMode, float *bodyCirc)
{
  switch (maleMode)
  {
    case 1: // MALE HEIGHT EXTRACTION
      GetHeight();
      break;
    case 2: // MALE NECK EXTRACTION
      GetNeck(*bodyCirc);
      break;
    case 3: // MALE WAIST EXTRACTION
      GetWaist(*bodyCirc);
      break;
    case 4: // BF% CALCULATION OF A MALE SAMPLE
      CalculateBFP();
      break;
    default: // DEFAULT STATE: SEX SELECTION (FEMALE = LEFT BUTTON, MALE = RIGHT BUTTON)
      SelectSex();
      break;
  }
}

//FUNCTION FOR FEMALE SAMPLE BF% CALCULATION
void OperateFemale(int femaleMode, float* bodyCirc)
{
  switch (femaleMode)
  {
    case 1: // FEMALE HEIGHT EXTRACTION
      GetHeight();
      break;
    case 2: // FEMALE NECK EXTRACTION
     GetNeck(*bodyCirc);
      break;
    case 3: // FEMALE WAIST EXTRACTION
      GetWaist(*bodyCirc);
      break;
    case 4: // FEMALE HIP EXTRACTION
      GetHip(*bodyCirc);
      break;
    case 5: // BF% CALCULATION OF A FEMALE SAMPLE
      CalculateBFP();
      break;
    default: // DEFAULT STATE: SEX SELECTION (FEMALE = LEFT BUTTON, MALE = RIGHT BUTTON)
      SelectSex();
      break;
  }
}

void loop(){ 
  //MALE
  if (isMale)
  {
    currentButtonState = DebounceMale(prevButtonState); // Read State
    if (prevButtonState == LOW && currentButtonState == HIGH) // Check if pressed
    {
      buttonMode++; // Increment the button count to traverse the next measurement
      lcd.clear();
      bodyCirc = 0.00;
      if (buttonMode == 2 || buttonMode == 3) //Reset Measurement in Measuring Device
      {
        radio.write(&reset_bodyCirc, sizeof(reset_bodyCirc));
        delay(2000);
      }
    }
    prevButtonState = currentButtonState; // Initialize current state into previous state
    // If all measurements were completely traversed
    // reset the button count to zero (0)
    if (buttonMode == 5)
      buttonMode = 0;
    OperateMale(buttonMode, &bodyCirc); // Execute Male Mode
  }
  //FEMALE
  else
  {
    currentButtonState = DebounceFemale(prevButtonState); // Read State
    if (prevButtonState == LOW && currentButtonState == HIGH) // Check if pressed
    {
      buttonMode++; // Increment the button count to traverse the next measurement
      lcd.clear();
      bodyCirc = 0.00;
      if (buttonMode == 2 || buttonMode == 3 || buttonMode == 4) //Reset Measurement in Measuring Device
      {
        //Serial.println(buttonMode);
        radio.write(&reset_bodyCirc, sizeof(reset_bodyCirc));
        delay(2000);
      }
    }
    prevButtonState = currentButtonState; // Initialize current state into previous state
    // If all measurements were completely traversed
    // reset the button count to zero (0)
    if (buttonMode == 6)
      buttonMode = 0;
    OperateFemale(buttonMode, &bodyCirc);      
  }
}
