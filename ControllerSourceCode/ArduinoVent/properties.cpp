
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
#include "config.h"
#include <stdint.h>

typedef struct __attribute__ ((packed))  props_st {
  uint8_t propVent;
  uint8_t propBps;
  uint8_t propDutyCycle;
  uint8_t propPause;
  uint8_t propLcdAutoOff;
  uint8_t propBle;
} PROPS_T;

static PROPS_T props = {
  .propVent         = DEFAULT_VENT,
  .propBps          = DEFAULT_BPS,
  .propDutyCycle    = DEFAULT_DUTY_CYCLE,
  .propPause        = DEFAULT_PAUSE,
  .propLcdAutoOff   = DEFAULT_LCD_AUTO_OFF,
  .propBle          = DEFAULT_BLE,
};

// Note: defaults values will takes place in case the stored parameters are corrupted or empty

const char * propDutyCycleTxt[4] = {
    "  1:1",
    "  1:2",
    "  1:3",
    "  1:4"
};

void propInit()
{

}


bool propSave()
{

}

void propSetVent(int val) {
    LOG("propSetVent");
    props.propVent = val;
}

void propSetBps(int val) {
    LOG("propSetBps");
    props.propBps = (uint8_t) val & 0x000000ff;
}

void propSetDutyCycle(int val) {
     LOG("propSetDutyCycle");
     props.propDutyCycle =  (uint8_t) val & 0x000000ff;
}

void propSetPause(int val) {
     LOG("propSetPause");
     props.propPause =  (uint8_t) val & 0x000000ff;
}

void propSetLcdAutoOff(int val) {
     LOG("propSetLcdAutoOff");
     props.propLcdAutoOff =  (uint8_t) val & 0x000000ff;
}

void propSetBle(int val) {
      LOG("propSetBle");
      props.propBle =  (uint8_t) val & 0x000000ff;
}


// ---------- Getters ------------
int propGetVent() {
//    LOG("propGetVent");
    return props.propVent;
}

int propGetBps() {
    LOG("propGetBps");
    return props.propBps;
}

int propGetDutyCycle() {
     LOG("propGetDutyCycle");
     return props.propDutyCycle;
}

int propGetPause() {
     LOG("propGetPause");
     return props.propPause;
}

int propGetLcdAutoOff() {
     LOG("propGetLcdAutoOff");
     return props.propLcdAutoOff;
}

int propGetBle() {
      LOG("propGetBle");
      return props.propBle;
}
