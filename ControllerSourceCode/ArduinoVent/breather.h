#ifndef BREATHER_H
#define BREATHER_H

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
typedef enum {
    B_ST_STOPPED,
    B_ST_IN,
    B_ST_WAIT_TO_OUT,
    B_ST_OUT,
    B_ST_FAST_CALIB,
    B_ST_PAUSE,
    B_ST_STOPPING,
} B_STATE_t;

void breatherLoop();
void breatherStartCycle();
B_STATE_t breatherGetState();
int breatherGetPropress();
void breatherRequestFastCalibration();


#endif // BREATHER_H
