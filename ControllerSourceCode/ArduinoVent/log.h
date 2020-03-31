#ifndef LOG_H
#define LOG_H

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
//#include "hardwareSerial.h"

#ifndef VENTSIM
  #include <avr/pgmspace.h>
  #ifdef DEBUG_SERIAL_LOGS
    #define LOG(x) Serial.println(F(x))
    #define LOGV(x)  Serial.println(x)
  #else
    #define LOG(x) /* dummy */
    #define LOGV(x) /* dummy */
  #endif
#else
  void LOG(const char * txt); // goes to flash memory in Arduino
  void LOGV(char * txt);      // char * comes from a variable
#endif

#endif // LOG_H
