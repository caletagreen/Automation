/* www.caletagreen.com
   Energy monitoring OnGrid system
   
   Store the following information on m2mlight.com:
   
   - Irms of Inverter1 (Amps)
   - Irms of Inverter2 (Amps)
   - Vrms of Inverters  (Volts)
   - RealPower of Inverter1 (Watts)
   - RealPower of Inverter2 (Watts)
   - IDC battery (Amps)
   - IDC solar panels (Amps)
   - IDC wind turbine (Amps)
   - Vin of battery (Volts)
   - Vin of solar panels (Volts)
   - Vin of wind turbine (Volts)
   - Total generated_power (Watts)
     
   Use m2mData arduino library (https://github.com/m2mlight/m2mData)
   First, define sensors and alert on m2mlight.com and obtain api_key identifiers

   Credits:
   - http://henrysbench.capnfatz.com/henrys-bench/arduino-current-measurements/acs758-arduino-current-sensor-tutorial/
   - https://roysoala.wordpress.com/2012/04/20/energy-monitoring-using-pachube-and-arduino-1-0/ 
   - http://arduinotronics.blogspot.com/2015/07/arduino-acs712-current-sensor.html
     
*/

// Define what libraries to load
#include "EmonLib.h" 
#include <SPI.h>
#include <Ethernet.h>
#include <m2mData.h>  

// Configure the Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB }; // arduino mac address
byte ip[] = { 192, 168, 1, 21};                     // arduino ip address

// Variables
EnergyMonitor emon1;             // create an instance for inverter 1
EnergyMonitor emon2;             // create an instance for inverter 2
m2mData sensor;                  // create an instance for m2mData
float realPowerInverter1;
float realPowerInverter2;
float irmsInverter1;
float irmsInverter2;
float vrmsInverters;
float solar_generated_power = 0;
float wind_generated_power = 0;
float generated_power = 0;

//Measuring Current Using ACS712
const int analogIn = A0;
int mVperAmp = 66; // use 185 for 5A Module, 100 for 20A Module, 66 for 30A Module
int RawValue= 0;
int ACSoffset = 2500; 
double Voltage = 0;
double Amps = 0;

// Variables for measuring Current Using ACS....
float reading0 = 0;          // raw data
float reading1 = 0;          // raw data
float currentValueSolar = 0;     // IDC sensor value Solar Panels
float currentValueWind = 0;     // IDC sensor value Wind Turbine


// VDC 
float vout = 0.0;
float R1 = 100000.0; // resistance of R1 (100K) 
float R2 = 10000.0; // resistance of R2 (10K) 
int value = 0;
float vin_battery;
float vin_turbine;
float vin_panels;

// Define sensor api_key variables obtained from m2mlight.com
char api_key_irms_inverter1[] = "1njcd3";
char api_key_irms_inverter2[] = "1njch2";
char api_key_power_inverter1[] = "1njcu6";
char api_key_power_inverter2[] = "1njcp9";
char api_key_idc_battery[] = "1njct2";
char api_key_vrms_inverters[] = "1njcg1";
char api_key_vdc_battery[] = "1njco3";
char api_key_vdc_panels[] = "1njcq6";
char api_key_vdc_turbine[] = "1njcl2";
char api_key_generated_power[] = "1njco2";
char api_key_idc_solar[] = "1njct6";
char api_key_idc_wind[] = "1njcc3";


void setup() 
 {
    // Pines
    pinMode(analogIn, INPUT); // DC Battery    
    pinMode(3, INPUT); // VDC Wind Turbine
    pinMode(4, INPUT); // VDC Solar Panels
    pinMode(5, INPUT); // VDC Battery 
    pinMode(13, INPUT); // AC Inverter 1
    pinMode(14, INPUT); // AC Inverter 2
    pinMode(15, INPUT); // VAC Inverters
    pinMode(8, INPUT); // IDC solar
    pinMode(9, INPUT); // IDC wind

    // Energy monitor Inverter 1
    emon1.voltage(15, 125, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon1.current(13, 29);        // Current: input pin, calibration

    // Energy monitor Inverter 2
    emon2.voltage(15, 125, 1.7);  // Voltage: input pin, calibration, phase_shift
    emon2.current(14, 29);        // Current: input pin, calibration

    Ethernet.begin(mac, ip); // start the ethernet connection
    Serial.begin(9600);      // enable serial data print  
    delay(1000);

 }
 
void loop() {   

   // VDC Battery
   value = analogRead(5);
   vout = (value * 5.0) / 1024.0; 
   vin_battery = vout / (R2/(R1+R2));
   Serial.print(F("VDC battery = ")); Serial.println(vin_battery); 

   // VDC Wind Turbine
   value = analogRead(3);
   vout = (value * 5.0) / 1024.0; 
   vin_turbine = vout / (R2/(R1+R2));
   Serial.print(F("VDC turbine = ")); Serial.println(vin_turbine); 

   // VDC Solar Panels
   value = analogRead(4);
   vout = (value * 5.0) / 1024.0; 
   vin_panels = vout / (R2/(R1+R2));
   Serial.print(F("VDC panel = ")); Serial.println(vin_panels); 

   //Measuring Battery IDC Current Using ACS712
   RawValue = analogRead(analogIn);
   Voltage = (RawValue / 1024.0) * 5000; // Gets you mV
   Amps = ((Voltage - ACSoffset) / mVperAmp);
//   Serial.print("Raw Value = " ); // shows pre-scaled value 
//   Serial.print(RawValue); 
   Serial.print(F("\t mV = ")); // shows the voltage measured 
   Serial.print(Voltage,3); // the '3' after voltage allows you to display 3 digits after decimal point
   Serial.print(F("\t Amps = ")); // shows the amps measured 
   Serial.println(Amps,3); // the '3' after voltage allows you to display 3 digits after decimal point

   // IDC with ACS758 current sensors
   reading0 = analogRead(8); // Raw data reading
   currentValueSolar = (reading0 - 510) * 5 / 1024 / 0.04 + 0.04;      

   reading1 = analogRead(9); // Raw data reading
   currentValueWind = (reading1 - 510) * 5 / 1024 / 0.04 + 0.04; 

   // AC inverters
   emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out for Inverter 1
   emon2.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out for Inverter 2

   realPowerInverter1 = emon1.realPower;  
   realPowerInverter2 = emon2.realPower;  
   irmsInverter1 = emon1.Irms;
   irmsInverter2 = emon2.Irms;
   vrmsInverters = emon1.Vrms;


   // Generated power
   solar_generated_power = 0;
   wind_generated_power = 0;
   if (currentValueSolar >0 and vin_panels>0){
      solar_generated_power = currentValueSolar * vin_panels;
   }
   if (currentValueWind >0 and vin_turbine>0){
      wind_generated_power = currentValueWind * vin_turbine;
   }     
   generated_power = solar_generated_power + wind_generated_power;

   Serial.print(F("realPowerInverter1:"));  Serial.println(realPowerInverter1); // 
   emon1.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print(F("realPowerInverter2:"));   Serial.println(realPowerInverter2); // 
   emon2.serialprint();           // print out all variables (realpower, apparent power, Vrms, Irms, power factor)

   Serial.print(F("IDC solar:")); Serial.println(currentValueSolar);
   Serial.print(F("IDC wind:")); Serial.println(currentValueWind);

   Serial.print(F("Generated_power:")); Serial.println(generated_power);

   // Prevent undesired readings
   if (irmsInverter1<0)
      irmsInverter1 = 0;
   if (irmsInverter2<0)
      irmsInverter2 = 0;
   if (realPowerInverter1<0)
      realPowerInverter1 = 0;
   if (realPowerInverter2<0)
      realPowerInverter2 = 0;     

   // Store sensors values on m2mlight
   sensor.sendValue(api_key_irms_inverter1, irmsInverter1);
   sensor.sendValue(api_key_irms_inverter2, irmsInverter2);  
   sensor.sendValue(api_key_power_inverter1, realPowerInverter1);
   sensor.sendValue(api_key_power_inverter2, realPowerInverter2);
   sensor.sendValue(api_key_idc_battery, Amps);
   sensor.sendValue(api_key_vrms_inverters, vrmsInverters);
   sensor.sendValue(api_key_vdc_battery, vin_battery);
   sensor.sendValue(api_key_vdc_panels, vin_panels);
   sensor.sendValue(api_key_vdc_turbine, vin_turbine);
   sensor.sendValue(api_key_idc_solar, currentValueSolar);
   sensor.sendValue(api_key_idc_wind, currentValueWind);
   sensor.sendValue(api_key_generated_power, generated_power);

   delay(15000); //send values every 15 seconds
   
}


