
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

#define MINUTE_MILLI 60000
#define TM_WAIT_TO_OUT 50 // 50 milliseconds


static int curr_pause;
static int curr_rate;
static int curr_in_milli;
static int curr_out_milli;
static int curr_total_cycle_milli;
static int curr_progress_cycly_milli;
static unsigned long tm_start;

static const int rate[4] = {1,2,3,4} ;

static B_STATE_t b_state = B_ST_STOPPED;

void breatherStartCycle()
{
    curr_total_cycle_milli = MINUTE_MILLI / propGetBps();
    curr_pause = propGetPause();
    curr_rate = propGetDutyCycle();
    int in_out_t = curr_total_cycle_milli - (curr_rate + TM_WAIT_TO_OUT);
    curr_out_milli = (in_out_t/2) / rate[curr_rate];
    curr_in_milli = in_out_t - curr_out_milli;
    curr_progress_cycly_milli = 0;
    tm_start = millis();
    b_state = B_ST_IN;
    halValveOutOff();
    halValveInOn();
#ifdef VENTSIM
  char buf[256];
  sprintf(buf, "  curr_total_cycle_milli = %d\n  curr_pause = %d\n  curr_in_milli = %d\n  curr_out_milli = %d\n",
          curr_total_cycle_milli,
          curr_pause,
          curr_in_milli,
          curr_out_milli);
  LOG(buf);
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
    if (tm_start + curr_in_milli < millis()) {
        // in valve off
        halValveInOff();
        tm_start = millis();
        b_state = B_ST_WAIT_TO_OUT;
    }
}

static void fsmWaitToOut()
{
    if (tm_start + TM_WAIT_TO_OUT < millis()) {
        // switch valves
        tm_start = millis();
        b_state = B_ST_OUT;
        halValveOutOn();
    }
}

static void fsmOut()
{
    if (tm_start + curr_out_milli < millis()) {
        // switch valves
        tm_start = millis();
        b_state = B_ST_PAUSE;
        halValveOutOff();
    }
}

static void fsmPause()
{
    if (tm_start + curr_pause < millis()) {
        breatherStartCycle();
    }
}

void breatherLoop()
{
    if (propGetVent() == 0) {
        // force stop
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
    else {
        LOG("breatherLoop: unexpected state");
    }
}
