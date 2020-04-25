
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
#include <string.h>
#include "log.h"
   

//----------- Locals -------------


#define QUEUE_SIZE 6
#define NUM_MAX_LISNERS 4

//extern void LOG(const char * txt);

//------------ Global -----------
 static event_t eventQ[QUEUE_SIZE];
 static int eventQIdxIn = 0;
 static int eventQIdxOut = 0;
 static int eventQIdxCount = 0;

 static CEvent * lisners[NUM_MAX_LISNERS];
 static int num_lisners = 0;

 void evtDispatchAll()
 {
     int i;
     propagate_t ret;
     while (eventQIdxCount>0) {
         for (i=0; i<num_lisners; i++) {
             ret = lisners[i]->onEvent(&eventQ[eventQIdxOut]);
             if (ret == PROPAGATE_STOP)
                 break;
         }

         eventQIdxOut++;
         if (eventQIdxOut >= QUEUE_SIZE) eventQIdxOut = 0;
         eventQIdxCount--;
     }
 }

 CEvent::CEvent()
 {
     if (num_lisners < NUM_MAX_LISNERS) {
         lisners[num_lisners++] = this;
     }
     else {
         LOG("critical error, no room for CEvent");
     }
 }

 void CEvent::post( EVENT_TYPE type,
                    int iParam)
 {
    event_t e;
    e.type = type;
    e.param.iParam = iParam;
    post(&e);
 }

 void CEvent::post( EVENT_TYPE type,
                    uint64_t lParam)
 {
     event_t e;
     e.type = type;
     e.param.lParam = lParam;
     post(&e);
 }

 void CEvent::post( EVENT_TYPE type,
                    char * tParam)
 {
     event_t e;
     e.type = type;
     memcpy(e.param.tParam, tParam, sizeof(e.param.tParam));
     post(&e);
 }

 void CEvent::post (event_t * event)
 {
     if (eventQIdxCount >= NUM_MAX_LISNERS ) {
         LOG("critical error: Event queue full");
         return;
     }
     event_t * e = &eventQ[eventQIdxIn++];
     eventQIdxCount++;
     if (eventQIdxIn >= QUEUE_SIZE) eventQIdxIn = 0;

    *e = *event;
 }


CEvent::~CEvent()
{

}

propagate_t CEvent::onEvent(event_t * event)
{
    return PROPAGATE;
}
