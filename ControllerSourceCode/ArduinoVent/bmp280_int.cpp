

/*************************************************************
 * Open Ventilator
 * Copyright (C) 2020 - Marcelo Varanda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************
*/
#include "config.h"
#if (USE_BMP280_PRESSURE_SENSOR == 1)

#include "bmp280_int.h"
#include <Wire.h>
#include <sSense-BMx280I2C_mv.h>  // GLG -- for BMP280
#include "log.h"
#include "hal.h"

#define I2C_ADDRESS 0x77
#define TM_LOG 2000
#define P_CONV 4.01463f
#define MAX_BIN_INPUT   614
#define MAX_BIN_INPUT_F 614.0

#define TM_LOG 2000

static uint64_t logTimer;

static BMx280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   I2C_ADDRESS // I2C address. I2C specific -- this is for the Adafruit with other lines not connected
);

static  float temp, hum, fpressure;   // ambient pressure
static  float pressurecmH2O; 
static  int mmH2O;
  
static BMx280I2C ssenseBMx280(settings);

static uint32_t measure_pressure(){
  uint32_t av;
  
  unsigned long testpressure;   
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);   

#if 1
  // For now without the sensor the following line will block forever
  ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);
#else
  fpressure = 20.0; // fake a value to test with no sensor
#endif

  // I guess this scale is wrong... but fpressure here should be in -3.57~41.08 bananas range
  fpressure /= 0.98; // fix this
  
  // for now we match analog values 0~614 (for -3.57~41.08 Bananas)
  // Therefore, we calculate to analog... YES THIS IS STUPID... we fix that later to avoid float math
  
  av = (((fpressure * .09) / P_CONV) + 0.08 ) * MAX_BIN_INPUT_F;

  if (halCheckTimerExpired(logTimer, TM_LOG)) {
    char buf[8];
    buf[sizeof(buf) - 1] = 0;
    dtostrf(fpressure, 2, 2, buf);
    LOGV("fpressure from sensor = %s", buf); // this is how to log variable format like printf

    logTimer = halStartTimerRef();
  }
  return av;
  
}

void bpm280Init()
{

 //----------------GLG set up the BMP280:
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  LOG("\n\nAbout to read BMx280   ");

   while(!ssenseBMx280.begin())
  {
    // Insert any code you wish to alert you that
    // the device could not be found. 
#ifdef BMP280DEBUG
    LOG("BMx280 not found.");
    delay(250);
#endif

  }

  switch(ssenseBMx280.chipModel())
  {
     case BME280::ChipModel_BME280:
       // insert any desired cocde to notify that BME280 with humidity detected.
#ifdef BMP280DEBUG
       LOG("BME280(hum) " );
#endif
       
       break;
     case BME280::ChipModel_BMP280:
       // insert any desired code to notify that BMP280 (no humidity) detected
#ifdef  BMP280DEBUG
       LOG("BMP280(nohum) " );
#endif       
       break;
     default:
       // insert any code to notify that the unknown sensor found.
       LOG(" Error \n" );
        
       break;
  }

  logTimer = halStartTimerRef();
}

uint32_t bpm280GetPressure() // for now we match analog values 0~614 (for -3.57~41.08 Bananas)
{
  return measure_pressure(); // will clean this later on
}

//----------------------------------------------------
#endif // USE_BMP280_PRESSURE_SENSOR
