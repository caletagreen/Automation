/* www.caletagreen.com
 
   Doors control with androide app and Photobean detector alarm:
   
   1. Open garage and pedestrian doors with the Android App
   2. Sound alarm when photobean is broken and send alert email
   3. Sound alarm can be started and stopped with the Android App

   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define actuators and alerts on m2mlight.com and obtain api_key identifiers
   
*/

// Define what libraries to load
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <m2mData.h> 

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };   // arduino mac address
byte ip[] = { 192, 168, 1, 14};                        // arduino ip address
EthernetServer server(9092);                           // arduino server port

// General variables 
String readString;                  // variable to store requests
int alarm_signal;                   // variable to store photobean detector signal
boolean alarm_photobean = false;
boolean alarm_sound = false;
int alarm_counter = 0;
const int sound_number = 6;         // ring time = 9 seg (sound_number x 1.5 seg)
m2mData actuator;                  // create an instance of actuator m2mData
m2mData alert;                     // create an instance of alert m2mData

// Define api_key variables obtained from m2mlight.com
char api_key_alarm_on[] = "SoD21njci3"; // actuator api_key entrance alarm on
char api_key_alarm_off[] = "7o5V1njci4"; // actuator api_key entrance alarm off
char api_key_garage_door[] = "q4KV1njchy"; // actuator api_key open garage door
char api_key_pedestrian_door[] = "4G7K1njci2"; // actuator api_key open pedestrian door
char api_key_photobean_detector[] = "kq9M1njci2"; // alert api_key photobean detector

void setup(){
   // start the ethernet connection and the server
   Ethernet.begin(mac, ip);
   server.begin();
   Serial.begin(9600); //start the serial monitor with 9600 baud
   pinMode(8, OUTPUT); //pin selected to control pedestrian door
   pinMode(5, OUTPUT); //pin selected to control garage door
   pinMode(6, OUTPUT); //pin selected to control alarm signal
}


// Create the web server listener and await for client connections:
void loop(){
   
   alarm_signal = analogRead(0);
   Serial.print("Alarm signal:"); Serial.println(alarm_signal);

   //If bean is broken
   if (alarm_signal < 100){ 
     alarm_sound = true;
     alarm_photobean = true;
   }
   
   // alarm sounds only for a moment, while alarm_counter is less than sound number
   if (alarm_sound == true) { //Start sound
       alarm_counter = alarm_counter + 1;
       Serial.print("alarm_counter:");   Serial.println(alarm_counter);        
       // sound for a while
       if (alarm_counter <= sound_number ){ // sound alarm
          if (alarm_counter == 1 and alarm_photobean == true){ // only one email
             alert.sendEmailAlert(api_key_photobean_detector);
             alarm_photobean = false;
          }
          digitalWrite(6,HIGH); // start alarm sound
          delay(1000);
          digitalWrite(6,LOW); // stop alarm sound
       } else {
          alarm_sound = false;
          alarm_counter = 0;   
       }
   }
  
   // Read request
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
              if(readString.indexOf(api_key_garage_door) > 0){
                 // if user input is garage door api_key then open/close garage door                
                 Serial.println(api_key_garage_door);
                 digitalWrite(5,HIGH);
                 delay(1000);                
                 digitalWrite(5,LOW);
              } else if(readString.indexOf(api_key_pedestrian_door) > 0){
                 // if user input is pedestrian api_key then open pedestrian door   
                 Serial.println(api_key_pedestrian_door);
                 digitalWrite(8,HIGH);
                 delay(1000);                
                 digitalWrite(8,LOW);
              } else if(readString.indexOf(api_key_alarm_on) > 0){
                 // if user input is entrance alarm ON  
                 Serial.println(api_key_alarm_on);
                 alarm_sound = true;
                 alarm_counter = 0;
              } else if(readString.indexOf(api_key_alarm_off) > 0){
                 // if user input is entrance alarm OFF
                 Serial.println(api_key_alarm_off);
                 digitalWrite(6,LOW);
                 alarm_sound = false;
              }
                     
               // clearing string for next read
              delay(100);
              readString="";
              Serial.println("Cleared readString");                    
              
            } // end if HTTP request is ended        
         } // end if client available
      } // end if client connected
      
   } // end if client

   delay(500);         

} // end loop

