
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

#define MINUTE_MILLI 60000
typedef enum {
    B_ST_STOPPED,
    B_ST_IN,
    B_ST_OUT,
    B_ST_PAUSE,
} B_STATE_t;

static int curr_pause;
static int curr_in_milli;
static int curr_out_milli;
static int curr_total_cycly_milli;
static int curr_progress_cycly_milli;
static unsigned long tm_start;

static const int rate[4] = {1,2,3,4} ;

static B_STATE_t b_state = B_ST_STOPPED;

static void breatherStartCycle()
{
    curr_total_cycly_milli = MINUTE_MILLI / propGetBps();
    curr_pause = propGetPause();
    int in_out_t = curr_total_cycly_milli - curr_pause;
    curr_out_milli = (in_out_t/2) / rate[curr_pause];
    curr_in_milli = in_out_t - curr_out_milli;
    curr_progress_cycly_milli = 0;
    tm_start = millis();
    b_state = B_ST_IN;
    halValveInOn();
}

void breatherLoop()
{


}
