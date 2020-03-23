
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

#include "log.h"
#include "properties.h"

#define  DEFAULT_VENT            0
#define  DEFAULT_BPS             15
#define  DEFAULT_DUTY_CYCLE      0
#define  DEFAULT_PAUSE           100
#define  DEFAULT_LCD_AUTO_OFF    0
#define  DEFAULT_BLE             0


static int propVent         = DEFAULT_VENT;
static int propBps          = DEFAULT_BPS;
static int propDutyCycle    = DEFAULT_DUTY_CYCLE;
static int propPause        = DEFAULT_PAUSE;
static int propLcdAutoOff   = DEFAULT_LCD_AUTO_OFF;
static int propBle          = DEFAULT_BLE;

const char * propDutyCycleTxt[4] = {
    "  1:1",
    "  1:2",
    "  1:3",
    "  1:4"
};


void propSetVent(int val) {
    LOG("propSetVent");
    propVent = val;
}

void propSetBps(int val) {
    LOG("propSetBps");
    propBps = val;
}

void propSetDutyCycle(int val) {
     LOG("propSetDutyCycle");
     propDutyCycle = val;
}

void propSetPause(int val) {
     LOG("propSetPause");
     propPause = val;
}

void propSetLcdAutoOff(int val) {
     LOG("propSetLcdAutoOff");
     propLcdAutoOff = val;
}

void propSetBle(int val) {
      LOG("propSetBle");
      propBle = val;
}


// ---------- Getters ------------
int propGetVent() {
//    LOG("propGetVent");
    return propVent;
}

int propGetBps() {
    LOG("propGetBps");
    return propBps;
}

int propGetDutyCycle() {
     LOG("propGetDutyCycle");
     return propDutyCycle;
}

int propGetPause() {
     LOG("propGetPause");
     return propPause;
}

int propGetLcdAutoOff() {
     LOG("propGetLcdAutoOff");
     return propLcdAutoOff;
}

int propGetBle() {
      LOG("propGetBle");
      return propBle;
}

