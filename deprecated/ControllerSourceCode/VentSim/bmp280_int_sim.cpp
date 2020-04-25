

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
#include "config.h"
#include <stdint.h>

//--------- special return values for bpm280GetPressure and getCmH2OGauge
#define BMP_ST__INITIALIZING   -100.0f  // initializing
#define BMP_ST__NOT_FOUND      -200.0f  // could not get initialized
#define BMP_ST__READ_ERROR     -300.0f  // was initialized but fail to read at some point


void  bpm280Init()
{

}

float bpm280GetPressure() // absolute pressure in Pa, ex:  101982.90
{
    return 101982.90;
}

void  bmp280SetReference()
{

}
float getCmH2OGauge()
{
    return 10.0;
}


