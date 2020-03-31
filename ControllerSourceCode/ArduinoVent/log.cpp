
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



#include <stdarg.h>
#include <stdio.h>

#ifdef VENTSIM
  #include <QDebug>
  #include <QElapsedTimer>
#else
  #include <hardwareSerial.h>
#endif

#define V_BUF_SIZE  64
static char buf[V_BUF_SIZE];

void logv(const char *fmt, ...) {
    int len;
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(buf, V_BUF_SIZE, fmt, args);
    va_end(args);
    buf[V_BUF_SIZE - 1] = 0;
    if (len >= 63) {
        buf[V_BUF_SIZE - 2] = '.';
        buf[V_BUF_SIZE - 3] = '.';
        buf[V_BUF_SIZE - 4] = '.';
    }
#ifdef VENTSIM
    qDebug() << buf;
#else
    Serial.println(buf);
#endif
}

#ifdef VENTSIM
void LOG(const char * s) {
    qDebug() << QString(s);
}
#endif
