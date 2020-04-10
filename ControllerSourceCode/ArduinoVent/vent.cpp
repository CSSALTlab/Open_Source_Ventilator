
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

#ifdef VENTSIM
#else
  #include <Arduino.h>
#endif

#include "vent.h"
#include "hal.h"
#include "alarm.h"
#include "ui_native.h"
#include "breather.h"

//------------ Global -----------
 void ventLoop()
 {
    halLoop();
    evtDispatchAll();
    uiNativeLoop();
    breatherLoop();
 }

void ventSetup()
{
  alarmInit();     // must be called before uiNativeInit
  uiNativeInit();
}
 
