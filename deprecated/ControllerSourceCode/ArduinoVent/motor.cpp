
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
#include "hal.h"
#include "log.h"
#include "event.h"
#include "alarm.h"

#ifdef STEPPER_MOTOR_STEP_PIN


/*

Limitations:

This code is using bit-bang to generate pulses

 P_START: is the mechanism position where the bag is deflated.
 P_END: is the mechanism position the the bag deflated.
 P_MAX: mechanic limitation
 EOC: End Of Course sensor (switch)
 M: motor moves the ARM to squeeze the bag


 P_START ARM                           BAG                               P_END P_MAX
      |   |                             |                                   |   |
      v   v                             v                                   v   v

      |   ||                                                                |   ||
      |   |>    ((((((((((((((((((((((((())))))))))))))))))))))))))))))))   |   ||
      |   ||                                                                |   ||

        /   \                                                               ^
       |  M  |                                                              |
        \   /                                                              EOC

*/
#define MIN_STEP_PERIOD 1200 // in microseconds. this is limited by the motor (max RPM)

//#define MAX_STEP_SPEED (1000000 / MIN_STEP_PERIOD) // steps per second (833 for MIN_STEP_PERIOD = 1200)

#define     P_MAX       1000
#define     P_START     0
#define     P_END       600
#define     EOC         P_END

// initialization:
//   - If starts with EOC == true then move back until EOC is false. It must find before ( (P_MAX - EOC) *  2)
//      - otherwise, declare motor error "EOC always true" and give up
//   - if starts with EOC == false then move fwd until EOC becomes true. It must find in P_MAX.
//        - otherwise, declare motor error "EOC always false" and give up
//     - move back until EOC is false. It must find before ( (P_MAX - EOC) *  2)
//        - otherwise, declare motor error "EOC always true 2" and give up
//     - move back P_END - P_START steps and declare this final position as P_START.

typedef enum {
  ST_INIT,
  ST_INIT_MOVING_TO_END,
  ST_INIT_MOVING_OUT_OF_END,
  ST_INIT_MOVING_TO_START,
  
  ST_AIR_IN,
  ST_AIR_IN_PAUSE,
  ST_AIR_OUT,
  ST_AIR_OUT_PAUSE,
  ST_STOPPED,
  
  ST_ERROR
} state_t;

typedef enum {
  FWD,
  BWD
} dir_t;

static uint16_t   speed; // in steps per second
static state_t    state;
static uint64_t   microTimerRef;

static uint32_t   step_periodo = MIN_STEP_PERIOD;  // must be >= MIN_STEP_PERIOD 
static uint8_t    phase;
static dir_t      direction;
static uint32_t   stepCounter;
static uint32_t   progress;



static void updateMicroFreeRunningTimer();

//

//------------- Finite State Machine -----------

static void fsmSt_INIT()
{
  stepCounter = P_END;
  halMotorDir(FWD);
  state = ST_INIT_MOVING_TO_END;
}

static void fsmSt_INIT_MOVING_TO_END()
{
  if (stepCounter == 0) {
    halMotorDir(BWD);
    stepCounter = P_END;
    state = ST_INIT_MOVING_OUT_OF_END;
  }
  
}

static void fsmSt_INIT_MOVING_OUT_OF_END()
{
  
}

static void fsmSt_INIT_MOVING_TO_START()
{
  
}
 
static void fsmSt_AIR_IN()
{
  
}

static void fsmSt_AIR_IN_PAUSE()
{
  
}


static void fsmSt_AIR_OUT()
{
  
}


static void fsmSt_AIR_OUT_PAUSE()
{
  
}


static void fsmSt_STOPPED()
{
  
}


  
static void fsmSt_ERROR()
{
  
}

//---------------- Global functions ----------------
void motorInit()
{
  state = ST_AIR_IN_PAUSE; // ST_INIT;
  microTimerRef = halStartTimerRef();
}

void motorLoop()
{
  static int lastProrgress = 0;
  
  switch (state) {
    case ST_INIT:                   fsmSt_INIT();                   break;
    case ST_INIT_MOVING_TO_END:     fsmSt_INIT_MOVING_TO_END();     break;
    case ST_INIT_MOVING_OUT_OF_END: fsmSt_INIT_MOVING_OUT_OF_END(); break;
    case ST_INIT_MOVING_TO_START:   fsmSt_INIT_MOVING_TO_START();   break;
    case ST_AIR_IN:                 fsmSt_AIR_IN();                 break;
    case ST_AIR_IN_PAUSE:           fsmSt_AIR_IN_PAUSE();           break;
    case ST_AIR_OUT:                fsmSt_AIR_OUT();                break;
    case ST_AIR_OUT_PAUSE:          fsmSt_AIR_OUT_PAUSE();          break;
    case ST_STOPPED:                fsmSt_STOPPED();                break;
    case ST_ERROR:                  fsmSt_ERROR();                  break;
    default: return;

  }
  
  if (stepCounter == 0) return;
  //--------- may move motor if stepCounter > 0 --------
  if (phase == 0) {
    //halMotorStep (true);      // Step PIN high
    microTimerRef = halStartMicroTimerRef();
    phase++;
    return;
  }
  if (phase == 1) {
    if (halCheckMicroTimerExpired(microTimerRef, step_periodo/10)) {
      halMotorStep (true);      // Step PIN high
      phase++;
    }
    return;
  }

  if (phase == 2) {
    if (halCheckMicroTimerExpired(microTimerRef, step_periodo/2)) {
      halMotorStep (false);      // Step PIN low
      phase++;
    }
    return;
  }
  if (phase == 3) {
    if (halCheckMicroTimerExpired(microTimerRef, step_periodo)) {
      stepCounter--;
      phase = 0;

      if (stepCounter == 0) {
        progress = 100;
      }
      else {
        progress = 100 - (stepCounter * 100)/ P_END;
        if (progress >= 100) progress = 99;
        if (progress < 0) progress = 0;
        
      }
      if (lastProrgress != progress) {
        lastProrgress = progress;
        //LOGV("%d", progress);;
      }
    }
  }  
}


static int getStepPeriod(uint32_t milli) {
  int r = (milli * 1000) / P_END;
  if (r < MIN_STEP_PERIOD) {
    CEvent::post(EVT_ALARM, ALARM_IDX_UNDER_SPEED_MOTOR);
    LOG("motor underspeed");
    return MIN_STEP_PERIOD;
  }
  LOGV("Period = %d microsec", r);
  return r;
}

void motorStartInspiration(int millisec)
{
  LOG(">> motorStartInspiration");
  progress = 0;
  direction = FWD;
  halMotorDir(true);
  step_periodo = getStepPeriod(millisec);
  stepCounter = P_END;
  microTimerRef = halStartMicroTimerRef();
  
}

void motorStartExhalation(int millisec)
{
    LOG("<< motorStartExhalation");
  progress = 0;
  direction = BWD;
  halMotorDir(false);
  step_periodo = getStepPeriod(millisec);
  stepCounter = P_END;
  microTimerRef = halStartMicroTimerRef();
}

int motorGetProgress()
{
  return progress;
}



//------------------------------------------------------------
#endif // #ifdef STEPPER_MOTOR_STEP_PIN
