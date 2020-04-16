

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

#include "hal.h"
#include "alarm.h"
#if (USE_BMP280_PRESSURE_SENSOR == 1)

#include "bmp280_int.h"
#include <Wire.h>
#include "sSense-BMx280I2C_mv.h"  // GLG -- for BMP280
#include "log.h"

//#define SHOW_PREESURE_LOGS

#define I2C_ADDRESS 0x77 // SDO pin must be left high. Otherwise, if SDO grounded, address is 0x76

#define TM_LOG 2000
#define TM_INIT_RETRY 200
#define TM_READ_MIN_PERIOD  50 // max rate to query BMP is 50ms (read takes about 2 ms)... 
                               // we will create a state machine to read in multiple calls and no blocks whatsoever.


#ifdef SHOW_PREESURE_LOGS
  static uint64_t logTimer;
#endif

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

static uint64_t tm;
static uint8_t state; // 0~3 starting... 4 is error... >=10 is OK

static float temp;
static float hum;
static float fPressurePa;
static float fReferencePa;
static float gaugeCmH2O;

#ifdef SHOW_PREESURE_LOGS
    char buf[24];
#endif
  
static BMx280I2C ssenseBMx280(settings);

static void checkInit() {
  if (state >= 4) return; // error or OK
  
  if ( halCheckTimerExpired(tm, TM_INIT_RETRY) ) {
    if (ssenseBMx280.begin()) {
      LOGV("Model 0x%x", ssenseBMx280.chipModel() );

      // prerform a first read for reference
      BME280::PresUnit presUnit(BME280::PresUnit_Pa); // check if we can move this tinit
      BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);   
      ssenseBMx280.read(fPressurePa, temp, hum, tempUnit, presUnit); // TODO: protect this method in the library
      bmp280SetReference();
      
      tm = halStartTimerRef();
      state = 10; // init is completed
      return;
    }
    state++;
    if (state == 4) {
      CEvent::post(EVT_ALARM, ALARM_IDX_BAD_PRESS_SENSOR);
    }
    tm = halStartTimerRef();
  }
}

float bpm280GetPressure() {
  
  checkInit();
  if (state < 4) return BMP_ST__INITIALIZING; // error or OK 
  if (state == 4) return BMP_ST__NOT_FOUND; // error or OK

  if ( halCheckTimerExpired(tm, TM_READ_MIN_PERIOD) ) {
    tm = halStartTimerRef();
    BME280::PresUnit presUnit(BME280::PresUnit_Pa); // check if we can move this tinit
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);   
    ssenseBMx280.read(fPressurePa, temp, hum, tempUnit, presUnit); // TODO: protect this method in the library
    gaugeCmH2O = (fPressurePa - fReferencePa) * 0.0101972;

  }

#ifdef SHOW_PREESURE_LOGS
  if (halCheckTimerExpired(logTimer, TM_LOG)) {

    dtostrf(fPressurePa, 2, 2, buf);
    buf[sizeof(buf) - 1] = 0;
    LOGV("fPressurePa = %s", buf); // this is how to log variable format like printf

    dtostrf(fReferencePa, 2, 2, buf);
    buf[sizeof(buf) - 1] = 0;
    LOGV("fReferencePa = %s", buf); // this is how to log variable format like printf

    float f = getCmH2OGauge();
    dtostrf(f, 2, 2, buf);
    buf[sizeof(buf) - 1] = 0;
    LOGV("Gauge = %s\n", buf); // this is how to log variable format like printf
    
    logTimer = halStartTimerRef();
  }
#endif

  return fPressurePa;
  
}

void bmp280SetReference()
{
  fReferencePa = fPressurePa;
}

float getCmH2OGauge()
{
  return gaugeCmH2O;
}

void bpm280Init()
{
  Wire.begin();

  tm = halStartTimerRef(); 
  checkInit();

#ifdef SHOW_PREESURE_LOGS
  logTimer = halStartTimerRef();
#endif
}

//----------------------------------------------------
#endif // USE_BMP280_PRESSURE_SENSOR
