
/* www.caletagreen.com
   DC solar current measurement   
   Get raw data from 50A current sensor
   Use m2mData arduino library   
*/


#include <Arduino.h>
#include <m2mData.h>
#include <Ethernet.h>
#include <SPI.h>

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xB4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 17 };                     // arduino ip address


m2mData sensor;

float reading = 0;        
float currentValue = 0;
 
void setup(){

    Ethernet.begin(mac, ip); // start the ethernet connection
    Serial.begin(115200);      // enable serial data print  
    delay(1000);

}

void loop() {
 
  String api_key_values = String("18kemLflHQ");
  String api_key_alert = String("14cNxdzFea");

  reading = analogRead(0); //Raw data reading
  currentValue = (reading - 510) * 5 / 1024 / 0.04 - 5.14; 

  Serial.println(currentValue);
  
  // store sensor value identified by api_key
  sensor.sendValue(api_key_values, currentValue);
   
  if (currentValue > 8){
     // send alert email
     sensor.sendAlertEmail(api_key_alert);
  }
    
  delay(30000);
}
