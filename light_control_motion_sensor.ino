/* wwww.caletagreen.com
   Light control with motion sensor and holidays alarm:

   1. Turn on/off lights with a motion sensor and in a 3-way switch fashion.
      Lights are turn on always at night when motion is detected 
      Use a light sensor (LDR) to check if lights are on and to check if it is night
   2. On holidays, an alarm sounds and an email is sent when motion is detected
      Holidays dates (Start and end) are setting using a an Android App and stored in the Arduino EEPROM
   3. Sound alarm can be started and stopped with an Android App
   
*/

/* Define what libraries to load */
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Time.h>

/* Configure the Ethernet Shield */
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xA4, 0x61 };   // arduino mac address
byte ip[] = { 192, 168, 1, 16 };                       // arduino ip address
EthernetServer server(8094);                           // arduino server port
byte serverName[] = { 88, 198, 207, 77};               // cloud server ip address

/* Motion sensor and LDR variables */ 
int LDR = 0;                        // analog pin to Light Sensor (LDR) is connected
int LDRValue = 0;                   // variable to store LDR values
const int light_sensitivity = 650;  // median value of light surrounding the LDR
int inMotion = 0;                   // variable to store read value of motion sensor (1:turn on the lights; 0:turn off the lights) 
int inRelay = 0;                    // variable to store read value of input relay
const int calibrationTime=10;       // time in seconds to start the LDR
int lightIsOn = LOW;                // variable to know if lights are on (HIGHT) or off (LOW)

/* Holidays alarm variables */
boolean inVacations = false;
boolean alarm_sound = false;
int alarm_counter = 0;
const int sound_number = 6;         // ring time = 9 seg (sound_number x 1.5 seg)
const int restart_number = 1200;    // restart time = 10 minutes (restart_number x 0.5 seg)  
String readString;                  // variable to store requests
boolean alarm_email = false;
     
/* NTP Server Settings */
IPAddress timeServer(88, 198, 207, 77); // time server address
const long timeZoneOffset = -18000L;    // GMT -5 (in seconds)
unsigned int ntpSyncTime = 3600;        // syncs to NTP server every 1 hour     
// local port to listen for UDP packets
unsigned int localPort = 8888;
// NTP time stamp is in the first 48 bytes of the message
const int NTP_PACKET_SIZE= 48;      
// buffer to hold incoming and outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];  
// a UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;                    
// keeps track of how long ago we updated the NTP server
unsigned long ntpLastUpdate = 0;    

void setup()
{  
    // start the ethernet connection and the server
    Ethernet.begin(mac, ip);
    server.begin();

    Serial.begin(9600);    // start the serial monitor with 9600 baud
    pinMode(2, OUTPUT);    // pin selected for the relay
    pinMode(8, INPUT);     // pin selected for the motion sensor
    pinMode(6, OUTPUT);    // pin selected for the alarm

    Serial.print("Calibrating sensor...");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
    }
    Serial.println("Done");
    delay(50);
 
    //Try to get the date and time
    int trys=0;
    while(!getTimeAndDate() && trys<10) {
       trys++;
    }
}
   
void loop(){
 
    // Update the time via NTP server every 1 hour
    if(now()-ntpLastUpdate > ntpSyncTime) {
       int trys=0;
       while(!getTimeAndDate() && trys<10){
         trys++;
       }
       if(trys<10){
         Serial.println("ntp server update success");
       }
       else{
         Serial.println("ntp server update failed");
       }
    }
//    clockDisplay();  
         
    LDRValue = analogRead(LDR);         // reads the ldr’s value
    inMotion = digitalRead(8);
    inRelay = digitalRead(2);

    Serial.print("LDR="); Serial.println(LDRValue);          
    Serial.print("inMotion="); Serial.println(inMotion);
    Serial.print("inRelay="); Serial.println(inRelay);
    Serial.print("lightIsOn="); Serial.println(lightIsOn);
   
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

    inVacations= false;
    check_date();
    // if motion is detected and it is holidays
    if (inMotion == HIGH && inVacations == true){ 
        alarm_sound = true;
    }
    
    // alarm sounds only for a moment, while alarm_counter is less than sound number, even if the movement continues
    // after a while (restar_number) the situation is reassessed
    if (alarm_sound == true) {
       alarm_counter = alarm_counter + 1;
       // sound for a while
       if (alarm_counter <= sound_number ){ // sound alarm
          if (alarm_counter == 1){ 
             send_alarm_email(2);  // send an alarm email with id_alarm=2                       
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
              client.println(" ");//Closes client sessions cleanly

              delay(1);
              // stopping client
              client.stop();
              Serial.println("Stopped client");
          
              // parse user http requests and see if they match what we are looking for:
                          
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
              // if user input is hi or hf then store holidays dates
              if(readString.indexOf("di") >0 or readString.indexOf("df") >0) {
                Serial.println("di or df found");
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

   if (alarm_email) {
      send_alarm_email(2);  // send an alarm email with id_alarm=2   
      alarm_email = false;      
   }
   
   delay(500);
       
} //end loop


// send an alert email identified with "id_alarm"
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

// Read "readString" and store in EEPROM, the start date and the end date for holidays
// Data format: didd:mm or dfdd:mm (dd: day y mm: month)

void save_settings(){
  int pos1, pos2;
  String h, m;
 
  // Start date
  pos1 = readString.indexOf("di");  // start position for "di"
  pos2 = readString.indexOf(":");  // position for ":"
  if (pos1 > 0) {
      h = readString.substring(pos1+2,pos2);
      m = readString.substring(pos2+1,pos2+3);          
      Serial.print("\nSetting Date On=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(1, h.toInt()); // start day
      EEPROM.write(2, m.toInt()); // start month
  }

  // End date
  pos1 = readString.indexOf("df");  // start position for "df"
  pos2 = readString.indexOf(":");   // position for ":"
  if (pos1 > 0) {
      h = readString.substring(pos1+2,pos2);
      m = readString.substring(pos2+1,pos2+3);
      Serial.print("\nSetting date Off=");
      Serial.print(h.toInt(),DEC);
      Serial.print(":");
      Serial.print(m.toInt(),DEC);
      Serial.println();
      EEPROM.write(3, h.toInt()); // end day
      EEPROM.write(4, m.toInt()); // end month
  } 
  
}

// Read from EEPROM the settings holidays days 
// change "inVacations" variable to true if current day is between start date of holidays and end date of holidays

void check_date(){

      int hi = EEPROM.read(1); //start day
      int mi = EEPROM.read(2); //start month
      int hf = EEPROM.read(3); //end day
      int mf = EEPROM.read(4); //end month 

      Serial.print("\Start day and month settings=");
      Serial.print(hi); Serial.print(":"); Serial.println(mi);

      Serial.print("\nEnd day and mont settings=");
      Serial.print(hf); Serial.print(":"); Serial.println(mf);

      int current_day = day();
      int current_month = month();

      Serial.print("current_day"); Serial.println(current_day);
      Serial.print("current_month"); Serial.println(current_month);
      
      if (current_month >= mi && current_month <= mf){  // holidays month
         if (current_day >= hi && current_day <= hf){ // holidays day
            inVacations = true;           
         }
         else{
            inVacations = false;
         }
      }
      else{
            inVacations = false;
      }         
}

// Do not alter this function, it is used by the system
int getTimeAndDate() {
       int flag=0;
       Udp.begin(localPort);
       sendNTPpacket(timeServer);
       delay(1000);
       if (Udp.parsePacket()){
         Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
         unsigned long highWord, lowWord, epoch;
         highWord = word(packetBuffer[40], packetBuffer[41]);
         lowWord = word(packetBuffer[42], packetBuffer[43]);  
         epoch = highWord << 16 | lowWord;
         epoch = epoch - 2208988800 + timeZoneOffset;
         flag=1;
         setTime(epoch);
         ntpLastUpdate = now();
       }
       return flag;
}
     
// do not alter this function, it is used by the system
unsigned long sendNTPpacket(IPAddress& address)
{
      memset(packetBuffer, 0, NTP_PACKET_SIZE);
      packetBuffer[0] = 0b11100011;
      packetBuffer[1] = 0;
      packetBuffer[2] = 6;
      packetBuffer[3] = 0xEC;
      packetBuffer[12]  = 49;
      packetBuffer[13]  = 0x4E;
      packetBuffer[14]  = 49;
      packetBuffer[15]  = 52;                  
      Udp.beginPacket(address, 123);
      Udp.write(packetBuffer,NTP_PACKET_SIZE);
      Udp.endPacket();
}
     
// clock display of the time and date (Basic)
void clockDisplay()
{
      Serial.print(hour());
      printDigits(minute());
      printDigits(second());
      Serial.print(" ");
      Serial.print(day());
      Serial.print(" ");
      Serial.print(month());
      Serial.print(" ");
      Serial.print(year());
      Serial.println();
}
     
// utility function for clock display: prints preceding colon and leading 0
void printDigits(int digits)
{
      Serial.print(":");
      if(digits < 10)
        Serial.print('0');
      Serial.print(digits);
}


