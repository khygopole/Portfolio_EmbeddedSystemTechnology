/*
IOT BASED COMFORT ROOM OCCUPANCY MONITORING SYSTEM
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This project demonstrates a Comfort Room Occupancy Monitoring System for Both Male and Female Sexes.
// An IoT feature is integrated to the system by utilizing ThingSpeak API and a WiFi connection for the
// writing of data to the ThingSpeak Channel.
// A NodeMCU ESP8266 Microcontroller was used to develop the system due to its WiFi feature that is absent
// from the Arduino UNO R3 Microcontroller utilized in other projects.

//Include Essential Libraries
#include <TM1637Display.h> //Library for 4DSSD
#include <LiquidCrystal_I2C.h> //Library for LCD 1602 Module
#include <Wire.h> //Library needed for LCD
#include <Servo.h> //Library for SG90 (Servo Motor Module)

//Module connection pins (Digital Pins)
#define CLK D5
#define DIO D6

// INITIALIZE EACH SEGMENT OF THE 4-DIGIT 7-SEGMENT DISPLAY MODULE
const uint8_t SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
    SEG_C | SEG_E | SEG_G,                           // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
    };

//Set the LCD address to 0x27 for 16 characters and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
//Set the Servo Motor Objects for the Two Comfort Room Gates (Male - Female)
Servo servoMale;
Servo servoFemale;
//Set object for TM1367 4-digit 7-segment Display
TM1637Display display(CLK, DIO);

// Create Variables for the Button
const int BUTTON = D0;
int theButton = 0;
// Create a Variable for the Sensor Value
int light;

const int LED1 = D8; //Male LED
const int LED2 = D7; //Female LED

// Create Constant Variables for the System
const int LIGHTPIN = A0;
const int ENTRY_THRESHOLD = 280;
const int EXIT_THRESHOLD = 130;
const int MAX = 9;

//Variables for Number of Person Per Comfort Room (Male - Female)
int males;
int females;

// Create Constant Variables for the Delay Variations
const int del = 1000;
const int delDoor = 5000;
const int delMess = 1250;

// Flag Variable to check activated comfort room
bool isMale;

// INCLUDE LIBRARIES FOR THE WIFI FUNCTIONALITY OF THE NODEMCU ESP8266
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <WiFiClient.h>

const char *myWriteAPIKey = "39WZ30I3KSEZ28KX"; // Enter your Write API key from ThingSpeak
unsigned long myChannelNumber = 2110764; // Enter the channel number from ThingSpeak
const char *ssid =  "Remi's Phone S22+"; // Enter the ID of the WiFi to be connected to
const char *pass =  "hellohowareyou"; // Enter the WiFi password where the system will connect to
const char* server = "api.thingspeak.com"; // Initialize the ThingSpeak http server

// Create an Object for the WiFi Client connection
WiFiClient client;

// SETUP
void setup() 
{
  // SET PINMODES FOR THE INPUT AND OUTPUT COMPONENTS
  pinMode(BUTTON, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  lcd.begin(); //Start the LCD
  //Turn on the backlight of the LCD
  lcd.backlight();
  lcd.clear();

  //Indicate Pin Number of Servo Motor
  servoMale.attach(D3);
  servoFemale.attach(D4);

  //Set the initial angular position of the motor's arm as the reference angle (0 degrees)
  servoMale.write(0);
  servoFemale.write(0);

  // Initialize Variables with their Default Values
  males = 0;
  females = 0;
  isMale = true;
  
  // Use Serial Monitor to Verify Connection with the WiFi and ThingSpeak API
  Serial.begin(115200);
  delay(10);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  ThingSpeak.begin(client);
}

// MAIN LOOP
void loop() 
{
  Preamble();
  light = CheckPrompt(&isMale);
  if (light >= ENTRY_THRESHOLD)
    Entry(&males, &females, &isMale);
  else if (light >= EXIT_THRESHOLD)
    Exit(&males, &females, &isMale);
  Operation(&males, &females, &isMale);
  delay(600);
}

// FUNCTION FOR THE WELCOME MESSAGE IN IDLE
void Preamble()
{
  lcd.setCursor(0,0);
  lcd.print("   WELCOME TO   ");
  lcd.setCursor(0,1);
  lcd.print("  COMFORT ROOM  ");
}

// FUNCTION TO CHECK IF OPERATION IS MODIFIED IN THE COMFORT ROOM (MALE OR FEMALE)
int CheckPrompt(bool *isMale)
{
  // Read from the button
  theButton = digitalRead(BUTTON);
  // If button is pressed while Male Comfort Room is Activated, Transition to Female Comfort Room for Activation
  if (theButton == HIGH && *isMale == true)
  {
    *isMale = false;
    lcd.setCursor(0,0);
    lcd.print(" FEMALE COMFORT ");
    lcd.setCursor(0,1);
    lcd.print(" ROOM TURNED ON ");
    delay(1250);
    lcd.clear();
  }
  // If button is pressed while Female Comfort Room is Activated, Transition to Male Comfort Room Activation
  else if (theButton == HIGH && *isMale == false)
  {
    *isMale = true;
    lcd.setCursor(0,0);
    lcd.print("  MALE COMFORT  ");
    lcd.setCursor(0,1);
    lcd.print(" ROOM TURNED ON ");
    delay(1250);
    lcd.clear();
  }
  // Start Reading from the Photoresistor then return the value
  int theLight = analogRead(LIGHTPIN);
  Serial.println(theLight);
  return theLight;
}

// FUNCTION TO PROMPT ENTRY FOR EITHER MALE OR FEMALE COMFORT ROOM AT A TIME
void Entry(int *males, int *females, bool *isMale)
{
  if (*isMale == true)
  {
    // If Male Comfort Room is not at Maximum Capacity, Verify Entry
    if (*males != MAX)
    {
      *males += 1;
      lcd.setCursor(0,0);
      lcd.print("   MALE ENTRY   ");
      lcd.setCursor(0,1);
      lcd.print("   VERIFIED!!   ");
      GateAccess(*isMale);
      // Write the Updated Data to the ThingSpeak Channel Corresponding to the Male Comfort Room Graph
      ThingSpeak.writeField(myChannelNumber, 1, *males, myWriteAPIKey);
      delay(200);
      lcd.clear();
    }
    else
    {
      // If Male Comfort Room is at Max capacity, Prohibit Entry
      lcd.setCursor(0,0);
      lcd.print("  MALE COMFORT  ");
      lcd.setCursor(0,1);
      lcd.print(" ROOM IS FULL!! ");
      delay(delMess);
      lcd.clear();
    }
  }
  else
  {
    // If Female Comfort Room is not at Maximum Capacity, Verify Entry
    if (*females != MAX)
    {
      *females += 1;
      lcd.setCursor(0,0);
      lcd.print("  FEMALE ENTRY  ");
      lcd.setCursor(0,1);
      lcd.print("   VERIFIED!!   ");
      GateAccess(*isMale);
      // Write the Updated Data to the ThingSpeak Channel Corresponding to the Female Comfort Room Graph
      ThingSpeak.writeField(myChannelNumber, 2, *females, myWriteAPIKey);
      delay(200);
      lcd.clear();
    }
    else
    {
      // If Female Comfort Room is at Maximum Capacity, Prohibit Entry
      lcd.setCursor(0,0);
      lcd.print(" FEMALE COMFORT ");
      lcd.setCursor(0,1);
      lcd.print(" ROOM IS FULL!! ");
      delay(delMess);
      lcd.clear();
    }
  }
}

// FUNCTION TO PROMPT EXIT FOR EITHER COMFORT ROOM AT A TIME
void Exit(int *males, int *females, bool *isMale)
{
  if (*isMale == true)
  {
    // If Male Comfort Room is Not Empty, Verify Exit
    if (*males != 0)
    {
      *males -= 1;
      lcd.setCursor(0,0);
      lcd.print("   MALE EXIT    ");
      lcd.setCursor(0,1);
      lcd.print("   VERIFIED!!   ");
      GateAccess(*isMale);
      // Write the Updated Data to the ThingSpeak Channel Corresponding to the Male Comfort Room Graph
      ThingSpeak.writeField(myChannelNumber, 1, *males, myWriteAPIKey);
      delay(200);
      lcd.clear();
    }
    else
    {
      // If Male Comfort Room is Empty, Prohibit Exit
      lcd.setCursor(0,0);
      lcd.print("  MALE COMFORT  ");
      lcd.setCursor(0,1);
      lcd.print(" ROOM IS EMPTY! ");
      delay(delMess);
      lcd.clear();
    }
  }
  else
  {
    // If Female Comfort Room is not Empty, Verify Exit
    if (*females != 0)
    {
      *females -= 1;
      lcd.setCursor(0,0);
      lcd.print("  FEMALE EXIT   ");
      lcd.setCursor(0,1);
      lcd.print("   VERIFIED!!   ");
      GateAccess(*isMale);
      // Write the Updated Data to the ThingSpeak Channel Corresponding to the Female Comfort Room Graph
      ThingSpeak.writeField(myChannelNumber, 2, *females, myWriteAPIKey);
      delay(200);
      lcd.clear();
    }
    else
    {
      // If Female Comfort Room is Empty, Prohibit Exit
      lcd.setCursor(0,0);
      lcd.print(" FEMALE COMFORT ");
      lcd.setCursor(0,1);
      lcd.print(" ROOM IS EMPTY! ");
      delay(delMess);
      lcd.clear();
    }
  }
}

// FUNCTION TO OPERATE THE SERVO MOTOR FOR GATE OPENING AND CLOSING
void GateAccess(bool &isMale)
{
  int rotDis = 180;
  // If the Flag is True, Operate the Male Comfort Room Gate
  if (isMale)
  {
    for (int i = 0; i < rotDis; i++)
    {
      servoMale.write(i);
      delay(1);
    }
    delay(delDoor);
    for (int i = rotDis; i > 0; i--)
    {
      servoMale.write(i);
      delay(1);
    }
    delay(del/2);
  }
  else
  {
    // If Flag is False, Operate Female Comfort Room Gate
   for (int j = 0; j < rotDis; j++)
   {
     servoFemale.write(j);
     delay(1);
   }
   delay(delDoor);
   for (int j = rotDis; j > 0; j--)
   {
     servoFemale.write(j);
     delay(1);
   }
   delay(del/2);
  }
}

// FUNCTION FOR THE OVERALL OPERATION OF THE LOGIC FOR THE COMFORT ROOM CAPACITY DISPLAY USING 4-DIGIT 7-SEGMENT DISPLAY MODULE
void Operation(int *males, int *females, bool *isMale)
{
  // Activate LED Corresponding to Male Comfort Room
  if (*isMale)
  {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
  }
  else
  {
    // Activate LED Corresponding to Female Comfort Room
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
  }
  // Initialize Variables for Each Digit of the 4-Digit 7-Segment Display
  int num1;
  int num2;
  int num3;
  int num4;

  // Extract Each Digit from the Number of Capacity in the Male Comfort Room
  if (*males >= 10)
  {
    num1 = *males / 10;
    num2 = *males % 10;
  }
  else
  {
    num1 = 0;
    num2 = *males;
  }
  // Extract Each Digit from the Number of Capacity in the Female Comfort Room
  if (*females >= 10)
  {
    num3 = *females / 10;
    num4 = *females % 10;
  }
  else
  {
    num3 = 0;
    num4 = *females;
  }
  // Initialize the Memory Segments for the 4-Digit 7-Segment Display
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
  // Set the Brightness for the 7-Segment Display (0 dimmest, 7 brightest, Recommended is 2)
  display.setBrightness(2);
  
  // Encode Each of the Extracted Digit from the Capacities in Either Comfort Room (Format: data 0123)
  data[0] = display.encodeDigit(num1);
  data[1] = display.encodeDigit(num2);
  data[2] = display.encodeDigit(num3);
  data[3] = display.encodeDigit(num4);
  // Display the Encoded Digits
  display.setSegments(data);
}
