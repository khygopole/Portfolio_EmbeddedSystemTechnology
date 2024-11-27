// Gesture-Controlled Robot Car - Khyle Matthew Gopole
// Controller Program - Transmitter

// Include Essential Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

// Declarations for Transceiver Module
RF24 radio(7, 8); // CE, CNS
const byte address[6] = "54321"; // Address for Communication

// Declarations for Buttons
const int PB_HORN = 5;
const int PB_SPEED = 6;
int curButtonHorn;
int prevButtonHorn;
int curButtonSpeed;
int prevButtonSpeed;

// Declarations for MPU6050 (Gyroscope and Accelerometer)
const int MPU_ADDRESS = 0x68; // Set MPU6050 I2C Address
float AccX, AccY, AccZ; // Acc Values of Axes
float GyroX, GyroY; // Gyro Values of Axes
float GyroZ; // Placeholder
float accAngleX, accAngleY, gyroAngleX, gyroAngleY; // Angle Values per Axis
float roll, pitch; // Main Rotational Angles
float elapTime, curTime, prevTime; // Time Variables for the Gyroscope
// Angle Thresholds for Controls
const int MIN_PITCH = -30; // BACKWARD THRESHOLD
const int MAX_PITCH = 30; // FORWARD THRESHOLD
const int MIN_ROLL = -30; // LEFT THRESHOLD
const int MAX_ROLL = 30; // RIGHT THRESHOLD

// Variables for MPU6050 Calibration
// float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
// int c = 0;

void setup()
{
  // Setup Buttons
  pinMode(PB_HORN, INPUT);
  pinMode(PB_SPEED, INPUT);

  // Setup MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU_ADDRESS); // Start Communication with MPU6050
  Wire.write(0x6B); // Communicate with Register 6B
  Wire.write(0x00); // Reset by placing 0 into 6B register
  Wire.endTransmission(true); // End Communication
  // Setup Transmitter
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  // Set Initial Values for Buttons
  curButtonHorn = 0;
  prevButtonHorn = 0;
  curButtonSpeed = 0;
  prevButtonSpeed = 0;

  // For Debugging
//   Serial.begin(9600);

  // For Sensitivity Configuration
  // ConfigSensorSens();

  // For Error Calculation
  // CalculateSensorError();
}

void loop()
{
  ReadButtons();
  ReadAccelerometer();
  ReadGyroscope();
  CalculateMainAngles();
  SetControl();
}

void ReadButtons()
{
  // Horn Feature
  curButtonHorn = digitalRead(PB_HORN);
  if (curButtonHorn != prevButtonHorn)
  {
    // Send Horn Signal to Activate/Deactivate
    char controlh = 'h';
    radio.write(&controlh, sizeof(controlh));
  }
  prevButtonHorn = curButtonHorn;

  // Speed Feature
  curButtonSpeed = digitalRead(PB_SPEED);
  if (curButtonSpeed != prevButtonSpeed)
  {
    if (curButtonSpeed == HIGH)
    {
      // Send Speed Signal to Transition Speed
      char controls = 's';
      radio.write(&controls, sizeof(controls));
    }
  }
  prevButtonSpeed = curButtonSpeed;
}

void ReadAccelerometer()
{
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x3B); // Communicate with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDRESS, 6, true); // Read 6 Registers since Each Axis Value is Stored in 2 Registers
  // Divide Raw Values by 16384 When Using Default Range (+/-2g) based on the Datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-Axis: Shift Read Value 8 bits to the Left then Divide
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-Axis: Shift Read Value 8 bits to the Left then Divide
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-Axis: Shift Read Value 8 bits to the Left then Divide
  // Calculate Roll and Pitch from the Data then Apply Error
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) + 0.91; // (-0.91): AccErrorX
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - 0.95; // (+0.95): AccErrorY
}

void ReadGyroscope()
{
  prevTime = curTime; // Set Previous Time to Current Time
  curTime = millis(); // Read Current Time
  elapTime = (curTime - prevTime) / 1000; // Get Elapsed Time then Convert ms into s
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x43); // Communicate with register 0x43 for Gyroscope
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDRESS, 6, true); // Read 6 Registers since Each Axis Value is Stored in 2 Registers
  // Divide Raw Values by 131 When Using Default Range (250deg/s) based on the Datasheet
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // X-Axis: Shift Read Value 8 bits to the Left then Divide
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0; // Y-Axis: Shift Read Value 8 bits to the Left then Divide
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0; // Z-Axis: Shift Read Value 8 bits to the Left then Divide - Placeholder
  // Correct Outputs Using Errors
  GyroX = GyroX + 0.40; // GyroErroX (-0.40)
  GyroY = GyroY + 0.29; // GyroErroY (-0.29)
  // GyroZ = GyroZ + 0.22; // GyroErroZ (-0.22) - Placeholder
  // Convert Raw Values of Gyroscope deg/s into deg by Multiplying Elapsed Time
  gyroAngleX = gyroAngleX + GyroX * elapTime;
  gyroAngleY = gyroAngleY + GyroY * elapTime;
}

void CalculateMainAngles()
{
  // yaw = yaw + GyroZ * elapTime;
  // Apply Complementary Filter to Base Angles from Gyroscope and Accelerometer
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  // Check Values on Serial Monitor
  // Serial.print("Roll: ");
  // Serial.println(roll);
  // Serial.print("Pitch: ");
  // Serial.println(pitch);
  // Serial.print("Yaw: ");
  // Serial.println(yaw);
}

void SetControl()
{
  if (pitch > MAX_PITCH)
  {
    // Forward
    char controlF = 'F';
    radio.write(&controlF, sizeof(controlF));
  }
  else if (pitch < MIN_PITCH)
  {
    // Backward
    char controlB = 'B';
    radio.write(&controlB, sizeof(controlB));
  }
  else if (roll > MAX_ROLL)
  {
    // Rightward
    char controlR = 'R';
    radio.write(&controlR, sizeof(controlR));
  }
  else if (roll < MIN_ROLL)
  {
    // Leftward
    char controlL = 'L';
    radio.write(&controlL, sizeof(controlL));
  }
  else
  {
    // Stop
    char controlS = 'S';
    radio.write(&controlS, sizeof(controlS));
  }
}

// void ConfigSensorSens()
// {
//   // Configure Accelerometer Sensitivity (Full Scale Range - Default +/- 2g)
//   Wire.beginTransmission(MPU_ADDRESS);
//   Wire.write(0x1C); // Communicate with ACCEL_CONFIG register (1C hex)
//   Wire.write(0x10); // Set the register bits as 00010000 (+/- 8g Full Scale Range)
//   Wire.endTransmission(true);

//   // Configure Gyroscope Sensitivity (Full Scale Range - Default +/- 250deg/s)
//   Wire.beginTransmission(MPU_ADDRESS);
//   Wire.write(0x1B); // Communicate with GYRO_CONFIG register (1B hex)
//   Wire.write(0x10); // Set the register bits as 00010000 (1000deg/s Full Scale Range)
//   Wire.endTransmission(true);
//   delay(20);
// }

// void CalculateSensorError()
// {
//   // Call this funtion in the setup section to calculate the accelerometer and gyro data error.
//   // From here we will get the error values used in the above equations printed on the Serial Monitor.
//   // Note that we should place the Sensor flat in order to get the proper values, so that we can correct values
//   // Read accelerometer values 200 times
//   while (c < 200)
//   {
//     Wire.beginTransmission(MPU_ADDRESS);
//     Wire.write(0x3B);
//     Wire.endTransmission(false);
//     Wire.requestFrom(MPU_ADDRESS, 6, true);
//     AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
//     AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
//     AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
//     // Sum all readings
//     AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
//     AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
//     c++;
//   }
//   //Divide the sum by 200 to get the error value
//   AccErrorX = AccErrorX / 200;
//   AccErrorY = AccErrorY / 200;
//   c = 0;
//   // Read gyro values 200 times
//   while (c < 200)
//   {
//     Wire.beginTransmission(MPU_ADDRESS);
//     Wire.write(0x43);
//     Wire.endTransmission(false);
//     Wire.requestFrom(MPU_ADDRESS, 6, true);
//     GyroX = Wire.read() << 8 | Wire.read();
//     GyroY = Wire.read() << 8 | Wire.read();
//     GyroZ = Wire.read() << 8 | Wire.read();
//     // Sum all readings
//     GyroErrorX = GyroErrorX + (GyroX / 131.0);
//     GyroErrorY = GyroErrorY + (GyroY / 131.0);
//     GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
//     c++;
//   }
//   //Divide the sum by 200 to get the error value
//   GyroErrorX = GyroErrorX / 200;
//   GyroErrorY = GyroErrorY / 200;
//   GyroErrorZ = GyroErrorZ / 200;
//   // Print the error values on the Serial Monitor
//   Serial.print("AccErrorX: ");
//   Serial.println(AccErrorX);
//   Serial.print("AccErrorY: ");
//   Serial.println(AccErrorY);
//   Serial.print("GyroErrorX: ");
//   Serial.println(GyroErrorX);
//   Serial.print("GyroErrorY: ");
//   Serial.println(GyroErrorY);
//   Serial.print("GyroErrorZ: ");
//   Serial.println(GyroErrorZ);
// }
