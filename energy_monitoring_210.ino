/* www.caletagreen.com
   Energy monitoring 210 V.
   
   Store data on a DB cloud server:
   - Real power consumption of cooktop
   - Real power of the house (210 V)
   - Input voltage of the house
   - Input currents (phase1 and phase2) of the house
*/

// Define what libraries to load
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xE4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 15 };                     // arduino ip address
byte server[] = { 88, 198, 207, 77 };                // cloud server address

// Variables
EnergyMonitor emon1;             // Create an instance for induction cooker
EnergyMonitor emon2;             // Create an instance for phase1
EnergyMonitor emon3;             // Create an instance for phase2
EthernetClient client;

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

   // send values to DB
   send_value(8, supplyVoltage);
   send_value(5, realPowerCooker);
   send_value(9, realPowerTotal);
   send_value(10, IrmsPhase1);
   send_value(11, IrmsPhase2);
   delay(15000);

}

// Store a value identified with id_sensor
void send_value(int id_sensor, float value){

  if (client.connect(server, 80)>0) {            // connect to server
    client.print("GET /tutoiot/iot.php?valor="); // send values using GET
    client.print(value);
    client.print("&");
    client.print("id_sensor=");
    client.print(id_sensor);
    client.println(" HTTP/1.0");
    client.println("User-Agent: Arduino 1.0");
    client.println();
    Serial.println("Connected");
  } else {
    Serial.println("Not connected");
  }
  Serial.println("Disconnecting...");
  if (!client.connected()) {
    Serial.println("Disconnected!");
  }
  client.stop();
} 



