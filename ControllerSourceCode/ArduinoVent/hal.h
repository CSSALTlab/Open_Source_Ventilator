#ifndef HAL_H
#define HAL_H

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
  #include "arduino_libs.h"
  #include <QPlainTextEdit>
#else
  #include <Arduino.h>
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef VENTSIM
  void halInit(QPlainTextEdit * ed);
#else
  void halInit();
  void LOG();
#endif

void halBlinkLED();
void halLcdClear();
void halLcdSetCursor(int col, int row);
void halLcdWrite(const char * txt);
void halLcdWrite(int col, int row, const char * txt);


#endif // HAL_H
