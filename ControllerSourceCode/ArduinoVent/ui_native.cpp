
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


#include "ui_native.h"
#include "hal.h"
#include <stdio.h>
#include <string.h>

#define TM_BLINK                    400  // milliseconds
#define BLINK_PARAMETER_VAL         1
#define BLINK_SATUS                 2

//----------- Locals -------------

static int state_idx = 0;
static int blink_mask = 0;
static unsigned long tm_blink;
static int blink_phase = 0;

static int bps = 10;
static float dutyCycle = 0.1f;

typedef enum {
    PARAM_INT = 1,
    PARAM_TXT_OPTIONS,

    PARAM_END

} p_type_t;

typedef struct params_st {
    p_type_t        type;
    const char *    name;
    int             val;
    int             step;
    int             min;
    int             max;
    const char **   options;
} params_t;

static const char * dutycycles[] = {
    "  1:1",
    "  1:2",
    "  1:3",
    "  1:4",
    0
};

static const  params_t params[] = {
    { PARAM_INT,                // type
      "BPM",                    // name
      10,                       // val
      2,                        // step
      10,                       // min
      30,                       // max
      0                         // text array for options
    },

    { PARAM_INT,                // type
      "Duty Cyc.",              // name
      0,                        // val
      1,                        // step
      0,                        // min
      3,                        // max
      dutycycles                // text array for options
    },

    { PARAM_INT,                // type
      "Pause (ms)",             // name
      200,                      // val
      50,                       // step
      0,                        // min
      2000,                     // max
      0                         // text array for options
    },

};

#define NUM_PARAMS (sizeof(params)/sizeof(params_t)) 

static int params_idx = 0;

#define LCD_STATUS_ROW              0
#define LCD_PARAMS_FIRST_ROW        1
#define LCD_PARAMS_LAST_ROW         3
#define LCD_PARAMS_NUM_ROWS         ( (LCD_PARAMS_LAST_ROW - LCD_PARAMS_FIRST_ROW) + 1)

//------------ Global -----------
CUiNative::CUiNative()
{
    tm_blink = millis();
    updateStatus();
    updateParams();
}

CUiNative::~CUiNative()
{

}

static const char * st_txt[3] = {
    (const char *) "idle",
    (const char *) "run ",
    (const char *) "Err "
};

#define PARAM_VAL_START_COL 15

void CUiNative::blinker()
{
    char buf[(LCD_NUM_COLS - PARAM_VAL_START_COL) + 1];
    int len = LCD_NUM_COLS - PARAM_VAL_START_COL;
    memset(buf, 0x20, (size_t) len); // spaces
    buf[len] = 0; // NULL terminate

    if (tm_blink + TM_BLINK < millis()) {
        tm_blink = millis();
        blink_phase++;
        blink_phase &= 1;

        if (blink_phase) {
            sprintf(buf, "%5d", params[params_idx].val);
        }
        else {

        }
        halLcdWrite(PARAM_VAL_START_COL, LCD_PARAMS_FIRST_ROW, buf);
    }


}

void CUiNative::updateParameterValue()
{

}

void CUiNative::blinkOn(int mask)
{
    blink_mask != mask;
}
void CUiNative::blinkOff(int mask)
{
    blink_mask != ~mask;
    updateParameterValue();
}

void CUiNative::updateStatus()
{
  char buf[LCD_NUM_COLS+1];
  memset(buf, 0x20, LCD_NUM_COLS);
  buf[LCD_NUM_COLS] = 0;
  int len = sprintf(buf, "st=%s Bt=%c", (const char *) st_txt[state_idx], 'X');

  buf[len] = 0x20;
  halLcdWrite(0, LCD_STATUS_ROW, buf);
}

void CUiNative::updateParams()
{
  int idx = params_idx;
  int i;
  char buf[LCD_NUM_COLS+1];

  for (i=0; i < LCD_PARAMS_NUM_ROWS; i++) {
      memset(buf, 0x20, LCD_NUM_COLS);
      buf[LCD_NUM_COLS] = 0;
      if (i == 0) {
        buf[0] = '>';
      }
      memcpy(&buf[1], params[idx].name, strlen(params[idx].name));
      halLcdWrite(0, LCD_PARAMS_FIRST_ROW + i, buf);

      idx++;
      if (idx >= NUM_PARAMS) idx = 0;
  }
}

void CUiNative::scroolParams()
{
    params_idx++;
    if ( params_idx >= NUM_PARAMS) params_idx = 0;
    updateParams();
}

 
propagate_t CUiNative::onEvent(event_t * event)
{
    char b[64];
    sprintf(b, "onEvent: type = %d, key = %d\n", event->type, event->iParam);
    LOG( (char *) b);

    if ( (event->type == EVT_KEY_PRESS) && (event->iParam == KEY_FUNCTION) ) {
        scroolParams();
    }

    return PROPAGATE;
}
