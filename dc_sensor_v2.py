#!/usr/bin/python
#-------------------------------------------------------------------------------------------------------------------   
# www.caletagreen.com
# Use to monitor DC sensors based in ACS758
# It uses the MCP3008 Analogue-to-digital converter
# It uploads values to m2mlight.com platform
# It can run in background with: # nohup ./dc_sensor_v2.py > foo.out 2> foo.err < /dev/null
#
# Credits:
# - http://henrysbench.capnfatz.com/henrys-bench/arduino-current-measurements/acs758-arduino-current-sensor-tutorial/
# - http://www.raspberrypi-spy.co.uk/2013/10/analogue-sensors-on-the-raspberry-pi-using-an-mcp3008/
#---------------------------------------------------------------------------------------------------------------------

import spidev
import time
import os
import urllib

# Open SPI bus
spi = spidev.SpiDev()
spi.open(0,0)

# Function to read SPI data from MCP3008 chip
# Channel must be an integer 0-7
def ReadChannel(channel):
  adc = spi.xfer2([1,(8+channel)<<4,0])
  data = ((adc[1]&3) << 8) + adc[2]
  return data

# Function to convert data to voltage level,
# rounded to specified number of decimal places. 
def ConvertVolts(data,places):
  volts = (data * 3.3) / float(1023)
  volts = round(volts,places)  
  return volts

# Return the current in amps for a sensor based in ACS758 
def ACS758_Current_Measurement(RawValue, mVperAmp, ACSoffset):
   Voltage = 0
   Amps = 0
   Voltage = (RawValue / 1023.0) * 5000
   Amps = ((Voltage - ACSoffset) / mVperAmp)
   return Amps
  
# Define sensor channels
dc_solar_channel = 0
dc_wind_channel = 1

# Define delay between readings
delay = 15

while True:

  # Read the DC sensor data - solar panels
  light_level0 = ReadChannel(dc_solar_channel)
  light_volts0 = ConvertVolts(light_level0,2)

  # Read the DC sensor data - wind turbine
  light_level1 = ReadChannel(dc_wind_channel)
  light_volts1 = ConvertVolts(light_level1,2)

  # Current sensor of 100 Amp - solar
  current_value_solar = ACS758_Current_Measurement(light_volts0, 20, 2500)

  # Current sensor of 50 Amp - wind
  current_value_wind = ACS758_Current_Measurement(light_volts1, 40, 2500)

  # Print out results
  print "--------------------------------------------"  
  print("Solar : {} ({}V) {}".format(light_level0,light_volts0, current_value_solar))  
  print("Wind : {} ({}V) {}".format(light_level1,light_volts1, current_value_wind))  

  # sensor and alert apy_key obtained from m2mlight.com
  api_key_solar = "56pLJotkFX"
  api_key_wind = "578i6BKAlY"
  api_key_alert = "267aTXd0Lx"

  # Upload values to m2mlight.com platform using api_key's
  data1 = urllib.urlencode({"api_key":api_key_solar,"value":current_value_solar})
  u1 = urllib.urlopen("http://m2mlight.com/iot/pg_send_value?%s" % data1)

  data2 = urllib.urlencode({"api_key":api_key_wind,"value":current_value_wind})
  u2 = urllib.urlopen("http://m2mlight.com/iot/pg_send_value?%s" % data2)

  # If solar current is greater than 8 then it sends a notification
  if current_value_solar > 8:
     data3 = urllib.urlencode({"api_key":api_key_alert,"value":current_value_solar})
     v3 = urllib.urlopen("http://m2mlight.com/iot/pg_send_email?%s" % data3)

  # Wait before repeating loop
  time.sleep(delay)
 
