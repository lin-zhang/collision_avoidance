// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.


#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 

const int analogInPin = A0;
 
int pos = 0;    // variable to store the servo position 
int sensorValue = 0;        // value read from the pot

void setup() 
{ 
  myservo.attach(8);  // attaches the servo on pin 9 to the servo object 
  Serial.begin(115200);
} 
 
 
void loop() 
{ /*
  for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(10);                       // waits 15ms for the servo to reach the position 
  } 
  for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(10);                       // waits 15ms for the servo to reach the position 
  } 
  */
  myservo.write(90);
  sensorValue = analogRead(analogInPin);

  Serial.print("sensor = " );
  Serial.println(sensorValue);


}

