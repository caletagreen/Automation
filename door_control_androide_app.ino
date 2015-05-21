/* www.caletagreen.com
   Door control with androide app and Photobean detector alarm:
   
   1. Open and close garage and peatonal doors with an Android App
   2. Sound alarm when photobean is broken and send an alert email
   3. Sound alarm can be started and stopped with an Android App

*/

// Define what libraries to load
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };   // arduino mac address
byte ip[] = { 192, 168, 1, 14};                        // arduino ip address
EthernetServer server(8092);                           // arduino server port 8092
byte serverName[] = { 88, 198, 207, 77};               // cloud server ip address

// General variables 
String readString;                  // variable to store requests
int alarm_signal;                   // variable to store photobean detector signal
boolean alarm_sound = false;
int alarm_counter = 0;
const int sound_number = 6;         // ring time = 9 seg (sound_number x 1.5 seg)
const int restart_number = 1200;    // restart time = 10 minutes (restart_number x 0.5 seg)  
boolean alarm_email = false;

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
             send_alarm_email(1);  // send an alarm email with id_alarm=1                      
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
              
              // if user input is "s" then open/close garage door
              if(readString.indexOf("s") > 0){
                 Serial.println("s found");
                 digitalWrite(5,HIGH);
                 delay(1000);                
                 digitalWrite(5,LOW);
               }
              
              // if user input is "t" then open peatonal door
              if(readString.indexOf("t") > 0){
                 Serial.println("t found");
                   digitalWrite(8,HIGH);
                   delay(1000);                
                   digitalWrite(8,LOW);
              }
              
              // if user input is "n" then start alarm sound
              if(readString.indexOf("n") > 0){
                 Serial.println("n found");
                 alarm_sound = true;
      		 alarm_counter = 0;
                 alarm_email = true;
              }
              
              // if user input is "f" then stop alarm sound
              if(readString.indexOf("f") > 0){
                 Serial.println("f found");
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
      send_alarm_email(1);  // send an alarm email with id_alarm=1   
      alarm_email = false;      
   }

   delay(500);         

} // end loop

// Send an alert email identified with "id_alarm"
void send_alarm_email(int id_alarm){

  EthernetClient client;                      
  Serial.println("Connecting...");
  if (client.connect(serverName, 80)) {                  // connect to cloud server 
    client.print("GET /alert/send_email.php?id_alarm="); // send id_alarm using GET
    client.print(id_alarm);    
    client.println(" HTTP/1.0");
    client.println("User-Agent: Arduino 1.0");
    client.println();
    Serial.println("Connected");
  } else {
    Serial.println("Not connected");
  }
   Serial.println("Disconnecting...");
   while(client.connected()) //Waiting until connection finish
   {
     if(client.available()) Serial.write(client.read());
   }
   client.stop(); 
}

