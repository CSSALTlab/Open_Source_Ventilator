
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

//---------- Constants ---------
#define VALVE_ACTIVE_LOW

#define KEY_FUNC_PIN  3
#define KEY_LEFT_PIN  4
#define KEY_RIGHT_PIN  5

#define VALVE_IN_PIN  6
#define VALVE_OUT_PIN  7

#define TM_KEY_SAMPLING 5  // 5 ms

//-------- variables --------
static unsigned long tm_led;

static unsigned long tm_key_sampling;

static char lcdBuffer [LCD_NUM_ROWS][LCD_NUM_COLS];
static int cursor_col = 0, cursor_row = 0;


#ifdef VENTSIM
  static QPlainTextEdit * lcdObj;
#else
  static int led_state = 0;
  

#endif

//----------- Locals -------------
#ifdef VENTSIM
void halInit(QPlainTextEdit * ed) {
  lcdObj = ed;
  tm_led = millis();
  halLcdClear();
}
#else
LiquidCrystal_I2C lcd(0x27,4,4);  
  
void halInit() {
  pinMode(LED_BUILTIN, OUTPUT);
  tm_led = millis();
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  halLcdClear();

  // -----  keys -------
  pinMode(KEY_FUNC_PIN, INPUT);           // set pin to input
  digitalWrite(KEY_FUNC_PIN, HIGH);       // turn on pullup resistors
  pinMode(KEY_RIGHT_PIN, INPUT);           // set pin to input
  digitalWrite(KEY_RIGHT_PIN, HIGH);       // turn on pullup resistors
  pinMode(KEY_LEFT_PIN, INPUT);           // set pin to input
  digitalWrite(KEY_LEFT_PIN, HIGH);       // turn on pullup resistors

// ------ valves -------
  pinMode(VALVE_IN_PIN, OUTPUT);           // set pin to input
  digitalWrite(VALVE_IN_PIN, HIGH);       // turn on pullup resistors
  pinMode(VALVE_OUT_PIN, OUTPUT);           // set pin to input
  digitalWrite(VALVE_OUT_PIN, HIGH);       // turn on pullup resistors

  tm_key_sampling = millis();
}
#endif

#ifndef VENTSIM
static void testKey()
{
  #if 1
  digitalWrite(VALVE_IN_PIN, digitalRead(KEY_FUNC_PIN));
  #else
    digitalWrite(VALVE_IN_PIN, digitalRead(KEY_LEFT_PIN));
  #endif
  digitalWrite(VALVE_OUT_PIN, digitalRead(KEY_RIGHT_PIN));
}
#endif

void halBlinkLED()
{
  //testKey();
    if (tm_led + 1000 < millis()) {
        tm_led = millis();

        
#ifdef VENTSIM
        LOG((char *) "LED Toggle");
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

//-------- display --------
#ifdef VENTSIM
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
#else
static void lcdUpdate()
{
    int i,r;
    char *s, *d;
    char out[LCD_NUM_COLS + 1];

    for (r=0; r<LCD_NUM_ROWS; r++) {
        d = out;
        s = &lcdBuffer [r][0];
        for (i=0; i<LCD_NUM_COLS; i++) {
            *d++ = *s++;
        }
        *d++ = 0;
        lcd.setCursor(0,r);
        lcd.print(out);
    }
}
#endif

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

//---------- valves Real
#ifndef VENTSIM
void halValveInOn()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_IN_PIN, LOW);
#else
    digitalWrite(VALVE_IN_PIN, HIGH);
#endif
}
void halValveInOff()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_IN_PIN, HIGH);
#else
    digitalWrite(VALVE_IN_PIN, LOW);
#endif
}
void halValveOutOn()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_OUT_PIN, LOW);
#else
    digitalWrite(VALVE_OUT_PIN, HIGH);
#endif
}
void halValveOutOff()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_OUT_PIN, HIGH);
#else
    digitalWrite(VALVE_OUT_PIN, LOW);
#endif
}
#else
// ----------- sim -------------
void halValveInOn()
{
  LOG(">>>>>> Valve IN ON");
}
void halValveInOff()
{
    LOG(">>>>>> Valve IN OFF");
}
void halValveOutOn()
{
  LOG("<<<<<<<< Valve OUT ON");
}
void halValveOutOff()
{
  LOG("<<<<<<<< Valve OUT OFF");
}
#endif

//---------------- process keys ----------
#define   DEBOUNCING_N    4
typedef struct keys_st {
  int state; // 0-> released
  int count;
  int pin;
  int keyCode;
} keys_t;

static keys_t keys[3] = {
  {0, 0, KEY_FUNC_PIN, KEY_FUNCTION},
  {0, 0, KEY_LEFT_PIN, KEY_LEFT},
  {0, 0, KEY_RIGHT_PIN, KEY_RIGHT},  
};

static void processKeys()
{
#ifndef VENTSIM
    if (tm_key_sampling + TM_KEY_SAMPLING < millis()) {
        tm_key_sampling = millis();

        if (digitalRead(keys[0].pin) == LOW) {
            evtPost(EVT_KEY_PRESS, keys[0].keyCode);
            evtPost(EVT_KEY_RELEASE, keys[0].keyCode);
        }
    }
      

    int i;
    if (tm_key_sampling + TM_KEY_SAMPLING < millis()) {
        tm_key_sampling = millis();
      for (i=0; i<3; i++) {
        if (keys[i].state == 0) {
          // ------- key is release state -------
          if (digitalRead(keys[i].pin) == LOW) { // if key is pressed
            keys[i].count++;
            if (keys[i].count >= DEBOUNCING_N) {
              //declare key pressed
              keys[i].count = 0;
              keys[i].state = 1;
              evtPost(EVT_KEY_PRESS, keys[i].keyCode);
            }

          }
          else {
              keys[i].count = 0;
          }
        }
        else {
          // ------- key is pressed state -------
           if (digitalRead(keys[i].pin) == HIGH) { // if key is release
            keys[i].count++;
            if (keys[i].count >= DEBOUNCING_N) {
              //declare key released
              keys[i].count = 0;
              keys[i].state = 0;
              evtPost(EVT_KEY_RELEASE, keys[i].keyCode);
            }
          }
          else {
              keys[i].count = 0;
          }       
        }
      }
        
    }
#endif
}

void halLoop()
{
  halBlinkLED();
  processKeys();
}


 
