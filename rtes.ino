#include <Wire.h>

// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication
const uint8_t scl = 5;
const uint8_t sda = 4;

// Acceleration scale as per the datasheet
const uint16_t accscale = 16384;

// MPU6050 configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV = 0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL = 0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1 = 0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2 = 0x6C;
const uint8_t MPU6050_REGISTER_CONFIG = 0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG = 0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG = 0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN = 0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE = 0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H = 0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET = 0x68;

int16_t AccelX, AccelY, AccelZ;
double saves[45] = {0};
double records[45];
int b = 0;
int c = 0;
int e = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin(sda, scl);
  MPU6050_Init();
}

void loop()
{
  double Ax, Ay, Az;
  int i, j, d = 0;
  if (c == 0)
  {
    Serial.println("What do you want to do? \n Record = 1 \n Detect = 2 \n Delete = 3");
    delay(3000);
    if (Serial.available() > 0)
    {
      c = Serial.read();
    }
  }
  //Prevent CR, LF or empty serial ports
  else if (c == 13 || c == 10 || c == -1)
  {
    c = Serial.read();
  }
  //Save a gesture
  else if (c == 49)
  {
    if (b == 10)
    {
      Serial.println("Gesture will be rewritten");
      b = 0;
    }
    if (e == 0)
    {
      Serial.println("Enter Gesture 10 times");
      e = 1;
    }

    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    Ax = (double)AccelX / accscale + 0.03;
    Ay = (double)AccelY / accscale - 0.00;
    Az = (double)AccelZ / accscale - 1.02;
    //Adjust the values by stabilizing the device, modify the values as per your calibration requirements. 
    //The values above were used by me, you can use the same or any other value as per your base state.
    // Ax = (double)AccelX / accscale;
    // Ay = (double)AccelY / accscale;
    // Az = (double)AccelZ / accscale;
    //Look for movement
    if (!mooment(Ax, Ay, Az))
    {
      delay(10);
      return;
    }
    Serial.print("Gesture no ");
    Serial.println(b);
    for (i = 0; i < 15; i++)
    {
      Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
      Ax = (double)AccelX / accscale + 0.03;
      Ay = (double)AccelY / accscale - 0.00;
      Az = (double)AccelZ / accscale - 1.02;
      saves[i * 3 + 0] = saves[i * 3 + 0] + Ax;
      saves[i * 3 + 1] = saves[i * 3 + 1] + Ay;
      saves[i * 3 + 2] = saves[i * 3 + 2] + Az;
      //      Serial.print(saves[i * 3 + 0]);
      //      Serial.print("\t");
      //      Serial.print(saves[i * 3 + 1]);
      //      Serial.print("\t");
      //      Serial.println(saves[i * 3 + 2]);
      delay(30);
    }
    b++;
    if (b == 10)
    {
      for (j = 0; j < 45; j++)
      {
        saves[j] = saves[j] / 10;
        //Serial.println(saves[j]);
      }
      c = 0;
      e = 0;
      Serial.println("Finished Recording");
    }
  }
  //Detect a gesture
  else if (c == 50)
  {
    if (b == 0)
    {
      Serial.println("No gesture saved");
      c = 0;
    }
    if (b == 10)
    {
      if (e == 0)
      {
        Serial.println("Enter recorded gesture");
        e = 1;
      }
      Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
      Ax = (double)AccelX / accscale + 0.03;
      Ay = (double)AccelY / accscale - 0.00;
      Az = (double)AccelZ / accscale - 1.02;
      if (!mooment(Ax, Ay, Az))
      {
        delay(10);
        return;
      }
      for (i = 0; i < 15; i++)
      {
        Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
        Ax = (double)AccelX / accscale + 0.03;
        Ay = (double)AccelY / accscale - 0.00;
        Az = (double)AccelZ / accscale - 1.02;
        records[i * 3 + 0] = Ax;
        records[i * 3 + 1] = Ay;
        records[i * 3 + 2] = Az;
        delay(30);
      }
      b = 8;
    }
    if (b == 8)
    {
      for (j = 0; j < 45; j++)
      {
        //        Serial.println("Recorded ");Serial.println(records[j]);
        //        Serial.println("Saved ");Serial.println(saves[j]);
        //        Serial.println("---------------");

        //        Tolerance values are -0.30 to +0.30
        if (!((records[j] <= saves[j] + 0.30) && (records[j] >= saves[j] - 0.30)))
        {
          d++;
        }
      }
      c = 0;
      b = 10;
      e = 0;
      if (d > 5)
      {
        Serial.println("Wrong gesture");
      }
      else
      {
        Serial.println("Correct gesture");
        //Put the resources or functions that you want to implement on successful gesture recognition
      }
    }
  }
  //Delete the gesture
  else if (c == 51)
  {
    for (i = 0; i < 45; i++)
    {
      saves[i] = 0;
      if (i == 44)
      {
        c = 0;
        b = 0;
      }
    }
    Serial.println("Gesture Removed");
  }
  //If unsupported values are passed to the serial
  else
  {
    Serial.println("Incorrect input");
    c = 0;
  }
  delay(10);
}


//Function to detect motion
bool mooment(double Ax, double Ay, double Az)
{
  if ((Ax + Ay + Az) > 0.01)
    return ((Ax + Ay + Az) > 0.1);
  else
    return ((Ax + Ay + Az) < (-0.1));
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read raw values from registers
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read() << 8) | Wire.read());
  AccelY = (((int16_t)Wire.read() << 8) | Wire.read());
  AccelZ = (((int16_t)Wire.read() << 8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init()
{
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);  //set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00); // set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
