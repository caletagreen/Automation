/* www.caletagreen.com
   Energy monitoring solar heater
   
   Store sensor data on m2mlight.com using the mqtt protocol:
   - Tank temperature of solar heater
   - Real power consumption of solar heater
   
   m2mlight.com send an alert sensor email when tank temperature is greater than a defined MAX value
   
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensors on m2mlight.com and obtain api_key sensors identifiers

   Credits:
   - https://roysoala.wordpress.com/2012/04/20/energy-monitoring-using-pachube-and-arduino-1-0/ 
*/

// Define what libraries to load
#include <bmNTC.h>
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>
#include <m2mData.h>  

// Configure the Ethernet Shield

byte mac[] = { 0x90, 0xA3, 0xDC, 0x0D, 0xD4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 20 };                     // arduino ip address

// Variables
EnergyMonitor emon1;             // create an instance for the solar heater
bmNTC sensor1;                   // temperature sensor
m2mData sensor;                  // create an instance for m2mData

// Define sensor api_key variables obtained from m2mlight.com
char api_key_tank[] = "1njci7";
char api_key_heater[] = "1njci6";

void setup() 
 {
 
    // initialize tank temperature sensor
    sensor1.begin(A1,3950,10000,9800); //NTC 10K B3950 connected to A1 pin with a resistance around of 10k

    // Energy monitor Solar Heater  
    emon1.voltage(4, 125, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon1.current(3, 29);        // Current: input pin, calibration
       
    Ethernet.begin(mac, ip); // start the ethernet connection
    Serial.begin(9600);      // enable serial data print  
    delay(1000);

 }
 
void loop() {   

   emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
    
   float realPowerHeater = emon1.realPower;          // extract Real Power into variable (Solar Heater)  
   float tank_temp = sensor1.getTemperatureCelcius();// extract Temperature into variable

   Serial.print(F("realPowerHeater:"));  Serial.println(realPowerHeater); // 
   emon1.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print(F("tank_temp:")); Serial.println(tank_temp);

   float supplyVoltage   = emon1.Vrms;      // extract voltage into variable           
   Serial.print(F("voltage:")); Serial.println(supplyVoltage);
   
   if (realPowerHeater<0)
     realPowerHeater = 0; // Prevent undesired readings

   // Store sensors values on m2mlight using mqtt protocol
   sensor.sendValueMqtt(api_key_tank, tank_temp);
   sensor.sendValueMqtt(api_key_heater, realPowerHeater);
 
   delay(20000); //send values every 20 seconds

}

