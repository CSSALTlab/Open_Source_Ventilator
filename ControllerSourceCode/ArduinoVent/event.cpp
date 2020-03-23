
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
   

//----------- Locals -------------


#define QUEUE_SIZE 6
#define NUM_MAX_LISNERS 4

extern void LOG(char * txt);

//------------ Global -----------
 static event_t eventQ[QUEUE_SIZE];
 static int eventQIdxIn = 0;
 static int eventQIdxOut = 0;
 static int eventQIdxCount = 0;

 static CEvent * lisners[NUM_MAX_LISNERS];
 static int num_lisners = 0;

 void evtPostEx(  EVENT_TYPE type,
                  int iParam,
                  unsigned long long lParam,
                  char * tParam)
 {
     if (eventQIdxCount >= NUM_MAX_LISNERS ) {
         LOG("critical error: Event queue full");
         return;
     }
     event_t * e = &eventQ[eventQIdxIn++];
     eventQIdxCount++;
     if (eventQIdxIn >= QUEUE_SIZE) eventQIdxIn = 0;

     e->type = type;
     e->iParam = iParam;
     e->lParam = lParam;
     if (tParam != 0) {
         memcpy(e->tParam, tParam, TEXT_PARAM_SIZE);
     }
     else {
         memset(e->tParam, 0, TEXT_PARAM_SIZE);
     }
 }

 void evtPost(  EVENT_TYPE type,
                int iParam)
 {
     evtPostEx( type,
                iParam,
                0,
                0);
 }

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

CEvent::~CEvent()
{

}

propagate_t CEvent::onEvent(event_t * event)
{
    return PROPAGATE;
}
