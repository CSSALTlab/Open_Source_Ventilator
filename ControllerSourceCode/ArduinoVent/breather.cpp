
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


#include "breather.h"
#include "properties.h"
#include "hal.h"
#include "log.h"
#include "pressure.h"
#include "event.h"

#define MINUTE_MILLI 60000
#define TM_WAIT_TO_OUT 50 // 50 milliseconds
#define TM_STOPPING 4000 // 4 seconds to stop


static int curr_pause;
static int curr_rate;
static int curr_in_milli;
static int curr_out_milli;
static int curr_total_cycle_milli;
static int curr_progress;
static uint64_t tm_start;

static const int rate[4] = {1,2,3,4} ;

static B_STATE_t b_state = B_ST_STOPPED;

int breatherGetPropress()
{
    return curr_progress;
}

void breatherStartCycle()
{
    curr_total_cycle_milli = MINUTE_MILLI / propGetBps();
    curr_pause = propGetPause();
    curr_rate = propGetDutyCycle();
    int in_out_t = curr_total_cycle_milli - (curr_rate + TM_WAIT_TO_OUT);
    curr_in_milli = (in_out_t/2) / rate[curr_rate];
    curr_out_milli = in_out_t - curr_in_milli;
    curr_progress = 0;
    tm_start = halStartTimerRef();
    b_state = B_ST_IN;
    halValveOutOff();
    halValveInOn();

#if 0
  LOG("Ventilation ON:");
  LOGV(" curr_total_cycle_milli = %d", curr_total_cycle_milli);
  LOGV(" curr_pause = %d", curr_pause);
  LOGV(" curr_in_milli = %d", curr_in_milli);
  LOGV(" curr_out_milli = %d", curr_out_milli);
#endif

}

B_STATE_t breatherGetState()
{
    return b_state;
}

static void fsmStopped()
{
  if (propGetVent() ) {
      breatherStartCycle();
  }
}

static void fsmIn()
{
    uint64_t m = halStartTimerRef();
    if (tm_start + curr_in_milli < m) {
        // in valve off
        halValveInOff();
        tm_start = halStartTimerRef();
        b_state = B_ST_WAIT_TO_OUT;
    }
    else {
        curr_progress = ((m - tm_start) * 100)/ curr_in_milli;
        //curr_progress = 100 - (100 * tm_start + curr_in_milli) / m;

        //--------- we check for low pressure at 50% or grater
        // low pressure hardcode to 3 InchH2O -> 90 int
        if (tm_start + curr_in_milli/2 < m) {
            if (pressGetRawVal() < 90) {
              CEvent::post(EVT_ALARM, EVT_ALARM_LOW_PRESSURE);
            }
        }
    }
    //------ check for high pressure hardcode to 35 InchH2O -> 531 int
    if (pressGetRawVal() > 513) {
      CEvent::post(EVT_ALARM, EVT_ALARM_HIGH_PRESSURE);
    }
}

static void fsmWaitToOut()
{
    if (halCheckTimerExpired(tm_start, TM_WAIT_TO_OUT)) {
        // switch valves
        tm_start = halStartTimerRef();
        b_state = B_ST_OUT;
        halValveOutOn();
    }
}

static void fsmOut()
{
    uint64_t m = halStartTimerRef();
    if (tm_start + curr_out_milli < m) {
        // switch valves
        tm_start = halStartTimerRef();
        b_state = B_ST_PAUSE;
        halValveOutOff();
    }
    else {
        //curr_progress = 100 - (m - tm_start / curr_out_milli * 100);
        //curr_progress = (100 * tm_start + curr_in_milli) / m;
        curr_progress = 100 - ((m - tm_start) * 100)/ curr_out_milli;
    }
}

static void fsmStopping()
{
    if (halCheckTimerExpired(tm_start, TM_STOPPING)) {
        // switch valves
        tm_start = halStartTimerRef();
        b_state = B_ST_STOPPED;
        halValveOutOff();
        halValveInOff();
    }
}

static void fsmPause()
{
    if (halCheckTimerExpired(tm_start, curr_pause)) {
        breatherStartCycle();
    }
}

void breatherLoop()
{
    if (b_state != B_ST_STOPPED && b_state != B_ST_STOPPING && propGetVent() == 0) {
        // force stop
        tm_start = halStartTimerRef();
        b_state = B_ST_STOPPING;
        curr_progress = 0;
        halValveInOff();
        halValveOutOn();
    }

    if (b_state == B_ST_STOPPED)
        fsmStopped();
    else if (b_state == B_ST_IN)
        fsmIn();
    else if (b_state == B_ST_WAIT_TO_OUT)
        fsmWaitToOut();
    else if (b_state == B_ST_OUT)
        fsmOut();
    else if (b_state == B_ST_PAUSE)
        fsmPause();
    else if (b_state == B_ST_STOPPING)
        fsmStopping();
    else {
        LOG("breatherLoop: unexpected state");
    }
}
