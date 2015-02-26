#include <Wire.h>

const int ultrasonic_pingPin = 2;
const int bumperPin[8]={12,11,10,9,8,7,6,5};

long duration, inches, cm;
int bumper=0x00;

char str_out[128];

void i2c_write(int address, byte reg, byte data) {
  // Send output register address
  Wire.beginTransmission(address);
  Wire.write(reg);
  // Connect to device and send byte
  Wire.write(data); // low byte
  Wire.endTransmission();
}

void i2c_read(int address, byte reg, int count, byte* data) {
 int i = 0;

 // Send input register address
 Wire.beginTransmission(address);
 Wire.write(reg);
 Wire.endTransmission();
 // Connect to device and request bytes
 Wire.beginTransmission(address);
 Wire.requestFrom(address,count);
 while(Wire.available()) // slave may send less than requested 
 {
   char c = Wire.read(); // receive a byte as character
   data[i] = c;
   i++;
 }
 Wire.endTransmission();
}

#define  ADXL345_ADDRESS (0xA6 >> 1)
//There are 6 data registers, they are sequential starting 
//with the LSB of X.  We'll read all 6 in a burst and won't
//address them individually
#define ADXL345_REGISTER_XLSB (0x32)

 //Need to set power control bit to wake up the adxl345
 #define ADXL_REGISTER_PWRCTL (0x2D)
 #define ADXL_PWRCTL_MEASURE (1 << 3)
 
void init_adxl345() {
  byte data = 0;

  i2c_write(ADXL345_ADDRESS, ADXL_REGISTER_PWRCTL, ADXL_PWRCTL_MEASURE);

  //Check to see if it worked!
  i2c_read(ADXL345_ADDRESS, ADXL_REGISTER_PWRCTL, 1, &data);
  Serial.println((unsigned int)data);
}

int accelerometer_data[3];

void read_adxl345() {
 byte bytes[6];
 memset(bytes,0,6);

 //read 6 bytes from the ADXL345
 i2c_read(ADXL345_ADDRESS, ADXL345_REGISTER_XLSB, 6, bytes);

 //now unpack the bytes
 for (int i=0;i<3;++i) {
 accelerometer_data[i] = (int)bytes[2*i] + (((int)bytes[2*i + 1]) << 8);
 }
}

#define ITG3200_ADDRESS (0xD0 >> 1)
//request burst of 6 bytes from this address
#define ITG3200_REGISTER_XMSB (0x1D)
#define ITG3200_REGISTER_DLPF_FS (0x16)
#define ITG3200_FULLSCALE (0x03 << 3)
#define ITG3200_42HZ (0x03)

void init_itg3200() {
  byte data = 0;

  //Set DLPF to 42 Hz (change it if you want) and
  //set the scale to "Full Scale"
  i2c_write(ITG3200_ADDRESS, ITG3200_REGISTER_DLPF_FS, ITG3200_FULLSCALE | ITG3200_42HZ);

  //Sanity check! Make sure the register value is correct.
  i2c_read(ITG3200_ADDRESS, ITG3200_REGISTER_DLPF_FS, 1, &data);

  Serial.println((unsigned int)data);
}


int gyro_data[3];

void read_itg3200() {
  byte bytes[6];
  memset(bytes,0,6);

  //read 6 bytes from the ITG3200
  i2c_read(ITG3200_ADDRESS, ITG3200_REGISTER_XMSB, 6, bytes);  //now unpack the bytes
  for (int i=0;i<3;++i) {
  gyro_data[i] = (int)bytes[2*i + 1] + (((int)bytes[2*i]) << 8);
  }
}


#define HMC5843_ADDRESS (0x3C >> 1)
//First data address of 6 is XMSB.  Also need to set a configuration register for
//continuous measurement
 #define HMC5843_REGISTER_XMSB (0x03)
 #define HMC5843_REGISTER_MEASMODE (0x02)
 #define HMC5843_MEASMODE_CONT (0x00)
 
void init_hmc5843() {
  byte data = 0;
  //set up continuous measurement
  i2c_write(HMC5843_ADDRESS, HMC5843_REGISTER_MEASMODE, HMC5843_MEASMODE_CONT);

  //Sanity check, make sure the register value is correct.
  i2c_read(HMC5843_ADDRESS, HMC5843_REGISTER_MEASMODE, 1, &data);
  Serial.println((unsigned int)data);
}

int magnetometer_data[3];
void read_hmc5843() {
 byte bytes[6];
 memset(bytes,0,6);

  //read 6 bytes from the HMC5843
 i2c_read(HMC5843_ADDRESS, HMC5843_REGISTER_XMSB, 6, bytes);

  //now unpack the bytes
 for (int i=0;i<3;++i) {
 magnetometer_data[i] = (int)bytes[2*i + 1] + (((int)bytes[2*i]) << 8);
 }
}

long ultrasonic_distance_detect(const int pingPin){
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  return pulseIn(pingPin, HIGH);
}

int bumper_detect(const int pin[8]){
	int i=0;
	int result=0x00;
        int temp;
	for(i=0;i<8;i++){
		pinMode(pin[i], INPUT);
                temp=digitalRead(pin[i]);
		result=result<<1|temp;
	}
        //Serial.println("");
    return result;
}


void setup() {
  // initialize serial communication:
  Serial.begin(115200);
  Wire.begin();

  for(int i = 0; i < 3; ++i) {
    accelerometer_data[i] = magnetometer_data[i] = gyro_data[i] = 0;
  }

  init_adxl345();
  init_hmc5843();
  init_itg3200();
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
void loop()
{
  // convert the time into a distance
  duration=ultrasonic_distance_detect(ultrasonic_pingPin);
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
 
  bumper=bumper_detect(bumperPin);
  //Serial.println("begin:");
/*  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.print(" bumper: ");
  Serial.print(bumper, BIN);

   read_adxl345();
   Serial.print("\t  ACCEL ");
   Serial.print(accelerometer_data[0]);
   Serial.print("\t");
   Serial.print(accelerometer_data[1]);
   Serial.print("\t");
   Serial.print(accelerometer_data[2]);
//   Serial.print("\n");
   read_hmc5843();
   Serial.print("\t  MAG ");
   Serial.print(magnetometer_data[0]);
   Serial.print("\t");
   Serial.print(magnetometer_data[1]);
   Serial.print("\t");
   Serial.print(magnetometer_data[2]);
 //  Serial.print("\n");
   read_itg3200();
   Serial.print("\t  GYRO ");
   Serial.print(gyro_data[0]);
   Serial.print("\t");
   Serial.print(gyro_data[1]);
   Serial.print("\t");
   Serial.print(gyro_data[2]);
   //Serial.print("\n");
   //Sample at 10Hz
*/
  read_adxl345();
  read_hmc5843();
  read_itg3200();
  sprintf(str_out,"%6ld,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d,%6d",duration/74+1,0,0,bumper,accelerometer_data[0],accelerometer_data[1],accelerometer_data[2],gyro_data[0],gyro_data[1],gyro_data[2],magnetometer_data[0],magnetometer_data[1],magnetometer_data[2]); 
  Serial.println(str_out);
  delay(20);
}



