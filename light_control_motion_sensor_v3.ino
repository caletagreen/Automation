/* wwww.caletagreen.com
   Light control with motion sensor:

   1. Turn on/off lights with a motion sensor and in a 3-way switch fashion.
      Lights are turn on always at night when motion is detected 
      Use a light sensor (LDR) to check if lights are on and to check if it is night
   2. Upload light sensor value to m2mlight plattform 

   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensors on m2mlight.com and obtain api_key identifiers

*/

/* Define what libraries to load */
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Wire.h>
#include <m2mData.h>

/* Configure the Ethernet Shield */
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xA4, 0x61 };   // arduino mac address
byte ip[] = { 192, 168, 1, 16 };                       // arduino ip address
EthernetServer server(9094);                           // arduino server port

/* Motion sensor and LDR variables */ 
int LDR = 0;                        // analog pin to Light Sensor (LDR) is connected
int LDRValue = 0;                   // variable to store LDR values
const int light_sensitivity = 650;  // median value of light surrounding the LDR
int inMotion = 0;                   // variable to store read value of motion sensor (1:turn on the lights; 0:turn off the lights) 
int inRelay = 0;                    // variable to store read value of input relay
const int calibrationTime=10;       // time in seconds to start the LDR
int lightIsOn = LOW;                // variable to know if lights are on (HIGHT) or off (LOW)
m2mData sensor;                  // create an instance for m2mData

char api_key_light[] = "LP2o1njciw"; // api_key to light intensity sensor, obtained from m2mlight.com
long prevMillis = 0;  // to send light intensity value
long interval_light = 15000; // every 15 sec. 


void setup()
{  
    // start the ethernet connection and the server
    Ethernet.begin(mac, ip);
    server.begin();

    Serial.begin(9600);    // start the serial monitor with 9600 baud
    pinMode(2, OUTPUT);    // pin selected for the relay
    pinMode(8, INPUT);     // pin selected for the motion sensor

    Serial.print("Calibrating sensor...");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
    }
    Serial.println("Done");
    delay(50);

}
   
void loop(){
        
    LDRValue = analogRead(LDR);         // reads the ldr’s value
   unsigned long currMillis = millis();
   if(currMillis - prevMillis > interval_light) { //every 15 sec.
     prevMillis = currMillis;   
     float light_measure = LDRValue;
     Serial.print(" light_measure="); Serial.println(light_measure);     
     sensor.sendValue(api_key_light, light_measure); // store value on m2mlight
   }
    
    inMotion = digitalRead(8);
    inRelay = digitalRead(2);
   
    // if motion is detected and lights are off 
    if (inMotion == HIGH && LDRValue < light_sensitivity)
    {
       Serial.println("Motion detected at night");
       // We change the state in the relay
       if (inRelay == LOW) { 
          digitalWrite(2, HIGH);
       } 
       else
       {
          digitalWrite(2, LOW);
       }
       lightIsOn = HIGH;
    }
    else
    {  // if motion is not detected and (lights are on or it is not night)
       if (inMotion == LOW && LDRValue > light_sensitivity) 
       {
         Serial.println("There is no motion or there is clarity");
          if (lightIsOn == HIGH) { // prevent to turn on/off in the day 
             if (inRelay == LOW) { 
                digitalWrite(2, HIGH);
             } 
             else
             {
                digitalWrite(2, LOW);
             }
             lightIsOn = LOW;
          }             
       }  
    }  

   delay(500);
       
} //end loop


