
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


#include "hal.h"
#include "event.h"
#include "config.h"
#include "properties.h"
#include "pressure.h"

#include <stdio.h>
#include <QElapsedTimer>
#include <QMediaPlayer>

static QElapsedTimer milliTimer;

static QMediaPlayer player;

static bool alarm = false;
static bool alarm_phase;

//---------- Constants ----------

#define TM_KEY_SAMPLING 5  // 5 ms

static MONITOR_LET_T monitor_led_speed = MONITOR_LED_NORMAL;


//-------- variables --------
static uint64_t tm_led;

static uint64_t tm_key_sampling;
static uint64_t tm_alarm;

static char lcdBuffer [LCD_NUM_ROWS][LCD_NUM_COLS];
static int cursor_col = 0, cursor_row = 0;



static QPlainTextEdit * lcdObj;
static QLabel * input_valve_on;
static QLabel * input_valve_off;
static QLabel * output_valve_on;
static QLabel * output_valve_off;


static void beep()
{
    player.play();
}

void halBeepAlarmOnOff( bool on)
{
  if (on == true) {
    alarm = true;
    alarm_phase = false;
    tm_alarm = halStartTimerRef();
    // turn tone on
    beep();

  }
  else {
    alarm = false;
    // turn tone off
  }
}

//----------- Locals -------------
void halInit(QPlainTextEdit * ed,
             QLabel * _input_valve_on,
             QLabel * _input_valve_off,
             QLabel * _output_valve_on,
             QLabel * _output_valve_off)
{

  player.setMedia(QUrl("qrc:/sound/Resource/tom_1s.mp3"));
  player.setVolume(80);

  milliTimer.start();
  lcdObj = ed;
  input_valve_on = _input_valve_on;
  input_valve_off = _input_valve_off;
  output_valve_on = _output_valve_on;
  output_valve_off = _output_valve_off;


  tm_led = halStartTimerRef();
  tm_alarm = halStartTimerRef();
  halLcdClear();
  propInit();
  pressInit();
}

uint64_t halStartTimerRef()
{
    return (uint64_t) milliTimer.elapsed();
}

bool halCheckTimerExpired(uint64_t timerRef, uint64_t time)
{
    uint64_t now = halStartTimerRef();
    if (timerRef + time < now)
        return true;
    return false;
}


void halSetMonitorLED (MONITOR_LET_T speed)
{
  monitor_led_speed = speed;
}

MONITOR_LET_T halGetMonitorLED ()
{
  return monitor_led_speed;
}

void halBlinkLED()
{
  uint64_t tm;
  if (monitor_led_speed == MONITOR_LED_FAST) {
    tm = TM_MONITOR_LED_FAST;
  }
  else if (monitor_led_speed == MONITOR_LED_SLOW) {
    tm = TM_MONITOR_LED_SLOW;
  }
  else {
    tm = TM_MONITOR_LED_NORMAL;
  }
  
    if (halCheckTimerExpired(tm_led, tm)) {
        tm_led = halStartTimerRef();        
#if 0
        LOG("LED Toggle");
#endif
        
    }
}

static void alarmToggler()
{
    if (alarm == true)
    if (halCheckTimerExpired(tm_alarm, TM_ALARM_PERIOD)) {
      tm_alarm = halStartTimerRef();
      if (alarm_phase == true) {
          alarm_phase = false;
          // turn tone on
          beep();
      }
      else {
          alarm_phase = true;
          // turn tone off
      }
    }
}

//-------- EEPROM ----------

//---- stubs -----
uint8_t EEPROM_read(int addr) {return 0;}
void EEPROM_write(uint8_t val, int addr){}


//-------- display --------
static void lcdUpdate()
{
    int i,r;
    char *s, *d;
    char out[((LCD_NUM_COLS + 1) * LCD_NUM_ROWS) + 1];
    d = out;
    for (r=0; r<LCD_NUM_ROWS; r++) {
        s = &lcdBuffer [r][0];
        for (i=0; i<LCD_NUM_COLS; i++) {
            *d++ = *s++;
        }
        *d++ = '\n';
    }
    *d++ = 0;
    lcdObj->setPlainText(out);
}


void halLcdClear()
{
    memset(lcdBuffer, 0x20, sizeof(lcdBuffer));
    cursor_col = 0;
    cursor_row = 0;

}

void halLcdSetCursor(int col, int row)
{
    if ( cursor_col >= LCD_NUM_COLS) {
        LOG("halLcdSetCursor: bad cursor_col");
        return;
    }
    if ( cursor_row >= LCD_NUM_ROWS) {
        LOG("halLcdSetCursor: bad cursor_row");
        return;
    }
    cursor_col = col;
    cursor_row = row;

}

void halLcdWrite(const char * txt)
{
  int n;
  if ( cursor_col >= LCD_NUM_COLS) {
      LOG("halLcdWrite: bad cursor_col");
      return;
  }
  if ( cursor_row >= LCD_NUM_ROWS) {
      LOG("halLcdWrite: bad cursor_row");
      return;
  }
  n = strlen(txt);
  if (n > ( LCD_NUM_COLS - cursor_col)) {
      LOG("halLcdWrite: clipping");
      n = LCD_NUM_COLS - cursor_col;
  }
  memcpy(&lcdBuffer[cursor_row][cursor_col], txt, n);
  // TODO: row overflow check or clipping
  lcdUpdate();
}

void halLcdWrite(int col, int row, const char * txt)
{
    halLcdSetCursor(col, row);
    halLcdWrite(txt);
}

//---------- valves -------------


void halValveInOn()
{
  //LOG(">>>>>> Valve IN ON");
  input_valve_off->hide();
  input_valve_on->show();
}
void halValveInOff()
{
    //LOG(">>>>>> Valve IN OFF");
    input_valve_on->hide();
    input_valve_off->show();

}
void halValveOutOn()
{
  //LOG("<<<<<<<< Valve OUT ON");
  output_valve_off->hide();
  output_valve_on->show();
}
void halValveOutOff()
{
  //LOG("<<<<<<<< Valve OUT OFF");
  output_valve_on->hide();
  output_valve_off->show();
}

extern unsigned int gAnalogPressure;
uint16_t halGetAnalogPressure()
{
    return (uint16_t) gAnalogPressure;
}

extern unsigned int gAnalogFlow;
uint16_t halGetAnalogFlow()
{
    return (uint16_t) gAnalogFlow;
}


#define STORAGE_FILENAME "ventsim_storage.dat"
//-------- storage ----------
bool halSaveDataBlock(uint8_t * data, int size)
{
  int len;
  FILE * fh = fopen(STORAGE_FILENAME, "wb");
  if (fh == NULL) {
      LOG("halSaveDataBlock: fopen fail.");
      return false;
  }

  len = fwrite(data, 1, (size_t) size, fh);
  if (len != size) {
      LOG("halSaveDataBlock: write fail.");
      fclose(fh);
      return false;
  }
  fclose(fh);
  LOG("halSaveDataBlock: OK.");
  return true;
}

bool halRestoreDataBlock(uint8_t * data, int size)
{
    int len;
    FILE * fh = fopen(STORAGE_FILENAME, "rb");
    if (fh == NULL) {
        LOG("halSaveDataBlock: fopen fail.");
        return false;
    }

    len = fread(data, 1, (size_t) size, fh);
    if (len != size) {
        LOG("halRestoreDataBlock: read fail.");
        fclose(fh);
        return false;
    }
    fclose(fh);
    LOG("halRestoreDataBlock: read OK.");
    return true;

}


//---------------- process keys ----------
#define   DEBOUNCING_N    4
typedef struct keys_st {
  int state; // 0-> released
  int count;
  int pin;
  int keyCode;
} keys_t;

static keys_t keys[3] = {
  {0, 0, KEY_DECREMENT_PIN, KEY_DECREMENT},
  {0, 0, KEY_INCREMENT_PIN, KEY_INCREMENT},
  {0, 0, KEY_SET_PIN, KEY_SET},
};

static void processKeys()
{

}

void halLoop()
{
  halBlinkLED();
  processKeys();
  propLoop();
  pressLoop();
  alarmToggler();

}

void halWriteSerial(char * s)
{

}



 
