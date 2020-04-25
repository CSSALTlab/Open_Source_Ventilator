#ifndef PRESSURE_H
#define PRESSURE_H

/*************************************************************
 * Open Ventilator
 * Copyright (C) 2020 - Dr. Bill Schmidt-J68HZ, Jack Purdum and Marcelo Varanda
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

#define AVERAGE_BIN_NUMBER        6        // Number of averaging bins for the averaging routine
#define PRESSURE_READ_DELAY       20L       // wait 20 ms between reads

typedef enum {
  PRESSURE,
  FLOW,
  
  NUM_P_SENSORS // must be the last one
} psensor_t;

void pressInit();
void pressLoop();

float pressGetFloatVal(psensor_t sensor); // in InchH2O
int32_t pressGetRawVal(psensor_t sensor);

#endif // PRESSURE_H
