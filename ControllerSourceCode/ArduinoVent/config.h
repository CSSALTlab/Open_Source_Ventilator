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
  =             Language               =
  =                                    =
  ======================================
 */

#define LANGUAGE_EN_US      1           // English
#define LANGUAGE_PT_BR      0           // Portuguese


/*======================================
  =                                    =
  =          Hardware Mapping          =
  =                                    =
  ======================================
 */

#define DEBUG_SERIAL_LOGS // MUST be commented out for production. Also a hack is needed to decrease buffers in HardwareSerial.h

#ifndef VENTSIM
  #define WATCHDOG_ENABLE  // to disable watchdog comment out this line
#else
enum {A0, A1, A2, A3, A4, A5, A6, A7};
#endif

#define PREESURE_ENABLE

/*************************************************
 * 
 *         B O A R D   S E L E C T I O N
 *                            
 *************************************************
 */
// Note: also add boards defined here in below's "Board check selection" just in case.
#define       HW_VERSION_CSSALT_PROTO_01        1  // CSSALT Board ref 1 (Arduino Nano)
#define       HW_VERSION_MV_01                  0  // Marcelo's prototype (Arduino Uno)
#define       HW_VERSION_MV_SIMULATOR           0
//-------------------------------------------------


//---- Board check selection -----
#if ( (HW_VERSION_CSSALT_PROTO_01 + \
       HW_VERSION_MV_01 + \
       HW_VERSION_MV_SIMULATOR \
    ) != 1)
  #error "At least one and only one HW_VERSION_xxx must be set to 1 in config.h"
#endif

/*
                ***********************************************
                *                                             *
                *                     BOARDS                  *
                *                                             *
                ***********************************************
*/

#if (HW_VERSION_CSSALT_PROTO_01 == 1)
/******************************************
 * 
 *         Hardware prototype PCB rev 01
 *                  Arduino Nano
 *                  
 ******************************************
 */
//------------ Input Keys ---------------

#define KEY_DECREMENT_PIN       A0
#define KEY_INCREMENT_PIN       A1
#define KEY_SET_PIN             A3

//------------ Output Valves -----------
#define VALVE_ACTIVE_LOW

#define VALVE_IN_PIN            2 // D2
#define VALVE_OUT_PIN           3 // D3

#ifndef BLUETOOTH_ENABLE
  #define MONITOR_LED_PIN LED_BUILTIN
#else
  #define MONITOR_LED_PIN       13
#endif

#define  ALARM_SOUND_PIN        6  // D6
//----------- PRESSURE_SENSOR ------------
#define PREESURE_ENABLE
#define PRESSURE_SENSOR_PIN     A7

//--------- LCD Num Rows ----------
// Default
#define LCD_CFG_20_COLS  0
#define LCD_CFG_16_COLS  1

#define LCD_CFG_2_ROWS  1
#define LCD_CFG_4_ROWS  0

// Parallel LCD
#define LCD_CFG_D7              8   // Connector Pin 11 - Digital 8
#define LCD_CFG_D6              9   // Connector Pin 12 - Digital9
#define LCD_CFG_D5              10  // Connector Pin 13 - Digital10
#define LCD_CFG_D4              11  // Connector Pin 14 - Digital11
#define LCD_CFG_E               12  // Connector Pin 15 - Digital12
#define LCD_CFG_RS              13  // Connector Pin 16 - Digital13

/* LiquidCrystal     (rs, enable, d4, d5, d6, d7)
   LiquidCrystal lcd( LCD_CFG_RS, LCD_CFG_E, LCD_CFG_D4, LCD_CFG_D5, LCD_CFG_D6, LCD_CFG_D7);
*/
//-------------------------------------------------------------------

#elif (HW_VERSION_MV_01 == 1)
/******************************************
 * 
 *         Hardware prototype Marcelo 01
 *                  Arduino Uno
 *                  
 ******************************************
 */
//------------ Input Keys ---------------

#define KEY_DECREMENT_PIN       3
#define KEY_INCREMENT_PIN       4
#define KEY_SET_PIN             5

//------------ Output Valves -----------
#define VALVE_ACTIVE_LOW

#define VALVE_IN_PIN            6
#define VALVE_OUT_PIN           7

#ifndef BLUETOOTH_ENABLE
  #define MONITOR_LED_PIN LED_BUILTIN
#else
  #define MONITOR_LED_PIN       13
#endif

//----------- PRESSURE_SENSOR ------------
#define PREESURE_ENABLE
#define PRESSURE_SENSOR_PIN     A7

//--------- LCD Num Rows ----------
// Default
#define LCD_CFG_I2C
#define LCD_CFG_20_COLS  0
#define LCD_CFG_16_COLS  1

#define LCD_CFG_2_ROWS  1
#define LCD_CFG_4_ROWS  0

// Parallel LCD
#define LCD_CFG_D7              8   // Connector Pin 11 - Digital 8
#define LCD_CFG_D6              9   // Connector Pin 12 - Digital9
#define LCD_CFG_D5              10  // Connector Pin 13 - Digital10
#define LCD_CFG_D4              11  // Connector Pin 14 - Digital11
#define LCD_CFG_E               12  // Connector Pin 15 - Digital12
#define LCD_CFG_RS              13  // Connector Pin 16 - Digital13

#elif (HW_VERSION_MV_SIMULATOR == 1)
/******************************************
 *
 *           S I M U L A T O R
 *
 *
 ******************************************
 */
//------------ Input Keys ---------------

#define KEY_DECREMENT_PIN       3
#define KEY_INCREMENT_PIN       4
#define KEY_SET_PIN             5

//--------- LCD Num Rows ----------

#define LCD_CFG_20_COLS  1
#define LCD_CFG_16_COLS  0

#define LCD_CFG_2_ROWS  0
#define LCD_CFG_4_ROWS  1



#endif // ----------------- END OF BOARDS DEFINITIONS ----------------------

/*
                ***********************************************
                *                                             *
                *                   Common                    *
                *                                             *
                ***********************************************
*/

#define TM_SAVE_TIMEOUT 30000 // save props to EEPROM is UI is "quiet" for longer than 30 seconds


/*======================================
  =                                    =
  =      Properties default Values     =
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



//-------------- Checks ---------------
#if (LCD_CFG_2_ROWS == 1)
  #define LCD_NUM_ROWS 2
#elif (LCD_CFG_4_ROWS == 1)
  #define LCD_NUM_ROWS 4
#else
  #error "At least one LCD_CFG_x_ROWS must be set to 1 in config.h"
#endif
#if ((LCD_CFG_2_ROWS == 1) && (LCD_CFG_4_ROWS == 1))
  #error "Only one LCD_CFG_x_ROWS must be set to 1 in config.h"
#endif

#if (LCD_CFG_20_COLS == 1)
  #define LCD_NUM_COLS 20
#elif (LCD_CFG_16_COLS == 1)
  #define LCD_NUM_COLS 16
#else
  #error "At least one LCD_CFG_XX_COLS must be set to 1 in config.h"
#endif
#if ((LCD_CFG_20_COLS == 1) && (LCD_NUM_COLS == 1))
  #error "Only one LCD_CFG_XX_COLS must be set to 1 in config.h"
#endif


//#define LOOP_MONITOR_PIN 5 // D5   this is for debugging only, should be always commented out
// Profile on April 06th: Main loop taking 180 microseconds to be processed.

//


#endif // CONFIG_H
