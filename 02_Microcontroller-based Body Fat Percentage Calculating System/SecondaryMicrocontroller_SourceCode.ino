/*
BODY FAT PERCENTAGE ESTIMATION USING MICROCONTROLLERS
BY: GOPOLE, KHYLE MATTHEW P.
*/

// This handles the body circumference extraction using rotary encoder. The measurement is then sent to the Main Microcontroller wirelessly.

//Secondary Microcontroller Program

// Include essential libraries for the transceiver module
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Define Rotary Encoder Pins
#define A 2
#define B 3

//CIRCUMFERENCE VARIABLES
float bodyCircumference; // Body Circumference Measurement Variable
int rotStep; // Number of Steps of the Rotary Encoder
int temp; // Previous Number of Step of the Rotary Encoder
const float WHEEL_CIRC = 21.50; // Circumference of the Wheel attached to Rotary Encoder
const float ROTARY_STEPS = 1200; // Number of steps equivalent to a single rotation based on Rotary Encoder Specification
const float CALIBRATION_VALUE = 6.60; // Calibration Value (Constant Value to be Added to Make Reading More Accurate Based on the Magnets' Placement)
int reset_bodyCirc; // Flag Variable to Reset the Value Measured by the Rotary Encoder based on the Main Microcontroller

//Set nRF24L01 Object and unique addresses for transmission and receival
RF24 radio(7, 8); //CE, CSN
const byte addresses[][6] = {"EDCBA", "ABCDE"};

void setup() 
{  
  // Set Rotary Encoder Pins as Inputs
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);

  // Setting Up the Interrupt for the Encoder
  // A rising pulse from encoder activates ai0(). AttachInterrupt 0 is DigitalPin 2.
  attachInterrupt(0, ai0, RISING);
  // B rising pulse from encoder activates ai1(). AttachInterrupt 1 is DigitalPin 3.
  attachInterrupt(1, ai1, RISING);
  
  // Set Unique Addresses for Transmission and Receival
  radio.begin();
  radio.openWritingPipe(addresses[0]); //EDCBA - Transmission
  radio.openReadingPipe(1, addresses[1]); //ABCDE - Receival
  radio.setPALevel(RF24_PA_MIN); // Set Minimum Frequency
  
  // Initialize Encoder Variables and Flag
  rotStep = 0;
  temp = 0;
  reset_bodyCirc = 0;
}

void loop() 
{
  delay(5);
  // Stop Receiving and Start Transmission
  radio.stopListening();
  //Only Start Reading from the Encoder if there is a change in the value of step
  if (rotStep != temp)
  {
    if (rotStep <= 0) //Prevent Negative Measurement
      rotStep = 0;
    bodyCircumference = 6.6 + (rotStep * (WHEEL_CIRC / ROTARY_STEPS));
    radio.write(&bodyCircumference, sizeof(bodyCircumference));
    delay(1);
    // Remember latest value of step
    temp = rotStep;
    // Put in a slight delay to help debounce the reading
    delay(1);
  }
  delay(5);
  // Start Checking if Flag is sent from the Main Microcontroller
  radio.startListening();
  if (radio.available())
    radio.read(&reset_bodyCirc, sizeof(reset_bodyCirc));
  if (reset_bodyCirc == 1) //Only Reset Values if 1 gets successfully transmitted to the flag
  {
    rotStep = 0;
    temp = 0;
    bodyCircumference = 0.00;
    reset_bodyCirc = 0; // Reset flag for the next reading
    delay(2500);
  }
}

// FUNCTION TO DETERMINE THE DIRECTION OF THE ROTATION OF THE ENCODER FROM PIN 3
void ai0() 
{
  // ai0 is activated if DigitalPin 2 (A) is going from LOW to HIGH
  // Check pin 3 (B) to determine the direction
  if(digitalRead(B)==LOW)
    rotStep++;
  else
    rotStep--;
}
  
// FUNCTION TO DETERMINE THE DIRECTION OF THE ROTATION OF THE ENCODER FROM PIN 2
void ai1()
{
  // ai0 is activated if DigitalPin 3 (B) is going from LOW to HIGH
  // Check with pin 2 (A) to determine the direction
  if(digitalRead(A)==LOW)
    rotStep--;
  else
    rotStep++;
}
