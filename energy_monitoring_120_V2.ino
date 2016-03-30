/* www.caletagreen.com
   Energy monitoring 120 V.
   
   Store data on m2mlight.com:
   - Tank temperature of solar heater
   - Real power consumption of solar heater
   - Real power generated of solar inverter
   - AC current generated of solar inverter
   
   Send an alert email when tank temperature is greater than 55 celcius degrees
   
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensors and alert on m2mlight.com and obtain api_key identifiers
   
*/

// Define what libraries to load
#include <bmNTC.h>
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>
#include <m2mData.h>  

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF4, 0x61 }; // arduino mac address
byte ip[] = { XXX, XXX, XXX, XXX };                     // arduino ip address

// Variables
EnergyMonitor emon1;             // create an instance for the solar heater
EnergyMonitor emon2;             // create an instance for a solar inverter
bmNTC sensor1;                   // temperature sensor
m2mData sensor;                  // create an instance for m2mData

void setup() 
 {
 
    // initialize tank temperature sensor
    sensor1.begin(A0,3950,10000,9800); //NTC 10K B3950 connected to A0 pin with a resistance around of 10k

    // Energy monitor Solar Heater  
    emon1.voltage(2, 125, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon1.current(3, 29);        // Current: input pin, calibration
    
    // Energy monitor Solar Inverter
    emon2.voltage(2, 125, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon2.current(4, 29);        // Current: input pin, calibration
    
    Ethernet.begin(mac, ip); // start the ethernet connection
    Serial.begin(9600);      // enable serial data print  
    delay(1000);

 }
 
void loop() {   

   // Define sensor api_key variables obtained from m2mlight.com
   String api_key_tank = String("XXXXXXXX");
   String api_key_heater = String("XXXXXXXX");
   String api_key_inverter = String("XXXXXXXX");
   String api_key_irms = String("XXXXXXXX");

   // Define alert api_key variable obtained from m2mlight.com
   String api_key_alert = String("XXXXXXXX");

   emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
   emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
    
   float realPowerHeater = emon1.realPower;          // extract Real Power into variable (Solar Heater)  
   float realPowerInverter = emon2.realPower;        // extract Real Power into variable (Solar Inverter)
   float tank_temp = sensor1.getTemperatureCelcius();// extract Temperature into variable
   float irms_inverter = emon2.Irms;                 // IRMS Inverter

   Serial.print("realPowerHeater:");  Serial.println(realPowerHeater); // 
   emon1.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("realPowerInverter:");   Serial.println(realPowerInverter); // 
   emon2.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("tank_temp:"); Serial.println(tank_temp);
   
   
   // Send alarm email if tank temperature >= 50 degrees
   if (tank_temp >= 58)
      sensor.sendAlertEmail(api_key_alert);

   // Store sensors values on m2mlight
   sensor.sendValue(api_key_tank, tank_temp);
   sensor.sendValue(api_key_heater, realPowerHeater);
   sensor.sendValue(api_key_inverter, realPowerInverter);
   sensor.sendValue(api_key_irms, irms_inverter);

   delay(15000); //send values every 15 seconds

}

