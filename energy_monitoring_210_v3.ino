/* www.caletagreen.com
   Energy monitoring 210 V.
   
   Store data on m2mlight.com:
   - Real power consumption of cooktop
   - Real power of the house (210 V)
   - Input voltage of the house
   - Input currents (phase1 and phase2) of the house
     
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensors on m2mlight.com and obtain api_key identifiers

   Credits:
   - https://roysoala.wordpress.com/2012/04/20/energy-monitoring-using-pachube-and-arduino-1-0/ 

*/

// Define what libraries to load
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>
#include <m2mData.h>

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xE4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 15 };                     // arduino ip address

// Variables
EnergyMonitor emon1;             // Create an instance for induction cooker
EnergyMonitor emon2;             // Create an instance for phase1
EnergyMonitor emon3;             // Create an instance for phase2
m2mData sensor;                  // create an instance for m2mData

void setup() 
 {
 
    // Energy monitor Cooktop 
    emon1.voltage(2, 245.5, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon1.current(3, 29);          // Current: input pin, calibration
    
    // Energy monitor Phase1
    emon2.voltage(2, 245.5, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon2.current(4, 29);          // Current: input pin, calibration 

    // Energy monitor Phase2
    emon3.voltage(2, 245.5, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon3.current(5, 29);          // Current: input pin, calibration 
    
    Ethernet.begin(mac, ip); // start the ethernet connection
    Serial.begin(9600);      // enable serial data print  
    delay(1000);

 }

// Define api_key for each sensor. These api_key values are obtained from m2mlight.com
char api_key1[] = "1nja1c"; // Voltage
char api_key2[] = "1njc28"; // Cooker real power
char api_key3[] = "1njd49"; // Current phase 1
char api_key4[] = "1njkia"; // Current phase 2
char api_key5[] = "1njwib"; // Total real power
 
void loop() {   

   emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
   emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
   emon3.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
    
   float supplyVoltage   = emon1.Vrms;      // extract voltage into variable           
   float realPowerCooker = emon1.realPower; // extract Real Power into variable
   float IrmsPhase1      = emon2.Irms;      // extract current1 into variable
   float IrmsPhase2      = emon3.Irms;      // extract current2 into variable
   float realPowerTotal =  (abs(IrmsPhase1) * supplyVoltage / 1.73) + (abs(IrmsPhase2) * supplyVoltage / 1.73);
  
   Serial.print("realPowerCooker:"); Serial.println(realPowerCooker); // 
   emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("IrmsPhase1:"); Serial.println(IrmsPhase1); // 
   emon2.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("IrmsPhase2:"); Serial.println(IrmsPhase2); // 
   emon3.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("realPowerTotal:"); Serial.println(realPowerTotal); // 

   // Prevent undesired readings
   if (realPowerCooker<0)
      realPowerCooker = 0;
   if (IrmsPhase1<0)
      IrmsPhase1 = 0;
   if (IrmsPhase2<0)
      IrmsPhase2 = 0;
   if (realPowerTotal<0)
      realPowerTotal = 0;
   
   // Store sensors values on m2mlight
   sensor.sendValue(api_key1, supplyVoltage);
   sensor.sendValue(api_key2, realPowerCooker);
   sensor.sendValue(api_key3, IrmsPhase1);
   sensor.sendValue(api_key4, IrmsPhase2);
   sensor.sendValue(api_key5, realPowerTotal);

   delay(15000); //send values every 15 seconds

}




