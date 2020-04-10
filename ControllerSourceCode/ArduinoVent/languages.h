#ifndef LANGUAGES_H
#define LANGUAGES_H

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

#if (LANGUAGE_EN_US == 1)
/************************************************
 *
 *                English - USA
 *
 ************************************************
 */

#define     STR_IDLE                    "idle"              // max 4
#define     STR_RUN                     "run "
#define     STR_ERR                     "Err "

#define     STR_OFF                     "  off"             // must be 5 characters and at least 1 space
#define     STR_ON                      "   on"             // must be 5 characters and at least 1 space
#define     STR_NO                      "   no"             // must be 5 characters and at least 1 space
#define     STR_YES                     "  yes"             // must be 5 characters and at least 1 space

#define     STR_VENTILATOR              "Ventilator"        // max 10
#define     STR_BPM                     "BPM"               // max 10
#define     STR_DUTY_CYCLE              "Duty Cyc."         // max 10
#define     STR_PAUSE                   "Pause (ms)"        // max 10
#define     STR_LCD_AUTO_OFF            "LCD auto-off"      // max 10
#define     STR_PRESSURE                "Pressure"          // max 10

#define     STR_LOW_PRESSURE            "LOW AIRWAY PRES!"      // max 16
#define     STR_HIGH_PRESSURE           "OVER PRES ALARM!"      // max 16
#define     STR_UNDER_SPEED             "MOT UNDER SPEED!"      // max 16

#elif (LANGUAGE_PT_BR == 1)
/************************************************
 *
 *                Portuguese - Brazil
 *
 ************************************************
 */

#define     STR_IDLE                    "desl"              // max 4
#define     STR_RUN                     "Lig."
#define     STR_ERR                     "Erro"

#define     STR_OFF                     " desl"             // must be 5 characters and at least 1 space
#define     STR_ON                      " liga"             // must be 5 characters and at least 1 space
#define     STR_NO                      "  nao"             // must be 5 characters and at least 1 space
#define     STR_YES                     "  sim"             // must be 5 characters and at least 1 space

#define     STR_VENTILATOR              "Respirador"        // max 10
#define     STR_BPM                     "BPM"               // max 10
#define     STR_DUTY_CYCLE              "Razao."            // max 10
#define     STR_PAUSE                   "Pausa (ms)"        // max 10
#define     STR_LCD_AUTO_OFF            "LCD L/D"           // max 10
#define     STR_PRESSURE                "Pressao"           // max 10

#define     STR_LOW_PRESSURE            " BAIXA PRESSAO! "      // max 16
#define     STR_HIGH_PRESSURE           "  ALTA PRESSAO! "      // max 16

#else
  #error "One Language must be set to 1 in config.h"
#endif




#endif // LANGUAGES_H
