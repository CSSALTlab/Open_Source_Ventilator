
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
   
static unsigned long tm_led;
static int led_state = 0;

//----------- Locals -------------
static void _init() {
#ifndef VENTSIM
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  tm_led = millis();
}

static void blinkLED()
{
    if (tm_led + 1000 < millis()) {
        tm_led = millis();

        
#ifdef VENTSIM
        LOG("LED Timer expired");
#else
        if (led_state) {
          led_state = 0;
          digitalWrite(LED_BUILTIN, LOW);
        }
        else {
          led_state = 1;
          digitalWrite(LED_BUILTIN, HIGH);
        }
#endif
        
    }
}


//------------ Global -----------
 void ventLoop()
 {
    blinkLED();
 }

 void ventSetup() {
  _init();
 }
 
