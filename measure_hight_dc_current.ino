
/* www.caletagreen.com
   DC solar current measurement
   
   Get raw data from 50A current sensor
   Store sensor values on m2mlight.com
   Send an alert email when DC current is greater than 8 Amp
   
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensor and alert on m2mlight.com and obtain api_key identifiers

*/

// Define what libraries to load
#include <Arduino.h>
#include <m2mData.h>
#include <Ethernet.h>
#include <SPI.h>

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xB4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 17 };                     // arduino ip address

// Variables
float reading = 0;          // raw data
float currentValue = 0;     // DC sensor value
m2mData sensor;             // create an instance for m2mData

 
void setup(){

    Ethernet.begin(mac, ip);   // start the ethernet connection
    Serial.begin(115200);      // enable serial data print  
    delay(1000);

}

void loop() {
 
  // Define api_key variables obtained from m2mlight.com
  String api_key_value = String("18kemLflHQ");
  String api_key_alert = String("14cNxdzFea");

  reading = analogRead(0); // Raw data reading
  currentValue = (reading - 510) * 5 / 1024 / 0.04 - 5.14; 

  Serial.println(currentValue);
  
  // Store sensor value identified by api_key on m2mlight
  sensor.sendValue(api_key_value, currentValue);
   
  if (currentValue > 8){
     // send alert email
     sensor.sendAlertEmail(api_key_alert);
  }
    
  delay(30000); //send values every 30 seconds
}
