
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


//---------- Constants ---------
#define LCD_NUM_ROWS 4
#define LCD_NUM_COLS 20

//-------- variables --------
static unsigned long tm_led;

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
void LOG(char * txt) { /*dummy */} 
  
void halInit() {
  pinMode(LED_BUILTIN, OUTPUT);
  tm_led = millis();
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  halLcdClear();
}
#endif

void halBlinkLED()
{
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



 
