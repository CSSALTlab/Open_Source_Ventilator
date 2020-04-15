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
#include "log.h"
#include <stdint.h>
#include "config.h"

#ifdef VENTSIM
  #include <QPlainTextEdit>
  #include <QLabel>
  #define PROGMEM /**/
  #define F /**/
#else
  #include <Arduino.h>
    #include <avr/pgmspace.h>
    #ifdef LCD_CFG_I2C
      #include "LiquidCrystal_I2C_mv.h"
    #else 
      #include "LiquidCrystal.h"
    #endif
#endif

#define TM_MONITOR_LED_NORMAL       500
#define TM_MONITOR_LED_FAST         50 //
#define TM_MONITOR_LED_SLOW         2000

#define TM_ALARM_PERIOD 1000
#define TM_KEY_SAMPLING 5  // 5 ms

typedef enum {
  MONITOR_LED_NORMAL,
  MONITOR_LED_FAST,
  MONITOR_LED_SLOW,
} MONITOR_LET_T;

#ifdef VENTSIM
  void halInit(QPlainTextEdit * ed,
               QLabel * input_valve_on,
               QLabel * input_valve_off,
               QLabel * output_valve_on,
               QLabel * output_valve_off);
#else
  void halInit(uint8_t reset_val);
#endif

void halSetMonitorLED (MONITOR_LET_T speed);
MONITOR_LET_T halGetMonitorLED ();

uint64_t halStartTimerRef(); // milliseconds reference
bool halCheckTimerExpired(uint64_t timerRef, uint64_t lapseTime); // lapseTime in milliseconds

#ifdef ENABLE_MICROSEC_TIMER
  uint64_t halStartMicroTimerRef(); // microseconds reference
  bool halCheckMicroTimerExpired(uint64_t timerRef, uint64_t lapseMicroTime); // lapseMicroTime in microseconds
#endif

void halWriteSerial(char * s);

void halLoop();
void halBlinkLED();
void halLcdClear();
void halLcdSetCursor(int col, int row);
void halLcdWrite(const char * txt);
void halLcdWrite(int col, int row, const char * txt);

void halValveInOn();
void halValveInOff();
void halValveOutOn();
void halValveOutOff();

void halBeepAlarmOnOff( bool on);

uint16_t halGetAnalogPressure();
uint16_t halGetAnalogFlow();

//uint8_t EEPROM_read(int addr);
//void EEPROM_write(uint8_t val, int addr);

bool halSaveDataBlock(uint8_t * data, int size);
bool halRestoreDataBlock(uint8_t * data, int size);

void halMotorStep(bool on);
void halMotorDir(bool dir);
bool halMotorEOC();

#endif // HAL_H
