
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


#include "alarm.h"
#include "log.h"
#include "hal.h"

// TODO: move
#define ALARM_MSG_HIGH_PRESSURE " High Pressure"
#define ALARM_MSG_LOW_PRESSURE  " Low Pressure"

#define ALARM_IDX_HIGH_PRESSURE     0 // index for high pressure alarm in alarms array
#define ALARM_IDX_LOW_PRESSURE      1 // index for low pressure alarm in alarms array

typedef enum : uint8_t {
    ST_NO_ALARM,
    ST_BEEPING,
    ST_MUTED,
    ST_IGNORED
} state_t;

typedef void (*muteFunc_t)(void);
typedef void (*goOffFunc_t)(void);

static bool beepIsOn = false;
static int8_t activeAlarmIdx = -1;

typedef struct alarm_st {
    state_t     state;
    uint8_t     cnt;         // num of times muted by operator (before start ignoring it)
    char *      message;
    goOffFunc_t goOffAction;
    muteFunc_t  muteAction;
} alarm_t;


void muteHighPressureAlarm()
{

}

void muteLowPressureAlarm()
{

}

static alarm_t alarms[] = {
  {
        ST_NO_ALARM,
        0,
        ALARM_MSG_HIGH_PRESSURE,
        0,
        muteHighPressureAlarm
  },

  {
        ST_NO_ALARM,
        0,
        ALARM_MSG_LOW_PRESSURE,
        0,
        muteLowPressureAlarm
  }

};


static Alarm * alarm;

void alarmInit()
{
    alarm = new Alarm();
}

void alarmLoop()
{

}

static void beepOnOff(bool on)
{
    if (on) {
        if (beepIsOn == false) {
            beepIsOn = true;
            halBeepAlarmOnOff(true);
        }
    }
    else {
        if (beepIsOn == true) {
            beepIsOn = false;
            halBeepAlarmOnOff(false);
        }
    }
}

static void processAlarmEvent(alarm_t * a)
{
    switch (a->state) {
      case ST_IGNORED:
        LOG("ignore alarm");
        break;

      case ST_NO_ALARM:
        a->state = ST_BEEPING;
        beepOnOff(true);
        break;

      case ST_BEEPING:
        LOG("already beeping state");

      case ST_MUTED:
      default:
        break;

    }
}


Alarm::Alarm ()
{
#ifdef VENTSIM
    player.setMedia(QUrl("qrc:/sound/Resource/tom_1s.mp3"));
    player.setVolume(80);
#endif
}

void Alarm::Loop()
{

}

propagate_t Alarm::onEvent(event_t * event)
{
    alarm_t * a;
    // beep();
    switch (event->type) {

      case EVT_ALARM_LOW_PRESSURE:
        a = &alarms[ALARM_IDX_LOW_PRESSURE];
        processAlarmEvent(a);
        break;

      case EVT_ALARM_HIGH_PRESSURE:
        a = &alarms[ALARM_IDX_HIGH_PRESSURE];
        processAlarmEvent(a);
        break;

      case EVT_KEY_PRESS:
      case EVT_KEY_RELEASE:
        break;

      default:
        return PROPAGATE;

    }

    return PROPAGATE;
}

Alarm::~Alarm()
{

}

void Alarm::beep()
{
#ifdef VENTSIM
    player.play();
#endif

}
