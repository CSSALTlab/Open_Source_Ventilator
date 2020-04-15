
/*************************************************************
 * Open Ventilator
 * Copyright (C) 2020 - Dr. Bill Schmidt-J68HZ, Jack Purdum and Marcelo Varanda
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


/*
 * History:
 *
  Release 1.01  April 1, 2020, Jack Purdum, W8TEE. Adjusted mapping to 5V from 3V
    Release 1.0   March 30, 2020 by Dr. Bill Schmidt-J68HZ.  This program ws written to interface a MP3V7010xx
                  gauge pressure sensor and will calculate instantaneous average pressures for the
                  Inhale or exhale cycle.  See comments at end for integrating variables to be displayed into the LCD.

LUA code:

int to InchH2O:

function getp(a)
  local p = 4.01463
  local d = 614
  local r = p * ((a / d) - 0.08) / 0.09
  return r
end

InchH2O to int
function geta(v)
  local p = 4.01463
  local d = 614
  local r = (((v * .09) / p) + 0.08 ) * d
  return r
end
*/

#include "pressure.h"
#include "log.h"
#include "hal.h"
#include "config.h"
#include "bmp280_int.h"
#include <stdint.h>

#ifdef VENTSIM
    #include<QRandomGenerator>
    #define TEST_RAND_MIN 300
    #define TEST_RAND_MAX 400
#endif

#if ( (USE_Mpxv7002DP_PRESSURE_SENSOR == 1) || (USE_Mpxv7002DP_FLOW_SENSOR == 1) )

#define SHOW_VAL

#define TM_LOG 2000
#define P_CONV 4.01463f
#define MAX_BIN_INPUT   614
#define MAX_BIN_INPUT_F 614.0

static int16_t tap_array[NUM_P_SENSORS][AVERAGE_BIN_NUMBER];
static int32_t accumulator[NUM_P_SENSORS];
static uint8_t head_idx = 0;
static uint8_t tail_idx = 0;
static uint8_t ready_cnt = 0;
static uint64_t tm_press;

static int32_t av[NUM_P_SENSORS];
static float cmH2O[NUM_P_SENSORS];

#ifdef SHOW_VAL
  static uint64_t tm_log;
#endif

void CalculateAveragePressure(psensor_t sensor)
{
  int i;
  uint16_t rawSensorValue;
  
  for (i=0; i<NUM_P_SENSORS; i++) {
    rawSensorValue = 0;
    if (i == 0) {

      #if (USE_Mpxv7002DP_PRESSURE_SENSOR == 1)
      /*****************************************
       *
       *      Analog NXP Mpxv7002DP pressure sensor
       *
       *****************************************/
        rawSensorValue = halGetAnalogPressure();

      #elif (USE_BMP280_PRESSURE_SENSOR == 1)
      /*****************************************
       *
       *      BOSH BMP280 pressure sensor
       *
       *****************************************/
        rawSensorValue = bpm280GetPressure();

      #else
        #warning "No pressure sensor defined in config.h"
      #endif

    } // if i == 0
    else {
      #if (USE_Mpxv7002DP_FLOW_SENSOR == 1)
      /*****************************************
       *
       *      Analog NXP Mpxv7002DP pressure sensor
       *
       *****************************************/
        rawSensorValue = halGetAnalogFlow();
      #endif
    }
    // clamp it to the max (max value provided by the sensor)
    if (rawSensorValue >= MAX_BIN_INPUT)
        rawSensorValue = MAX_BIN_INPUT - 1;

    if (ready_cnt >= AVERAGE_BIN_NUMBER)  {
      accumulator[i] -= tap_array[i][tail_idx] ;

    }
//    else {
//      ready_cnt++;
//    }

    tap_array[i][head_idx] = rawSensorValue;
    if (head_idx >= AVERAGE_BIN_NUMBER) head_idx = 0;
    accumulator[i] += rawSensorValue;

    av[i] = accumulator[i]/AVERAGE_BIN_NUMBER;
    cmH2O[i] = P_CONV * ((av[i] / MAX_BIN_INPUT_F) - 0.08) / 0.09;
    
  } // for loop

  tail_idx++;
  if (tail_idx >= AVERAGE_BIN_NUMBER) tail_idx = 0;
  head_idx++;
  if (head_idx >= AVERAGE_BIN_NUMBER) head_idx = 0;

  if (ready_cnt < AVERAGE_BIN_NUMBER) {
    ready_cnt++;
  }
  
}

//====================================================================
void pressInit()   {
#ifndef VENTSIM
  #if (USE_Mpxv7002DP_PRESSURE_SENSOR == 1)
    analogReference(DEFAULT); // Arduino function
  #endif
#endif
  
#if (USE_BMP280_PRESSURE_SENSOR == 1)
  bpm280Init();
#endif
  
  tm_press = halStartTimerRef();

#ifdef SHOW_VAL
  tm_log = tm_press;
#endif
}


void pressLoop()
{
  if (halCheckTimerExpired(tm_press, PRESSURE_READ_DELAY)) {
    CalculateAveragePressure(PRESSURE);
    tm_press = halStartTimerRef();
  }

#ifdef SHOW_VAL
  char buf[24];
  if (halCheckTimerExpired(tm_log, TM_LOG)) {
    LOGV("av = %d", av);
  #ifndef VENTSIM
    dtostrf(cmH2O[PRESSURE], 8, 2, buf);
    LOGV("Pa = %s\n", buf);
  #else
    LOGV("Pa = %f\n", cmH2O[PRESSURE]);
  #endif
    tm_log = halStartTimerRef();
  }
#endif
}

float pressGetFloatVal(psensor_t sensor) // in InchH2O
{
    return cmH2O[sensor];
}
int32_t pressGetRawVal(psensor_t sensor)
{
    return av[sensor];
}


//-----------------------------------------------------------------
#else
// Stubbs
void pressInit()   { }
void pressLoop() {}
float pressGetFloatVal(psensor_t sensor) { return 0.0; }
int32_t pressGetRawVal(psensor_t sensor) { return 0; }

#endif //#if ( (USE_Mpxv7002DP_PRESSURE_SENSOR == 1) || (USE_Mpxv7002DP_FLOW_SENSOR == 1) )
