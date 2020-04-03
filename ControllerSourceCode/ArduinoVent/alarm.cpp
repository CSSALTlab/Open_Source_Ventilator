
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

#define NUM_ALARM_UNTIL_IGNORE 3

#define SIM_HIGH_PRESSURE

typedef enum : uint8_t {
    ST_NO_ALARM,
    ST_ON,
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
#define NUM_ALARMS  sizeof(alarms) / sizeof(alarm_t)

static Alarm * alarm;

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

static void setNextAlarmIfAny()
{
    int i;
    alarm_t * a;

    if (activeAlarmIdx >= 0) {
        // tolerates as there is already an alarm. mute will take care of calling this func once again
        return;
    }

    for (i=0; i< NUM_ALARMS; i++) {
        a = &alarms[i];

        if (a->state == ST_ON) {
            activeAlarmIdx = i;
            if (a->goOffAction) { // call an action if a callback was defined
                a->goOffAction();
            }
            CEvent::post(EVT_ALARM_DISPLAY_ON, a->message);
            beepOnOff(true);
            return;
        }
    }
}

static void muteAlarmIfOn()
{
    if (activeAlarmIdx < 0)
        return;

    beepOnOff(false);

    alarm_t * a = &alarms[activeAlarmIdx];

    if (a->state != ST_ON) {
        LOG("muteAlarmIfOn: unexpected state, fix me");
        return;
    }

    if (a->muteAction) { // call an action if a callback was defined
        a->muteAction();
    }
    a->state = ST_NO_ALARM;
    if (++(a->cnt) >= NUM_ALARM_UNTIL_IGNORE)
        a->state = ST_IGNORED;

    activeAlarmIdx = -1;
    setNextAlarmIfAny();
}


void alarmInit()
{
    alarm = new Alarm();
}

void alarmLoop()
{

}

static void processAlarmEvent(alarm_t * a)
{
//    switch (a->state) {
//      case ST_IGNORED:
//        LOG("ignore alarm");
//        break;

//      case ST_NO_ALARM:
//        a->state = ST_ON;
//        beepOnOff(true);
//        break;

//      case ST_BEEPING:
//        LOG("already beeping state");

//      case ST_MUTED:
//      default:
//        break;

//    }
  if (a->state != ST_IGNORED) {
    a->state = ST_ON;
    setNextAlarmIfAny();
  }
}


Alarm::Alarm ()
{

}

void Alarm::Loop()
{

}

propagate_t Alarm::onEvent(event_t * event)
{
    alarm_t * a;

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
#ifdef SIM_HIGH_PRESSURE
        if (event->param.iParam == KEY_SET) {
          LOG("SIM High pressure Alarm event");
          CEvent::post(EVT_ALARM_HIGH_PRESSURE, 0);
        }
        else if (event->param.iParam == KEY_INCREMENT_PIN) {
          LOG("SIM Low pressure Alarm event");
          CEvent::post(EVT_ALARM_LOW_PRESSURE, 0);
        }
        else {
          LOG("mute event");
          muteAlarmIfOn();
          //return PROPAGATE_STOP;
        }
#endif
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

