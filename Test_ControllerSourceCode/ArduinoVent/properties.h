#ifndef PROPS_H
#define PROPS_H

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

#define PROT_DUTY_CYCLE_SIZE        4
extern const char * propDutyCycleTxt[PROT_DUTY_CYCLE_SIZE];

void propInit();
void propLoop();

bool propSave();

// ---------- Setters ------------
void propSetVent(int val);
void propSetBps(int val);
void propSetDutyCycle(int val);
void propSetPause(int val);
void propSetLcdAutoOff(int val);
void propSetBle(int val);

// ---------- Getters ------------
int propGetVent();
int propGetBps();
int propGetDutyCycle();
int propGetPause();
int propGetLcdAutoOff();
int propGetBle();

#endif // PROPS_H
