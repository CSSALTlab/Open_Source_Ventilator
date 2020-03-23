
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
#include "properties.h"
#include <stdio.h>
#include <string.h>

#define TM_BLINK                    400   // milliseconds
#define TM_FUNC_HOLD                500  // twoseconds
#define BLINK_PARAMETER_VAL         1
#define BLINK_SATUS                 2

static int params_idx = 0;

#define LCD_STATUS_ROW              0
#define LCD_PARAMS_FIRST_ROW        1
#define LCD_PARAMS_LAST_ROW         3
#define LCD_PARAMS_NUM_ROWS         ( (LCD_PARAMS_LAST_ROW - LCD_PARAMS_FIRST_ROW) + 1)

typedef enum {
    SHOW_MODE = 0,
    ENTER_MODE,
} UI_STATE_T;

static UI_STATE_T ui_state = SHOW_MODE;
static unsigned long tm_func_hold;
static bool check_func_hold = false;
static int ignore_release = 0;

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

typedef void (*propchancefunc_t)(int);

typedef struct params_st {
    p_type_t        type;
    const char *    name;
    int             val;
    int             step;
    int             min;
    int             max;
    const char **   options;
    bool            quickUpdate;
    propchancefunc_t handler;

} params_t;

//static const char * propDutyCycleTxt[] = {
//    "  1:1",
//    "  1:2",
//    "  1:3",
//    "  1:4"
//};

static void handleChangeVent(int val) {
    propSetVent(val);
}

static void handleChangeBps(int val) {
    propSetBps(val);
}

static void handleChangeDutyCycle(int val) {
     propSetDutyCycle(val);
}

static void handleChangePause(int val) {
     propSetPause(val);
}

static void handleChangeLcdAutoOff(int val) {
     propSetLcdAutoOff(val);
}

static void handleChangeBle(int val) {
      propSetBle(val);
}

static const char * onOffTxt[] = {
     "  off",
     "   on",
};

static params_t params[] = {
    { PARAM_TXT_OPTIONS,        // type
      "Ventilator",              // name
      0,                        // val
      1,                        // step
      0,                        // min
      1,                        // max
      onOffTxt ,                // text array for options
      false,                    // no dynamic changes
      &handleChangeVent           // change prop function
    },
    { PARAM_INT,                // type
      "BPM",                    // name
      10,                       // val
      5,                        // step
      10,                       // min
      30,                       // max
      0,                        // text array for options
      true,                     // no dynamic changes
      &handleChangeBps            // change prop function
    },

    { PARAM_TXT_OPTIONS,        // type
      "Duty Cyc.",              // name
      0,                        // val
      1,                        // step
      0,                        // min
      3,                        // max
      propDutyCycleTxt,               // text array for options
      true,                     // no dynamic changes
      &handleChangeDutyCycle      // change prop function
    },

    { PARAM_INT,                // type
      "Pause (ms)",             // name
      200,                      // val
      50,                       // step
      0,                        // min
      2000,                     // max
      0,                        // text array for options
      true,                     // no dynamic changes
      &handleChangePause          // change prop function
    },
    { PARAM_TXT_OPTIONS,        // type
      "LCD auto-off",           // name
      0,                        // val
      1,                        // step
      0,                        // min
      1,                        // max
      onOffTxt,                 // text array for options
      false,                    // no dynamic changes
      &handleChangeLcdAutoOff     // change prop function
    },
    { PARAM_TXT_OPTIONS,        // type
      "Bluetooth",              // name
      0,                        // val
      1,                        // step
      0,                        // min
      1,                        // max
      onOffTxt,                 // text array for options
      false,                    // no dynamic changes
      &handleChangeBle            // change prop function
    },
};

#define NUM_PARAMS (sizeof(params)/sizeof(params_t))

//------------ Global -----------
CUiNative::CUiNative()
{
    tm_blink = millis();
    updateStatus();
    updateParams();
//    char buf[32];
//    sprintf(buf,"size of tm_blink = %d\n", sizeof(tm_blink));
//    LOG(buf);
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

static void fillValBuf(char * buf, int idx)
{
    if (params[idx].type == PARAM_INT)
        sprintf(buf, "%5d", params[idx].val);
    else if (params[idx].type == PARAM_TXT_OPTIONS)
        strcpy(buf, params[idx].options[ params[idx].val ]);
    else
        LOG("blinker: Unexpected type");
}


void CUiNative::loop()
{
    checkFuncHold();
    blinker();
}

void CUiNative::blinker()
{   
    char buf[(LCD_NUM_COLS - PARAM_VAL_START_COL) + 1];
    int len = LCD_NUM_COLS - PARAM_VAL_START_COL;
    memset(buf, 0x20, (size_t) len); // spaces
    buf[len] = 0; // NULL terminate

    if (tm_blink + TM_BLINK < millis()) {
        tm_blink = millis();

        if (blink_mask == 0) return;

        blink_phase++;
        blink_phase &= 1;

        if (blink_mask & BLINK_PARAMETER_VAL) {
            if (blink_phase) {
                fillValBuf(buf, params_idx);
            }
            halLcdWrite(PARAM_VAL_START_COL, LCD_PARAMS_FIRST_ROW, buf);
        }
    }
}

void CUiNative::refreshValue(bool force)
{
    char buf[(LCD_NUM_COLS - PARAM_VAL_START_COL) + 1];
    int len = LCD_NUM_COLS - PARAM_VAL_START_COL;

    if (params[params_idx].quickUpdate || force) {
        params[params_idx].handler(params[params_idx].val);
    }

    memset(buf, 0x20, (size_t) len); // spaces
    buf[len] = 0; // NULL terminate

    tm_blink = millis();
    fillValBuf(buf, params_idx);
    halLcdWrite(PARAM_VAL_START_COL, LCD_PARAMS_FIRST_ROW, buf);
}


void CUiNative::updateParameterValue()
{

}

void CUiNative::blinkOn(int mask)
{
    blink_mask |= mask;
}
void CUiNative::blinkOff(int mask)
{
    blink_mask &= ~mask;
    updateParams();
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
      fillValBuf(&buf[PARAM_VAL_START_COL], idx);
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

void CUiNative::checkFuncHold()
{
    if (check_func_hold == false || ui_state != SHOW_MODE) {
        return;
    }
    if (tm_func_hold + TM_FUNC_HOLD < millis()) {
      blinkOn(BLINK_PARAMETER_VAL);
      LOG("** ENTER mode");
      ui_state = ENTER_MODE;

    }
}

propagate_t CUiNative::onEvent(event_t * event)
{
    char b[64];
    //sprintf(b, "onEvent: type = %d, key = %d\n", event->type, event->iParam);
    //LOG( (char *) b);


    //============== SHOW Mode =============
    if (ui_state == SHOW_MODE) {

        //--------- Function Key -------------
        if (event->iParam == KEY_FUNCTION)  {
            if (event->type == EVT_KEY_RELEASE)  {
                if (ignore_release) {
                    ignore_release--;
                    return PROPAGATE;
                }
                scroolParams();
                check_func_hold = false;
            }
            else if (event->type == EVT_KEY_PRESS) {
                tm_func_hold = millis();
                check_func_hold = true;
            }
        }
    }

    //============== ENTER Mode =============
    else if (ui_state == ENTER_MODE) {

        //--------- Function Key -------------
        if (event->iParam == KEY_FUNCTION && event->type == EVT_KEY_PRESS)  {
            blinkOff(BLINK_PARAMETER_VAL);
            LOG("** SHOW mode");
            ui_state = SHOW_MODE;
            check_func_hold = false;
            ignore_release = 1;
            refreshValue(true);
        }

        //------- Right (UP) Key
        if (event->iParam == KEY_RIGHT && event->type == EVT_KEY_PRESS)  {
          if (params[params_idx].val < params[params_idx].max) {
              params[params_idx].val += params[params_idx].step;
              refreshValue(false);
          }
        }
        if (event->iParam == KEY_LEFT && event->type == EVT_KEY_PRESS)  {
          if (params[params_idx].val > params[params_idx].min) {
              params[params_idx].val -= params[params_idx].step;
              refreshValue(false);
          }
        }
    }

    return PROPAGATE;
}
