
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
   
static int gCount = 0;

//----------- Locals -------------
static void _setup() {
#ifndef VENTSIM
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
#endif
}

// the loop function runs over and over again forever
static void _loop() {
#ifndef VENTSIM
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
  delay(100);                       // wait for half a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
  delay(100);                       // wait for half a second
#endif
}

//------------ Global -----------
 void ventLoop()
 {
   gCount++; 
 //  LOG("This is printf counter = %d\n");
 _loop();
 

 }

 void ventSetup() {
  _setup();
 }
 
