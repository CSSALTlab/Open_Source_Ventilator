
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


#include "vent.h"
#include "hal.h"
#include "ui_native.h"
   

//----------- Locals -------------


static CUiNative * uiNative;


//------------ Global -----------
 void ventLoop()
 {
    halBlinkLED();
    evtDispatchAll();
 }

void ventSetup()
{
  uiNative = new CUiNative();
//  halLcdWrite(0,0,(const char *) "01234567890");
//  halLcdWrite(9,1,(const char *) "X1234567890");
//  halLcdWrite(14,2,(const char *) "Y1234567890");
//  halLcdWrite(0,3,(const char *) "Z1234567890");
}
 
