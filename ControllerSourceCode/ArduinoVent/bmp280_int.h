#ifndef BMP280_INT_H
#define BMP280_INT_H

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

/**
 * @file bmp280_int.h
 * @author Marcelo Varanda
 * @date April 2020
 *
 * @brief File containing pressure reading from Bosh BMP280 sensor.
 *
 * It uses a modified code from the original sSense-BMx280I2C library now present in this code as cpp files: 
 * sSense-BMx280I2C_mv and BME280_mv
 *
 */

#include "config.h"
#include <stdint.h>

//--------- special return values for bpm280GetPressure and getCmH2OGauge
#define BMP_ST__INITIALIZING   -100.0f  /**< Special returning value: Initializing */
#define BMP_ST__NOT_FOUND      -200.0f  /**< Special returning value: could not get initialized */ 
#define BMP_ST__READ_ERROR     -300.0f  /**< Special returning value: was initialized but fail to read */


/**
 * @brief Initiialize the BMP280 related components.
 *
 * This function should only be called once during system initialization
 * In case the initialization fails it will try again later during calls for bpm280GetPressure or getCmH2OGauge funtions.
 *
 * @param None
 * @return None
 */
void  bpm280Init();
                                                   
/**
 * @brief return the absolute pressure in Pa.
 *
 * regardless how fast this function is called its return value only changes every 50milliseconds.
 *
 * @param None
 * @return The absolute pressure value in Pa (Pascals)
 */
float bpm280GetPressure();  // absolute pressure in Pa, ex:  101982.90

/**
 * @brief Set the current absolute pressure as reference for calculating the relative pressure
 *
 * A reference is required for this code to calculate the Gauge value. This function should be called when the exhale valve i open and the pressure drops to the ambient pressure.
 *
 * @param None
 * @return None
 */

void  bmp280SetReference();

/**
 * @brief return the Gauge value in cmH2O.
 *
 * Runs the following calc:
 * gaugeCmH2O = (Read_pressure - Stored_Reference_pressure) * 0.0101972;
 *
 * @param None
 * @return The gaugeCmH2O described above in cmH2O
 */

float getCmH2OGauge();

#endif // BMP280_INT_H
