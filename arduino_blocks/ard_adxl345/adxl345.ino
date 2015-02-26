//Arduino 1.0+ only
//
#include <Wire.h>

#define GYRO_CTRL_REG1 0x20
#define GYRO_CTRL_REG2 0x21
#define GYRO_CTRL_REG3 0x22
#define GYRO_CTRL_REG4 0x23
#define GYRO_CTRL_REG5 0x24

int ITG3200_Address = 0x68; //I2C address of the ITG3200
int AXDL345_Address = 0x53; //I2C address of the AXDL345

int gyro_x;
int gyro_y;
int gyro_z;
int acc_x;
int acc_y;
int acc_z;
char str1[512]; 
char str2[512]; 
char val;

void writeRegister(int deviceAddress, byte address, byte val) {
Wire.beginTransmission(deviceAddress); // start transmission to device 
Wire.write(address); // send register address
Wire.write(val); // send value to write
Wire.endTransmission(); // end transmission
}

int readRegister(int deviceAddress, byte address){

int v;
Wire.beginTransmission(deviceAddress);
Wire.write(address); // register to read
Wire.endTransmission();
Wire.beginTransmission(deviceAddress);
Wire.requestFrom(deviceAddress, 1); // read a byte

while(!Wire.available()) {
// waiting
}

v = Wire.read();
Wire.endTransmission(); //end transmission 
return v;
}

void getGyroValues()
{
byte xMSB = readRegister(ITG3200_Address, 0x29);
byte xLSB = readRegister(ITG3200_Address, 0x28);
gyro_x = ((xMSB << 8) | xLSB);
byte yMSB = readRegister(ITG3200_Address, 0x2B);
byte yLSB = readRegister(ITG3200_Address, 0x2A);
gyro_y = ((yMSB << 8) | yLSB);
byte zMSB = readRegister(ITG3200_Address, 0x2D);
byte zLSB = readRegister(ITG3200_Address, 0x2C);
gyro_z = ((zMSB << 8) | zLSB);
}
void getAccValues()
{
byte xMSB = readRegister(AXDL345_Address, 0x33);
byte xLSB = readRegister(AXDL345_Address, 0x32);
acc_x = ((xMSB << 8) | xLSB);
byte yMSB = readRegister(AXDL345_Address, 0x35);
byte yLSB = readRegister(AXDL345_Address, 0x34);
acc_y = ((yMSB << 8) | yLSB);
byte zMSB = readRegister(AXDL345_Address, 0x37);
byte zLSB = readRegister(AXDL345_Address, 0x36);
acc_z = ((zMSB << 8) | zLSB);
}

void setupADXL345()
{
writeRegister(AXDL345_Address,0x31,0x00);
writeRegister(AXDL345_Address,0x2c,0x08);
writeRegister(AXDL345_Address,0x2D,0x08);
writeRegister(AXDL345_Address,0x2E,0x00);
writeRegister(AXDL345_Address,0x1E,0x00);
writeRegister(AXDL345_Address,0x1F,0x00);
writeRegister(AXDL345_Address,0x20,0x00);

}

void setup()
{
	Wire.begin();
	Serial.begin(115200);
	Serial.println("starting up setupADXL345");
	setupADXL345();
	Serial.println("finished setupADXL345");
	delay(1500); //wait for the sensor to be ready 
}

void loop()
{
getAccValues();
delay(100);
sprintf(str2,"%4d,%4d,%4d",acc_x,acc_y,acc_z);
Serial.println(str2);
}
