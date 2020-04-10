
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
#include "languages.h"

#define ALARM_IDX_HIGH_PRESSURE     0 // index for high pressure alarm in alarms array
#define ALARM_IDX_LOW_PRESSURE      1 // index for low pressure alarm in alarms array

#define NUM_ALARM_UNTIL_IGNORE 3

// #define SIM_HIGH_PRESSURE

typedef enum : uint8_t {
    ST_NO_ALARM,
    ST_ON,
    ST_MUTED,
    ST_IGNORED
} state_t;

typedef void (*muteFunc_t)(void);
typedef void (*goOffFunc_t)(void);

//static bool beepIsOn = false;
//static int8_t activeAlarmIdx = -1;

typedef struct alarm_st {
    state_t     state;
    uint8_t     cnt;         // num of times muted by operator (before start ignoring it)
    const char *  message;
    goOffFunc_t goOffAction;
    muteFunc_t  muteAction;
} alarm_t;

static Alarm * alarm;

void alarmResetAll()
{
    alarm->internalAlarmResetAll();
}

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
        STR_HIGH_PRESSURE,
        0,
        muteHighPressureAlarm
  },

  {
        ST_NO_ALARM,
        0,
        STR_LOW_PRESSURE,
        0,
        muteLowPressureAlarm
  }

};
#define NUM_ALARMS  sizeof(alarms) / sizeof(alarm_t)

void Alarm::beepOnOff(bool on)
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

void Alarm::internalAlarmResetAll()
{
    uint8_t i;
    alarm_t * a = alarms;
    activeAlarmIdx = -1;
    for (i=0; i< NUM_ALARMS; i++) {
        a->cnt = 0;
        a->state = ST_NO_ALARM;
        a++;
    }
    beepOnOff(false);
    CEvent::post(EVT_ALARM_DISPLAY_OFF,0);
}

void Alarm::setNextAlarmIfAny()
{
    uint8_t i;
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
            CEvent::post(EVT_ALARM_DISPLAY_ON, (char *) a->message);
            beepOnOff(true);
            return;
        }
    }
}

void Alarm::muteAlarmIfOn()
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
    CEvent::post(EVT_ALARM_DISPLAY_OFF, 0);
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
  if (a->state != ST_IGNORED) {
    a->state = ST_ON;
    alarm->setNextAlarmIfAny();
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
    int i;

    switch (event->type) {

      case EVT_ALARM:
        switch (event->param.iParam)
        {
          case EVT_ALARM_HIGH_PRESSURE: i = ALARM_IDX_HIGH_PRESSURE; break;
          case EVT_ALARM_LOW_PRESSURE: i = ALARM_IDX_LOW_PRESSURE; break;

          default: LOG("Alarm::onEvent: fix me"); return PROPAGATE;
        }
        a = &alarms[i];
        processAlarmEvent(a);
        break;

      case EVT_KEY_PRESS:
#ifdef SIM_HIGH_PRESSURE
        if (event->param.iParam == KEY_SET) {
          LOG("SIM High pressure Alarm event");
          CEvent::post(EVT_ALARM, EVT_ALARM_HIGH_PRESSURE);
        }
        else if (event->param.iParam == KEY_INCREMENT_PIN) {
          LOG("SIM Low pressure Alarm event");
          CEvent::post(EVT_ALARM, EVT_ALARM_LOW_PRESSURE);
        }
        else {
          LOG("mute event");
          muteAlarmIfOn();
          //return PROPAGATE_STOP;
        }
#else
        muteAlarmIfOn();
        return PROPAGATE;
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
