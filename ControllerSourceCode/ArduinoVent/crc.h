#ifndef CRC_H
#define CRC_H

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

// usage: CalculateCRC16(0xffff, "123456789", 9);

uint16_t CalculateCRC16(
    uint16_t crc,      // Seed for CRC calculation
    const void *c_ptr, // Pointer to byte array to perform CRC on
    int len);          // Number of bytes to CRC

#endif // CRC_H
