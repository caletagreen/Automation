/* www.caletagreen.com
   Light control with android app

   Turn on/off lights with an Android App in a 3-way switch fashion.
   Use a timer and a light sensor (photoresistor)
   Timer to check the time and photoresistor to check if lights are on
   Lights can be turned on/off in the following ways:
   1. Manually with a 3-ways switch
   2. Manually with an Android App
   3. Automatically with defined times (ON and OFF) which can be set using an Android App

   Send an alert email when lights turn on or off automatically

   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define alerts on m2mlight.com and obtain api_key identifiers
  
*/


// Define what libraries to load
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <Wire.h>  // 
#include "RTClib.h" // 
#include <HttpClient.h>
#include <m2mData.h>

// RTC Timer
RTC_DS1307 rtc;
DateTime now;
m2mData sensor;

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   // arduino mac address
byte ip[] = { 192, 168, 1, 5 };                        // arduino ip address
EthernetServer server(8065);                           // arduino server port

// Variables
String readString;            // create readString class to read HTTP requests
int sensorReading = 0;        // variable to store read values of the photoresistor
const int lightcontrol = 950; // light intensity value defined by testing 
boolean done_on = false; 
boolean done_off = false; 

char api_key_on[] = "8adP1njchu"; // api_key to lights on, obtained from m2mlight.com
char api_key_off[] = "DmNs1njchv"; // api_key to lights off, obtained from m2mlight.com

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
               
              if (readString.indexOf("action=ON") > 0){// receive ON
                 Serial.println("ON found");
                 sensorReading = analogRead(A0); // read light sensor value
                 if (sensorReading <= lightcontrol){ // lights are OFF
                    digitalWrite(49, HIGH); // try to turn ON with HIGH
                    delay(300); // wait lights and light sensor
                    sensorReading = analogRead(A0); // read light sensor value
                    if (sensorReading < lightcontrol){ // lights are still OFF
                       digitalWrite(49, LOW); // turn on with LOW
                    }
                 }
              } else if (readString.indexOf("action=OFF") > 0){ //receive OFF
                 Serial.println("OFF found");
                 sensorReading = analogRead(A0); // read light sensor value
                 if (sensorReading >= lightcontrol){ // lights are ON 
                    digitalWrite(49, LOW); // try to turn OFF with LOW
                    delay(300); // wait lights and light sensor
                    sensorReading = analogRead(A0); // read light sensor value
                    if (sensorReading > lightcontrol){ // lights are still ON
                       digitalWrite(49, HIGH); // turn OFF with HIGH
                    }
                 }
               } else if (readString.indexOf("start_hour") >0 ) { //receive new start hour
                  Serial.println("start hour found");
                  save_settings();
              }

              // clearing string for next read
              delay(100);
              readString="";
              Serial.println("Cleared readString");                    
              
            } // end if HTTP request is ended        
         } // end if client available
      } // end if client connected
      
   } // end if client
   
   delay(2000);
         
} // end loop


// Read "readString" and store in EEPROM the start time and the end time for light control
// Data format: hihh:mm or hfhh:mm (hh: hours y mm: minutes)
// New data format: api_key=____&start_hour=MMSS for lights on and off

void save_settings(){
  int pos1, pos2;
  String h, m;
  int pos_api = readString.indexOf("api_key=");
  int pos_hou = readString.indexOf("&start_hour=");
  String api_key = readString.substring(pos_api+8, pos_hou);
   
  if (api_key == String(api_key_on)){ //Lights ON (start time)
    
      h = readString.substring(pos_hou+12,pos_hou+14);
      m = readString.substring(pos_hou+15,pos_hou+17);          
      Serial.print("\nSetting Time On=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(1, h.toInt()); // hours
      EEPROM.write(2, m.toInt()); // minutes
      
  }else if (api_key == String(api_key_off)){  //Lights OFF (end time)
    
      h = readString.substring(pos_hou+12,pos_hou+14);
      m = readString.substring(pos_hou+15,pos_hou+17);  
      Serial.print("\nSetting Time Off=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(3, h.toInt()); // hours
      EEPROM.write(4, m.toInt()); // minutes     
  }

  /*
  // Start time
  pos1 = readString.indexOf("hi");  // start position for "hi"
  pos2 = readString.indexOf(":");   // position for ":"
  
  if (pos1 > 0) {

      h = readString.substring(pos1+2,pos2);
      m = readString.substring(pos2+1,pos2+3);          
      Serial.print("\nSetting Time On=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(1, h.toInt()); // hours
      EEPROM.write(2, m.toInt()); // minutes
  }
  
  // End Time
  pos1 = readString.indexOf("hf"); // start position for "hf"
  pos2 = readString.indexOf(":");  // position for ":"  
  if (pos1 > 0) {
      h = readString.substring(pos1+2,pos2);
      m = readString.substring(pos2+1,pos2+3);
      Serial.print("\nSetting Time Off=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(3, h.toInt()); // hours
      EEPROM.write(4, m.toInt()); // minutes
  } 
*/
   
}

// Read from EEPROM the time settings and turn ON or OFF the lights
// Send an alert email when lights turn on or off automatically
// Use http function send_email_action to inform to server

void check_time(){
                   
    int hi = EEPROM.read(1); // start hours
    int mi = EEPROM.read(2); // star minutes
    int hf = EEPROM.read(3); // end hours
    int mf = EEPROM.read(4); // end minutes

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
     
     // Check time to turn on the lights
      if (current_hour == hi){
         if (current_minute == mi){
            if (sensorReading < lightcontrol){ // if lights are off
               if (done_on == false){ // does this only once
                  sensor.sendEmailAction(api_key_on);     // Send an email action                     
                  Serial.print("Turn on at this Time=");
                  Serial.print(hi);
                  Serial.print(":");
                  Serial.println(mi);
                  digitalWrite(49, HIGH); // try to turn on with HIGH 
                  delay(500); // wait lights and light sensor
                  sensorReading = analogRead(A0); // read ligtht sensor value
                  if (sensorReading < lightcontrol){ // if lights are still off                 
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
                  sensor.sendEmailAction(api_key_off);    // Send an email action                        
                  Serial.print("Turn off at this Time=");
                  Serial.print(hf);
                  Serial.print(":");
                  Serial.println(mf);
                  digitalWrite(49, LOW); //  try to turn off with LOW
                  delay(500); // wait lights and light sensor
                  sensorReading = analogRead(A0); // read light sensor value
                  if (sensorReading >= lightcontrol){ // lights are still on        
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


