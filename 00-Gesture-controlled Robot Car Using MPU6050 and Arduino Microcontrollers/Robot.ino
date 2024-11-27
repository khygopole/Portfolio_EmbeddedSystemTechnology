// Gesture-Controlled Robot Car - Khyle Matthew Gopole
// Robot Program - Receiver

// Include Essential Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Declarations for Transceiver Module
RF24 radio(7, 8); // CE, CNS
const byte address[6] = "54321"; // Address for Communication

// DEFINE MOTOR PINS
// Motor 1
#define ENA 10
#define IN1 9
#define IN2 6
// Motor 2
#define ENB 5
#define IN3 4
#define IN4 3

// Declare Buzzer and Indicator Pins
const int HORN = 2;
const int LED_A = 14;
const int LED_B = 15;
const int LED_C = 16;

bool isHorn; // Variable to Change State

// Declare Speed Variables
int ctrSpeed;
const int SPEED[3] = {135, 195, 255}; // Low, Medium, High
const int TURN_SPEED[3] = {110, 159, 207}; // CHANGED BASE ON RATIO ~0.814

void setup()
{
  // Setup Motor Pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Setup Indicator Pins
  pinMode(HORN, OUTPUT);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);

  // Setup Receiver
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  // Set Initial Values and Statements
  isHorn = false;
  ctrSpeed = 0;
  noTone(HORN);
  Stop();
}

void loop()
{
  OperateRobot();
}

void OperateRobot()
{
  if (radio.available())
  {
    char control = '0';
    radio.read(&control, sizeof(control));

    switch (control)
    {
      case 'h':
        TriggerHorn();
        break;
      case 's':
        ChangeSpeed();
        break;
      case 'F':
        Forward();
        break;
      case 'B':
        Backward();
        break;
      case 'R':
        Right();
        break;
      case 'L':
        Left();
        break;
      default:
        Stop();
        break;
    }
  }
}

void TriggerHorn()
{
  isHorn = !isHorn;
  if (isHorn)
    tone(HORN, 440);
  else
    noTone(HORN);
}

void ChangeSpeed()
{
  Stop();
  Indicators();
  if (ctrSpeed == 2)
    ctrSpeed = 0;
  else
    ctrSpeed++;
  delay(1000);
  Indicators();
  delay(1000);
}

void Indicators()
{
  switch (ctrSpeed)
  {
    case 1:
      // MEDIUM SPEED
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, HIGH);
      digitalWrite(LED_C, LOW);
      break;
    case 2:
      // HIGH SPEED
      digitalWrite(LED_A, LOW);
      digitalWrite(LED_B, LOW);
      digitalWrite(LED_C, HIGH);
      break;
    default:
      // LOW SPEED
      digitalWrite(LED_A, HIGH);
      digitalWrite(LED_B, LOW);
      digitalWrite(LED_C, LOW);
      break;
  }
}

// MOTOR SETUP
void Forward()
{
  // Set Currently Activated Speed
  analogWrite(ENA, SPEED[ctrSpeed]);
  analogWrite(ENB, SPEED[ctrSpeed]);
  // Left wheel
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // Right wheel
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Set Indicators
  Indicators();
}

void Stop()
{
  // Left wheel
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  // Right wheel
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // Idle LEDs
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_C, HIGH);
}

void Left()
{
  // Set Currently Activated Speed
  analogWrite(ENA, TURN_SPEED[ctrSpeed]);
  analogWrite(ENB, SPEED[ctrSpeed]);
  // Left wheel
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  // Right wheel
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Set Indicators
  Indicators();
}

void Right()
{
  // Set Currently Activated Speed
  analogWrite(ENA, SPEED[ctrSpeed]);
  analogWrite(ENB, TURN_SPEED[ctrSpeed]);
  // Left wheel
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // Right wheel
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Set Indicators
  Indicators();
}

void Backward()
{
  // Set Currently Activated Speed
  analogWrite(ENA, SPEED[ctrSpeed]);
  analogWrite(ENB, SPEED[ctrSpeed]);
  // Left wheel
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  // Right wheel
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Set Indicators
  Indicators();
}

void StopAll()
{
  // Stop Robot Car
  // Left wheel
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  // Right wheel
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  // RESET EVERYTHING TO PREPARE IT FOR THE NEXT RUN
  // Turn Off All LEDs
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_C, LOW);
  // Reset Horn
  isHorn = false;
  noTone(HORN);
  // Reset Control Speed
  ctrSpeed = 0;
}
