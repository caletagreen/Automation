/* www.caletagreen.com
   Light control with android app

   Turn on/off lights with an Android App in a 3-way switch fashion.
   Use a timer and a light sensor (photoresistor)
   Timer to check the time and photoresistor to check if lights are on
   Lights can be turned on/off in the following ways:
   1. Manually with a 3-ways switch
   2. Manually with an Android App
   3. Automatically with defined parameters (hours and minutes) to turn on/off the lights. These parameters are
      stored in m2mlight.com

   Automatically send an alert email when lights turn on/off automatically
    
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define parameters and alerts on m2mlight.com and obtain api_key identifiers
  
*/

// Define what libraries to load
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <Wire.h>   
#include "RTClib.h"  
#include <HttpClient.h>
#include <m2mData.h>

// RTC Timer
RTC_DS1307 rtc;
DateTime now;
m2mData sensor;
m2mData alert;

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   // arduino mac address
byte ip[] = { 192, 168, 1, 5 };                        // arduino ip address
EthernetServer server(XXXX);                           // arduino server port

// Variables
String readString;            // create readString class to read HTTP requests
int sensorReading = 0;        // variable to store read values of the photoresistor
const int lightcontrol = 950; // light intensity value defined by testing 
boolean done_on = false; 
boolean done_off = false; 
float hi, nhi; //initial hour 
float mi, nmi; //initial minutes
float hf, nhf; //final hour
float mf, nmf; //final minutes

// Define api_key variables obtained from m2mlight.com
String api_key_on = String("1180SgN4pHk"); // api_key to send alert email lights on
String api_key_off = String("139uhLDYA2Z"); // api_key to send alert email lights off
String api_key_hi = String("351Gx5cIHeO"); // initial hours api_key
String api_key_mi = String("223knMeftQN"); // initial minutes api_key
String api_key_hf = String("344E8osODNw"); // final hours api_key
String api_key_mf = String("564qE7Yx0Sm"); // final minutes api_key


void setup(){

  // start the ethernet connection and the local server
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.begin(9600);  // enable serial data print  
  pinMode(49, OUTPUT); // pin selected to control
  
  Wire.begin(); // set I2C bus
  rtc.begin();  // set RTC
  
  Serial.println("Started");

}


void loop(){

   sensorReading = analogRead(A0); // read light sensor value
   Serial.println(sensorReading);

   local_time();  // show local time
   
   //Check time settings in order to turn ON/OFF the lights
    check_time();
   
   // Read request from Android App
   EthernetClient client = server.available(); // create a client connection  
   if (client) {
      Serial.println("Client connected");
      while (client.connected()) {
         if (client.available()) {
            char c = client.read();
            // read char by char HTTP request
            if (readString.length() < 100) {
               // store characters to string
               readString += c;
            }
            // if HTTP request has ended
            if (c == '\n') {
              Serial.println("HTTP request ended");
              Serial.print("readString = ");
              Serial.println(readString); // print to serial monitor for debugging

              // now output HTML data header
              client.println("HTTP/1.1 200 OK"); //send new page
              client.println("Content-Type: text/html");
              client.println();
              client.println(" ");
              client.println("OK");
              client.println(" ");// close client sessions cleanly

              delay(1);
              // stopping client
              client.stop();
              Serial.println("Stopped client");
          
              // parse user http requests and see if they match what we are looking for
              
              // if user input is ON then lights are turned on or off 
              if(readString.indexOf("ON") > 0){//checks for ON
                 Serial.println("ON found");
                 sensorReading = analogRead(A0); // read light sensor value
                 if (sensorReading >= lightcontrol){ // lights are on
                    digitalWrite(49, HIGH); // commute
                    delay(500); // wait lights and light sensor
                    sensorReading = analogRead(A0); // read light sensor value
                    if (sensorReading > lightcontrol){ // lights are still lit
                       digitalWrite(49, LOW); // commute
                    }
                 }
                 else{ // lights are off
                    digitalWrite(49, HIGH); // commute
                    delay(500); // wait lights and light sensor
                    sensorReading = analogRead(A0); // read light sensor value
                    if (sensorReading < lightcontrol){ // lights are still off
                       digitalWrite(49, LOW); // commute
                    }
                 }
              }
              
              // clearing string for next read
              delay(100);
              readString="";
              Serial.println("Cleared readString");                    
              
            } // end if HTTP request is ended        
         } // end if client available
      } // end if client connected
      
   } // end if client
   
   delay(3000);
         
} // end loop


// Read from m2mlight.com the time settings and turn ON/OFF the lights
// Send an alert email when lights turn on/off automatically


void check_time(){

    // Read parameters values from m2mlight
    nhi = sensor.readParameter(api_key_hi);
    nmi = sensor.readParameter(api_key_mi);
    nhf = sensor.readParameter(api_key_hf);
    nmf = sensor.readParameter(api_key_mf);

    // Check if is reading values from m2mlight
    if (nhi>-1)
       hi = nhi;
    if (nmi>-1)
       mi = nmi;
    if (nhf>-1)
       hf = nhf;
    if (nmf>-1)
       mf = nmf;   
          
    Serial.print("\nTurns on Time setting=");
    Serial.print(hi); Serial.print(":"); Serial.print(mi);

    Serial.print("\nTurns off Time setting=");
    Serial.print(hf); Serial.print(":");  Serial.println(mf);

    sensorReading = analogRead(A0); // read light sensor value
    int current_hour = now.hour();
    int current_minute = now.minute();

    Serial.print(" Current_hour="); Serial.println(current_hour);
    Serial.print(" Current_minute="); Serial.println(current_minute);
    Serial.print(" sensorReading="); Serial.println(sensorReading);

    if (hi>-1 and mi>-1 and hf>-1 and mf>-1){

       Serial.print("Checking time...");
       // Check time to turn on the lights
       if (current_hour == hi){
         if (current_minute == mi){
            if (sensorReading < lightcontrol){ // if lights are off
               if (done_on == false){ // does this only once
                  alert.sendAlertEmail(api_key_on);     // Send an alert email                     
                  Serial.print("Turn on at this Time=");
                  Serial.print(hi);
                  Serial.print(":");
                  Serial.println(mi);
                  digitalWrite(49, HIGH); // try to turn on with HIGH 
                  delay(500); // wait lights and light sensor
                  sensorReading = analogRead(A0); // read ligtht sensor value
                  if (sensorReading < lightcontrol){ // if lights are still off
                     alert.sendAlertEmail(api_key_on);                          
                     digitalWrite(49, LOW); // turn on with LOW
                  }
                  done_on = true; // avoid to do this again
               }
            }  
         }
         else{
            done_on = false; // enable to turn on in the future
         }   
      }
      // Check time to turn off the lights 
      if (current_hour == hf){
         if (current_minute == mf) {
            if (sensorReading >= lightcontrol){ // if lights are on
               if (done_off == false){ // does this only once 
                  alert.sendAlertEmail(api_key_off);    // Send an alert email                        
                  Serial.print("Turn off at this Time=");
                  Serial.print(hf);
                  Serial.print(":");
                  Serial.println(mf);
                  digitalWrite(49, LOW); //  try to turn off with LOW
                  delay(500); // wait lights and light sensor
                  sensorReading = analogRead(A0); // read light sensor value
                  if (sensorReading >= lightcontrol){ // lights are still on
                     alert.sendAlertEmail(api_key_off);                       
                     digitalWrite(49, HIGH); // turn off with HIGH
                  }
                  done_off = true; // avoid to do this again
               }
            } 
         }
         else{
            done_off = false; // enable to turn on in the future
         }
      }
    }   
}

// Show local time
void local_time(){
   now = rtc.now(); // Get date and time from RTC     
   Serial.println();
   Serial.print(now.year(), DEC);
   Serial.print('/');
   Serial.print(now.month(), DEC);
   Serial.print('/');
   Serial.print(now.day(), DEC);
   Serial.print(' ');
   Serial.print(now.hour(), DEC);
   Serial.print(':');
   Serial.print(now.minute(), DEC);
   Serial.print(':');
   Serial.print(now.second(), DEC);
   Serial.println();            
}

