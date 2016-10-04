/* www.caletagreen.com
   Doors control with androide app and Photobean detector alarm:
   
   1. Open and close garage and peatonal doors with an Android App
   2. Sound alarm when photobean is broken and send an alert email
   3. Sound alarm can be started and stopped with an Android App

   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define alert on m2mlight.com and obtain api_key identifiers
   
*/

// Define what libraries to load
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <m2mData.h> 

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };   // arduino mac address
byte ip[] = { 192, 168, 1, 14};                        // arduino ip address
EthernetServer server(XXXX);                           // arduino server port

// General variables 
String readString;                  // variable to store requests
int alarm_signal;                   // variable to store photobean detector signal
boolean alarm_sound = false;
int alarm_counter = 0;
const int sound_number = 6;         // ring time = 9 seg (sound_number x 1.5 seg)
const int restart_number = 1200;    // restart time = 10 minutes (restart_number x 0.5 seg)  
boolean alarm_email = false;
m2mData alert;                  // create an instance for m2mData

void setup(){

   // start the ethernet connection and the server
   Ethernet.begin(mac, ip);
   server.begin();
  
   Serial.begin(9600); //start the serial monitor with 9600 baud
   pinMode(8, OUTPUT); //pin selected to control peatonal door
   pinMode(5, OUTPUT); //pin selected to control garage door
   pinMode(6, OUTPUT); //pin selected to control alarm signal
  
   Serial.println("Started");
}

// Define api_key variables obtained from m2mlight.com
String api_key_alert = String("113hmzYpSFq"); // api_key to send an alert email

// Create the web server listener and await for client connections:
void loop(){
   
   alarm_signal = analogRead(0);
   Serial.print("Alarm signal:"); Serial.println(alarm_signal);

   //If bean is broken
   if (alarm_signal < 100){ 
     alarm_sound = true;
   }
   // alarm sounds only for a moment, while alarm_counter is less than sound number, even if the signal continues
   // after a while (restar_number) the situation is reassessed

   Serial.print("alarm_sound:");   Serial.println(alarm_sound);

   if (alarm_sound == true) {
       alarm_counter = alarm_counter + 1;
       
       Serial.print("alarm_counter:");   Serial.println(alarm_counter);
        
       // sound for a while
       if (alarm_counter <= sound_number ){ // sound alarm
          if (alarm_counter == 1){ 
             alert.sendAlertEmail(api_key_alert, -1);
          }
          digitalWrite(6,HIGH); // start alarm sound
          delay(1000);
          digitalWrite(6,LOW); // stop alarm sound
       }
       if (alarm_counter > restart_number){ // reset alarm evaluation
          alarm_sound = false;
          alarm_counter = 0;  
       }
   }
  
   //Read request from Android App 
   EthernetClient client = server.available();// create a client connection
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

              client.println("HTTP/1.1 200 OK"); //send new page
              client.println("Content-Type: text/html");
              client.println();
              client.println(" ");
              client.println("OK");
              client.println(" "); // closes client sessions cleanly

              delay(1);
              // stopping client
              client.stop();
              client.flush();
              Serial.println("Stopped client");
          
              // Parse user http requests and see if they match what we are looking for
              
              // if user input is "GDOOR" then open/close garage door
              if(readString.indexOf("GDOOR") > 0){
                 Serial.println("GDOOR found");
                 digitalWrite(5,HIGH);
                 delay(1000);                
                 digitalWrite(5,LOW);
               }
              
              // if user input is "PDOOR" then open peatonal door
              if(readString.indexOf("PDOOR") > 0){
                 Serial.println("PDOOR found");
                   digitalWrite(8,HIGH);
                   delay(1000);                
                   digitalWrite(8,LOW);
              }
              
              // if user input is "START" then start alarm sound
              if(readString.indexOf("START") > 0){
                 Serial.println("START found");
                 alarm_sound = true;
                 alarm_counter = 0;
                 alarm_email = true;
              }
              
              // if user input is "STOP" then stop alarm sound
              if(readString.indexOf("STOP") > 0){
                 Serial.println("STOP found");
                 digitalWrite(6,LOW);
                 alarm_sound = true; // with the next instruction it is a trick to stop the alarm for a while
                 alarm_counter = sound_number;
              }
                     
               // clearing string for next read
              delay(100);
              readString="";
              Serial.println("Cleared readString");                    
              
            } // end if HTTP request is ended        
         } // end if client available
      } // end if client connected
      
   } // end if client
   
   if (alarm_email) {
      alert.sendAlertEmail(api_key_alert, -1);
      alarm_email = false;      
   }

   delay(1000);         

} // end loop

