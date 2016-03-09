/* www.caletagreen.com
   Energy monitoring 210 V.
   
   Store data on a DB cloud server:
   - Real power consumption of cooktop
   - Real power of the house (210 V)
   - Input voltage of the house
   - Input currents (phase1 and phase2) of the house
   
   This version V2 uses the arduino library m2mdata
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
m2mData sensor;

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

   // set api_key for each sensor. These api_key values are obtained from m2mlight.com
   String api_key1 = String("8YszrStin"); // Voltage
   String api_key2 = String("5EUmWZfnB"); // Cooker real power
   String api_key3 = String("10OjcBi0AQ"); // Current phase 1
   String api_key4 = String("11itawqPLz"); // Current phase 2
   String api_key5 = String("9vnsWkQpz"); // Total real power
   
   // send values to cloud
   sensor.sendValue(api_key1, supplyVoltage);
   sensor.sendValue(api_key2, realPowerCooker);
   sensor.sendValue(api_key3, IrmsPhase1);
   sensor.sendValue(api_key4, IrmsPhase2);
   sensor.sendValue(api_key5, realPowerTotal);

   delay(15000); //send values every 15 seconds

}



