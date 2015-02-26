//Arduino 1.0+ only
//
#include <Wire.h>
#include <Servo.h>
int AXDL345_Address = 0x53; //I2C address of the AXDL345

int acc_x;
int acc_y;
int acc_z;
char str[512]; 
char val;
String pwm[2]="";
int num_pwm[2];
String inString = "";

Servo myservo0, myservo1;

void writeRegister(int deviceAddress, byte address, byte val) {
	Wire.beginTransmission(deviceAddress); // start transmission to device 
	Wire.write(address); // send register address
	Wire.write(val); // send value to write
	Wire.endTransmission(); // end transmission
}

String getValue(String data, char separator, int index)
{	
	int found = 0;
  	int strIndex[] = {0, -1};
  	int maxIndex = data.length()-1;

  	for(int i=0; i<=maxIndex && found<=index; i++){
    		if(data.charAt(i)==separator || i==maxIndex){
    	    		found++;
    	    		strIndex[0] = strIndex[1]+1;
    	    		strIndex[1] = (i == maxIndex) ? i+1 : i;
    		}
	}

  	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
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
	//TCCR1A=0x00;
	//TCCR1B=0x12;
	//ICR1=0x1F40;
	//TCCR2B = TCCR2B & 0b11111000 | 0x02;
	//setPwmFrequency(9, 8);
	//setPwmFrequency(10, 8);
	myservo0.attach(9);
	myservo1.attach(10);
	Wire.begin();
	Serial.begin(9600);
	Serial.println("starting up setupADXL345");
	setupADXL345();
	Serial.println("finished setupADXL345");
	Serial.println("[I] IDLE");
	delay(1500); //wait for the sensor to be ready 
}

void loop()
{
	inString="";

	while(Serial.available()>0){
		val = Serial.read();
		inString += (char)val;
	}
	
	//String token=inString.substring(0,3);
	//Serial.println(token);
	/*if(token=="[R]"){
		getAccValues();
		delay(100);
		sprintf(str,"%4d,%4d,%4d",acc_x,acc_y,acc_z);
	}*/
	
	//if(token=="[C]"){
                getAccValues();
		delay(25);
		pwm[0]=getValue(inString, ',' , 1);
		pwm[1]=getValue(inString, ',' , 2);
		num_pwm[0]=pwm[0].toInt();
		num_pwm[1]=pwm[1].toInt();
		myservo0.write(num_pwm[0]); 
		myservo1.write(num_pwm[1]); 
		sprintf(str, "[R],%4d,%4d,%4d,[M],%d,%d",acc_x,acc_y,acc_z,num_pwm[0],num_pwm[1]);
		Serial.println(str);
	//}//

	delay(20);
}
