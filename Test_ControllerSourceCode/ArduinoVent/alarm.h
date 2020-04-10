#ifndef ALARM_H
#define ALARM_H

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
#include "event.h"


void alarmInit();
void alarmLoop();
void alarmResetAll();

class Alarm : CEvent {

public:
    Alarm(void);
    ~Alarm();

    void Loop();
    virtual propagate_t onEvent(event_t * event);

    void setNextAlarmIfAny();
    void internalAlarmResetAll();

private:

    //void beep();
    void beepOnOff(bool on);
    void muteAlarmIfOn();

    //--- variables
    bool beepIsOn = false;
    int8_t activeAlarmIdx = -1;


};

//----------------------------------------------------
#endif // ALARM_H
