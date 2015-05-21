/* www.caletagreen.com
   Energy monitoring 120 V.
   
   Store data on a DB cloud server:
   - Tank temperature of solar heater
   - Real power consumption of solar heater
   - Real power generated of solar inverter
*/

// Define what libraries to load
#include <bmNTC.h>
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>

// Configure the Ethernet Shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF4, 0x61 }; // arduino mac address
byte ip[] = { 192, 168, 1, 10 };                     // arduino ip address
byte server[] = { 88, 198, 207, 77 };                // cloud server address

// Variables
EnergyMonitor emon1;             // create an instance for the solar heater
EnergyMonitor emon2;             // create an instance for a solar inverter
bmNTC sensor1;                   // temperature sensor
EthernetClient client;

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
  
   emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
   emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out 
    
   float realPowerHeater = emon1.realPower;          // extract Real Power into variable (Solar Heater)  
   float realPowerInverter = emon2.realPower;        // extract Real Power into variable (Solar Inverter)
   float tank_temp = sensor1.getTemperatureCelcius();// extract Temperature into variable

   Serial.print("realPowerHeater:");  Serial.println(realPowerHeater); // 
   emon1.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("realPowerInverter:");   Serial.println(realPowerInverter); // 
   emon2.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print("tank_temp:"); Serial.println(tank_temp); 

   // send values to DB
   send_value(1, tank_temp);
   send_value(4, realPowerHeater);
   send_value(3, realPowerInverter);
 
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

