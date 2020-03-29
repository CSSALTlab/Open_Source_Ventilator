
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
#include "breather.h"
#include <stdio.h>
#include <string.h>
#include "config.h"

//#define TEST_WDT // Debug only... it makes Watchdor to trigger reset when Set button is pressed

#define TM_BLINK                    400   // milliseconds
#define TM_FUNC_HOLD                500  // twoseconds
#define BLINK_PARAMETER_VAL         1
#define BLINK_SATUS                 2

static int params_idx = 0;

#define LCD_STATUS_ROW              0
#define LCD_PARAMS_FIRST_ROW        1

#if (LCD_CFG_2_ROWS == 1)
  #define LCD_PARAMS_LAST_ROW         1
#elif (LCD_CFG_4_ROWS == 1)
  #define LCD_PARAMS_LAST_ROW         3
#else
  #error "At least one LCD_CFG_x_ROWS must be set to 1 in config.h"
#endif

#define LCD_PARAMS_NUM_ROWS         ( (LCD_PARAMS_LAST_ROW - LCD_PARAMS_FIRST_ROW) + 1)

#define PROGRESS_ROW        0
#define PROGRESS_NUM_CHARS  6
#define PROGRESS_COL       14
#define PROGRESS_CHARACTER '|'

static int progress = 0;

typedef enum {
    SHOW_MODE = 0,
    ENTER_MODE,
} UI_STATE_T;

static UI_STATE_T ui_state = SHOW_MODE;
static unsigned long tm_set_hold;
static bool check_set_hold = false;
static unsigned long tm_decrement_hold;
static bool check_decrement_hold = false;
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
typedef int (*propgetfunc_t)();

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
    propgetfunc_t   propGetter;

} params_t;


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

static void handleSave(int val); // prototype
//---------------

static int handleGetVent() {
    return propGetVent();
}

static int handleGetBps() {
    return propGetBps();
}

static int handleGetDutyCycle() {
     return propGetDutyCycle();
}

static int handleGetPause() {
     return propGetPause();
}

static int handleGetLcdAutoOff() {
     return propGetLcdAutoOff();
}

//static void handleChangeBle(int val) {
//      propSetBle(val);
//}

static const char * onOffTxt[] = {
     "  off",
     "   on",
};

static const char * yesNoTxt[] = {
     "   no",
     "  yes",
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
      &handleChangeVent,        // change prop function
      &handleGetVent            // propGetter
    },
    { PARAM_INT,                // type
      "BPM",                    // name
      10,                       // val
      5,                        // step
      10,                       // min
      30,                       // max
      0,                        // text array for options
      true,                     // no dynamic changes
      &handleChangeBps,          // change prop function
      &handleGetBps             // propGetter
    },

    { PARAM_TXT_OPTIONS,        // type
      "Duty Cyc.",              // name
      0,                        // val
      1,                        // step
      0,                        // min
      3,                        // max
      propDutyCycleTxt,         // text array for options
      true,                     // no dynamic changes
      &handleChangeDutyCycle,   // change prop function
      &handleGetDutyCycle       // propGetter
    },

    { PARAM_INT,                // type
      "Pause (ms)",             // name
      200,                      // val
      50,                       // step
      0,                        // min
      2000,                     // max
      0,                        // text array for options
      true,                     // no dynamic changes
      &handleChangePause,       // change prop function
      &handleGetPause           // propGetter
    },
    { PARAM_TXT_OPTIONS,        // type
      "LCD auto-off",           // name
      0,                        // val
      1,                        // step
      0,                        // min
      1,                        // max
      onOffTxt,                 // text array for options
      false,                    // no dynamic changes
      &handleChangeLcdAutoOff,  // change prop function
      &handleGetLcdAutoOff      // propGetter
    },
//    { PARAM_TXT_OPTIONS,        // type
//      "Bluetooth",              // name
//      0,                        // val
//      1,                        // step
//      0,                        // min
//      1,                        // max
//      onOffTxt,                 // text array for options
//      false,                    // no dynamic changes
//      &handleChangeBle          // change prop function
//      &handleGetBle             // propGetter
//    },


    //---- This Must be the very last parameter ----
    { PARAM_TXT_OPTIONS,        // type
      "Save (now)",             // name
      0,                        // val
      1,                        // step
      0,                        // min
      1,                        // max
      yesNoTxt,                 // text array for options
      false,                    // no dynamic changes
      &handleSave,              // change prop function
      0
    },
};

#define NUM_PARAMS (sizeof(params)/sizeof(params_t))

//------------ Global -----------
CUiNative::CUiNative()
{
    initParams();
    tm_blink = halStartTimerRef();
    updateStatus();
    updateParams();
//    char buf[32];
//    sprintf(buf,"size of tm_blink = %d\n", sizeof(tm_blink));
//    LOG(buf);

}

CUiNative::~CUiNative()
{

}

void CUiNative::initParams()
{
  int i;
  for (i=0; i<NUM_PARAMS; i++) {
    if (params[i].propGetter) {
      params[i].val = params[i].propGetter();
    }
  }
}

static const char * st_txt[3] = {
    (const char *) "idle",
    (const char *) "run ",
    (const char *) "Err "
};

#define PARAM_VAL_START_COL 15

static void handleSave(int val) {
    if (val) {
        propSave();
        params[NUM_PARAMS-1].val = 0; // sets back to off
    }
}


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
    updateProgress();
}

void CUiNative::blinker()
{   
    char buf[(LCD_NUM_COLS - PARAM_VAL_START_COL) + 1];
    int len = LCD_NUM_COLS - PARAM_VAL_START_COL;
    memset(buf, 0x20, (size_t) len); // spaces
    buf[len] = 0; // NULL terminate

    if (halCheckTimerExpired(tm_blink, TM_BLINK)) {
        tm_blink = halStartTimerRef();

        if (blink_mask == 0) return;

        blink_phase++;
        blink_phase &= 1;

        //-------- paramater Blinking ------------
        if (blink_mask & BLINK_PARAMETER_VAL) {
            if (blink_phase) {
                fillValBuf(buf, params_idx);
            }
            halLcdWrite(PARAM_VAL_START_COL, LCD_PARAMS_FIRST_ROW, buf);
        }

        //-------- other Blinking... ------------
        //if (blink_mask & WHATEVER) {
        //}

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

    tm_blink = halStartTimerRef();
    fillValBuf(buf, params_idx);
    halLcdWrite(PARAM_VAL_START_COL, LCD_PARAMS_FIRST_ROW, buf);
}

void CUiNative::updateProgress()
{
    int i;
    char buf[PROGRESS_NUM_CHARS+1];
    memset(buf, 0x20, sizeof (buf)); // spaces
    buf[sizeof (buf) - 1] = 0;

    B_STATE_t s = breatherGetState();
    int p = (breatherGetPropress() * PROGRESS_NUM_CHARS) / 90;
    if (p > PROGRESS_NUM_CHARS) p = PROGRESS_NUM_CHARS;
    if (p == progress)
        return;

    progress = p;
    for(i=0; i<progress; i++) {
        buf[i] = PROGRESS_CHARACTER;
    }
    halLcdWrite(PROGRESS_COL, PROGRESS_ROW, buf);
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

void CUiNative::scroolParams( bool down)
{
    if (down) {
      params_idx++;
      if ( params_idx >= NUM_PARAMS)
          params_idx = 0;
    }
    else {
      params_idx--;
      if (params_idx < 0)
          params_idx = NUM_PARAMS - 1;
    }

    updateParams();
}

void CUiNative::checkFuncHold()
{
    if (ui_state != SHOW_MODE) {
        return;
    }

    //-------- process KEY_SET hold ------
    if (check_set_hold) {
      if (halCheckTimerExpired(tm_set_hold, TM_FUNC_HOLD)) {
        blinkOn(BLINK_PARAMETER_VAL);
        LOG("** ENTER mode");
        ui_state = ENTER_MODE;
      }
    }

    //-------- process KEY_DECREMENT hold ------
    if (check_decrement_hold) {
      if (halCheckTimerExpired(tm_decrement_hold, TM_FUNC_HOLD)) {
        params_idx = -1;
        scroolParams(true);
      }
    }
}

propagate_t CUiNative::onEvent(event_t * event)
{
    char b[64];
    //sprintf(b, "onEvent: type = %d, key = %d\n", event->type, event->iParam);
    //LOG( (char *) b);


    //============== SHOW Mode =============
    if (ui_state == SHOW_MODE) {

        //--------- SET Key -------------
        if (event->iParam == KEY_SET)  {
            if (event->type == EVT_KEY_RELEASE)  {
                if (ignore_release) {
                    ignore_release--;
                    return PROPAGATE;
                }
                // scroolParams(true);
                check_set_hold = false;
            }
            else if (event->type == EVT_KEY_PRESS) {
                tm_set_hold = halStartTimerRef();
                check_set_hold = true;
#ifdef TEST_WDT
                // test WDT
                delay(2000); // triggers reset via WDT
#endif
            }
        }

        //------------- Decrement key --------------
        if (event->iParam == KEY_DECREMENT) {
            if (event->type == EVT_KEY_PRESS)  {
              scroolParams(false);
              tm_decrement_hold = halStartTimerRef();
              check_decrement_hold = true;
            }
            else {
                check_decrement_hold = false;
            }
        }

        //------------- increment key --------------
        if (event->iParam == KEY_INCREMENT && event->type == EVT_KEY_PRESS)  {
            scroolParams(true);
        }
    }

    //============== ENTER Mode =============
    else if (ui_state == ENTER_MODE) {

        //--------- Function Key -------------
        if (event->iParam == KEY_SET && event->type == EVT_KEY_PRESS)  {
            blinkOff(BLINK_PARAMETER_VAL);
            LOG("** SHOW mode");
            ui_state = SHOW_MODE;
            check_set_hold = false;
            ignore_release = 1;
            refreshValue(true);
        }

        //------- Right (UP) Key
        if (event->iParam == KEY_INCREMENT && event->type == EVT_KEY_PRESS)  {
          if (params[params_idx].val < params[params_idx].max) {
              params[params_idx].val += params[params_idx].step;
              refreshValue(false);
          }
        }
        if (event->iParam == KEY_DECREMENT && event->type == EVT_KEY_PRESS)  {
          if (params[params_idx].val > params[params_idx].min) {
              params[params_idx].val -= params[params_idx].step;
              refreshValue(false);
          }
        }
    }

    return PROPAGATE;
}
