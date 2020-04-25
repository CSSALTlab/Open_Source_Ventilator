#ifndef EVENT_H
#define EVENT_H

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

#include <stdint.h>
#define TEXT_PARAM_SIZE 21

enum  {
    KEY_DECREMENT,
    KEY_INCREMENT,
    KEY_SET,

    KEY_RESERVE
};


typedef enum  {
    EVT_KEY_PRESS,
    EVT_KEY_RELEASE,

    EVT_ALARM,

    EVT_ALARM_DISPLAY_ON,
    EVT_ALARM_DISPLAY_OFF,

} EVENT_TYPE;

typedef  enum {
    PROPAGATE = 0,
    PROPAGATE_STOP,
} propagate_t;

typedef struct event_st {
    EVENT_TYPE type;
    union {
        int iParam;
        unsigned long long lParam;
        char tParam[TEXT_PARAM_SIZE];
    } param;
} event_t;


void evtDispatchAll();

class CEvent {

public:
    CEvent();
    ~CEvent();

    static void post( EVENT_TYPE type,
                      int iParam);

    static void post( EVENT_TYPE type,
                      uint64_t lParam);

    static void post( EVENT_TYPE type,
                      char * tParam);

    virtual propagate_t onEvent(event_t * event);

private:
    static void post (event_t * event);
};


#endif // EVENT_H
