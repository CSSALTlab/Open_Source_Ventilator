
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

static Alarm * alarm;

void alarmInit()
{
    alarm = new Alarm();
}

void alarmLoop()
{

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
    // beep();
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
