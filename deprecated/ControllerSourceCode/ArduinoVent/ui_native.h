#ifndef UI_NATIVE_H
#define UI_NATIVE_H

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
#else
  #include <Arduino.h> 
#endif

#include "config.h"
#include "event.h"

void uiNativeInit();
void uiNativeLoop();

typedef enum {
    SHOW_MODE = 0,
    ENTER_MODE,
} UI_STATE_T;

typedef  enum {
    STATE_IDLE = 0,
    STATE_RUN,
    STATE_ERROR
} RUN_STATE_T;

class CUiNative : CEvent {
public:
    CUiNative();
    ~CUiNative();
    void loop();
    //void updateStatus();
    void updateParams();
    void updateParameterValue();
    void scroolParams(bool down);
    void blinker();
    void blinkOff(int mask);
    void blinkOn(int mask);
    void refreshValue(bool force);
    void checkFuncHold ();
    void updateProgress();
    void initParams();
    void fillValBuf(char * buf, int idx);
    void updateStatus(bool blank);

    virtual propagate_t onEvent(event_t * event);

    // --- public var (neede for static function) ---
    RUN_STATE_T state_idx; // = STATE_IDLE;

 private:
    int params_idx;
    int progress;

    UI_STATE_T ui_state; // = SHOW_MODE;
    unsigned long tm_set_hold;
    bool check_set_hold; // = false;
    unsigned long tm_decrement_hold;
    bool check_decrement_hold; // = false;
    bool shortcut_to_top_done;
    int ignore_release; // = 0;

    int blink_mask; // = 0;
    unsigned long tm_blink;
    int blink_phase; // = 0;

    bool alarm_mode; // = false;
    char alarm_msg[LCD_NUM_COLS+1];

    int bps; // = 10;
    float dutyCycle; // = 0.1f;

};

#endif // UI_NATIVE_H
