#ifndef CONFIG_H
#define CONFIG_H

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

/*======================================
  =                                    =
  =          Hardware Mapping          =
  =                                    =
  ======================================
 */

#ifndef VENTSIM
  #define WATCHDOG_ENABLE  // to disable watchdog comment out this line
#endif

//------------ Input Keys ---------------

#define KEY_DECREMENT_PIN   3
#define KEY_INCREMENT_PIN   4
#define KEY_SET_PIN         5

//------------ Output Valves -----------
#define VALVE_ACTIVE_LOW

#define VALVE_IN_PIN  6
#define VALVE_OUT_PIN  7

#define MONITOR_LED_PIN LED_BUILTIN

//--------- LCD Num Rows ----------
#define LCD_CFG_2_ROWS  0
#define LCD_CFG_4_ROWS  1


/*======================================
  =                                    =
  =      Paramater default Values      =
  =                                    =
  ======================================

  Note: in case Stored parameters are corrupted or empty
*/

#define  DEFAULT_VENT            0
#define  DEFAULT_BPS             15
#define  DEFAULT_DUTY_CYCLE      0
#define  DEFAULT_PAUSE           100
#define  DEFAULT_LCD_AUTO_OFF    0
#define  DEFAULT_BLE             0


#endif // CONFIG_H
