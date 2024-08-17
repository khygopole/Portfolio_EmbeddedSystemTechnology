/*
FUME EXTRACTOR WITH WARNING SYSTEM FOR POWDER SHAKER & DRYER USING ATMEGA328P
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This system makes use of an ATmega328P as its microcontroller. Its primary purpose is to exhaust fumes from powder shaker & dryer machines that are operated along with plotter printers for heavy printing. 
// The fumes exhausted have high temperature which will condense and become fluid once exhausted. Thus, a warning system is integrated to the system using LEDs and Buzzer with a combinational circuit.
// This informs the user when to clean the basin of the system for longevity and safety.

// Declare Pins
const int FAN1 = 5; //PD5 - Pin 11
const int FAN2 = 3;// PD3 - Pin 5
const int USS_TRIGPIN = 13; // PB5 - Pin 19
const int USS_ECHOPIN = 7; //PD7 - Pin 13
const int LED_LOW = 2; // PD0 - Pin 4
const int LED_MEDIUM = 10; // PB2 - Pin 16
const int LED_HIGH = 11; // PB3 - Pin 17
const int BUTTON = 12;//PB4 - Pin 18

// Pins for the Signal to be sent into the Fluid Warning System Combinational Circuit
const int DO1 = 4; //PD4 - pin 6
const int DO2 = 8; //PB0 - pin 14

// Declare Ultrasonic Variables
const float USS_THRESHOLD1 = 0.8;
const float USS_THRESHOLD2 = 1.2;
const float USS_THRESHOLD3 = 1.6;
const float USS_THRESHOLD4 = 2;
const int GAP = 5.10;
const float SPEED_OF_SOUND = 0.034;
long duration;
float level;

//Declare Fan Speed Variables
//from 0 to 255
const int FAN_LOW = 150;
const int FAN_MEDIUM = 200;
const int FAN_HIGH = 220;

// Declare Button Variable
int currentButton;
int previousButton;
int pressCount;

void setup()
{
  // pin mode for fans
  pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);
  analogWrite(FAN1, 0);
  analogWrite(FAN2, 0);
  
  // Add a delay for the fans activation (rotation)
  delay(5000);

  //Set the Pin Mode of the Sensors and Actuators
  pinMode(USS_TRIGPIN, OUTPUT);
  pinMode(USS_ECHOPIN, INPUT);
  pinMode(LED_LOW, OUTPUT);
  pinMode(LED_MEDIUM, OUTPUT);
  pinMode(LED_HIGH, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(DO1, OUTPUT);
  pinMode(DO2, OUTPUT);
  
  //Set Initial Values for Button
  currentButton = 0;
  previousButton = 0;
  pressCount = 0;
}

// MAIN PROGRAM LOOP
void loop()
{
  OperateFan(&pressCount, &currentButton, &previousButton);
  level = GetLevel();
  SetDigitalOutput(level);
}

// READ BUTTON AND ACCEPT PREVIOUS AND CURRENT BUTTON STATES ALONG WITH PRESS COUNT AS ARGUMENT
int ReadButton(int &pressCount, int &currentButton, int &previousButton)
{
  currentButton = digitalRead(BUTTON);
  if (currentButton != previousButton) 
  {
    if (currentButton == HIGH)
      if (pressCount == 2)
        pressCount = 0;
      else
        pressCount += 1;
    delay(100);
  }
  //Initialize Current Button State into Previous Button State
  previousButton = currentButton;
  return pressCount;
}

// FUNCTION TO READ THE FLUID SENSED BY THE USS IN THE BASIN
float GetLevel() 
{
  //delay(50);
  digitalWrite(USS_TRIGPIN, LOW);
  digitalWrite(USS_TRIGPIN, HIGH);
  //delayMicroseconds(10);
  digitalWrite(USS_TRIGPIN, LOW);
  duration = pulseIn(USS_ECHOPIN, HIGH);
  level = GAP - ((duration * SPEED_OF_SOUND) / 2);
  return level;
}

// FUNCTION FOR THE OPERATION OF THE FANS
void OperateFan(int *pressCount, int *currentButton, int *previousButton)
{
  *pressCount = ReadButton(*pressCount, *currentButton, *previousButton);

  switch (*pressCount)
  {
    //Triggers: { Fans Speed: Medium, Speed Indicator: Medium }
    case 1:
      analogWrite(FAN1, FAN_MEDIUM);
      analogWrite(FAN2, FAN_MEDIUM);
      digitalWrite(LED_LOW, LOW);
      digitalWrite(LED_MEDIUM, HIGH);
      digitalWrite(LED_HIGH, LOW);
      break;
    //Triggers: { Fans Speed: High, Speed Indicator: High }
    case 2:
      analogWrite(FAN1, FAN_HIGH);
      analogWrite(FAN2, FAN_HIGH);
      digitalWrite(LED_LOW, LOW);
      digitalWrite(LED_MEDIUM, LOW);
      digitalWrite(LED_HIGH, HIGH);
      break;
    //Triggers: { Fans Speed: Low, Speed Indicator: Low}
    default:
      analogWrite(FAN1, FAN_LOW);
      analogWrite(FAN2, FAN_LOW);
      digitalWrite(LED_LOW, HIGH);
      digitalWrite(LED_MEDIUM, LOW);
      digitalWrite(LED_HIGH, LOW);
      break;
  }
}

// FUNCTION FOR THE SIGNAL TO BE SENT INTO THE WARNING SYSTEM COMBINATIONAL CIRCUIT
void SetDigitalOutput(float level)
{
  // [0-0.8cm] Triggers Low Fluid Level Indicator (00 - Green LED)
  if (0 <= level && level <= USS_THRESHOLD1)
  {
    digitalWrite(DO1, LOW);
    digitalWrite(DO2, LOW);
  }
  // (0.8cm-1.2cm] Triggers Medium Fluid Level Indicator (01 - Yellow LED)
  else if (USS_THRESHOLD1 < level && level <= USS_THRESHOLD2)
  {
    digitalWrite(DO1, LOW);
    digitalWrite(DO2, HIGH);
  }
  // (1.2cm-1.6cm] Triggers High Fluid Level Indicator (10 - Buzzer)
  else if (USS_THRESHOLD2 < level && level <= USS_THRESHOLD3)
  {
    digitalWrite(DO1, HIGH);
    digitalWrite(DO2, LOW);
  }
  // (1.6cm-2cm] Triggers Critical Fluid Level Indicator (11 - Buzzer, Green, and Yellow LEDs)
  else if (USS_THRESHOLD3 < level && level <= USS_THRESHOLD4)
  {
    digitalWrite(DO1, HIGH);
    digitalWrite(DO2, HIGH);
  }
}
