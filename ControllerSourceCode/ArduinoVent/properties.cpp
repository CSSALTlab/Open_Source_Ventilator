
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

#include "log.h"
#include "properties.h"
#include "config.h"
#include <stdint.h>
#include "crc.h"
#include "hal.h"

#ifndef VENTSIM
  #include <EEPROM.h>
#endif

#define EEPROM_PROPS_BASE_ADDRESS 0
#define TM_SAVE_TIMEOUT 30000 // save props to EEPROM is UI is "quiet" for longer than 30 seconds

typedef struct __attribute__ ((packed))  props_st {
  uint8_t propVent;
  uint8_t propBps;
  uint8_t propDutyCycle;
  uint8_t propPause;
  uint8_t propLcdAutoOff;
  uint8_t propBle;

  uint8_t crc;
} PROPS_T;

typedef enum {
  GOOD,
  EMPTY,
  ZERO,
  BAD
} CHECK_T;

static PROPS_T props;
static bool pendingSave = false;
static uint64_t tm_save;

// Note: defaults values will takes place in case the stored parameters are corrupted or empty

const char * propDutyCycleTxt[4] = {
    "  1:1",
    "  1:2",
    "  1:3",
    "  1:4"
};

static void setDefaultValues()
{
  props.propVent         = DEFAULT_VENT;
  props.propBps          = DEFAULT_BPS;
  props.propDutyCycle    = DEFAULT_DUTY_CYCLE;
  props.propPause        = DEFAULT_PAUSE;
  props.propLcdAutoOff   = DEFAULT_LCD_AUTO_OFF;
  props.propBle          = DEFAULT_BLE;
}

static void readRecord(int eeprom_addr, PROPS_T * dst_prop_ptr )
{
  int i;
  uint8_t * eeprom_addr_dst = (uint8_t *) dst_prop_ptr;
  for (i=0; i< sizeof(PROPS_T); i++) {
    *eeprom_addr_dst++ = EEPROM_read(eeprom_addr++);
  }
}

static void writeRecord(int eeprom_addr, PROPS_T * dst_prop_src )
{
  int i;
  uint8_t * eeprom_addr_src = (uint8_t *) dst_prop_src;
  for (i=0; i< sizeof(PROPS_T); i++) {
    EEPROM_write(*eeprom_addr_src++, eeprom_addr++);
  }
}


static CHECK_T checkRecord(PROPS_T * prop_ptr)
{
  int i;
  bool is_empty = true; // assume empty
  // ------  check if empty ------
  uint8_t * byte_ptr = (uint8_t *) prop_ptr;
  
  for (i=0; i<sizeof(PROPS_T); i++) {
    // check if record if empty
    if ( *byte_ptr != 0xff) {
      is_empty = false;
      break;
    }
  }
  if (is_empty)
    return EMPTY;

  // ------  check if zero ------
  byte_ptr = (uint8_t *) prop_ptr;
  is_empty = true; // here use as zero'ed
  for (i=0; i<sizeof(PROPS_T); i++) {
    // check if record if empty
    if ( *byte_ptr != 0) {
      is_empty = false;
      break;
    }
  }
  if (is_empty)
    return ZERO;


  // -------- check CRC ---------
  uint16_t crc = crc_8( (uint8_t *) prop_ptr, sizeof(PROPS_T) - 1);
  if (crc == prop_ptr->crc)
    return GOOD;

  return BAD;
}

void propInit()
{
  readRecord(EEPROM_PROPS_BASE_ADDRESS, &props );
  if (checkRecord(&props) != GOOD) {
    LOG("EEPROM values not valid, loading default parameters");
    setDefaultValues();
  }
}

void propLoop()
{
  if (pendingSave) {
    if (halCheckTimerExpired(tm_save, TM_SAVE_TIMEOUT)) {
      // save props in EEPROM
      LOG("Save timeout... lets save props into EEPROM");
      propSave();
    }
  }
}

static void setSavePending()
{
  pendingSave = true;
  tm_save = halStartTimerRef();
}

bool propSave()
{
  // update crc
  uint16_t crc = crc_8( (uint8_t *) &props, sizeof(PROPS_T) - 1);
  props.crc = crc;
  writeRecord(EEPROM_PROPS_BASE_ADDRESS, &props );
  pendingSave = false;
}

void propSetVent(int val) {
    LOG("propSetVent");
    props.propVent = (uint8_t) val & 0x000000ff;
    setSavePending();
}

void propSetBps(int val) {
    LOG("propSetBps");
    props.propBps = (uint8_t) val & 0x000000ff;
    setSavePending();
}

void propSetDutyCycle(int val) {
     LOG("propSetDutyCycle");
     props.propDutyCycle =  (uint8_t) val & 0x000000ff;
     setSavePending();
}

void propSetPause(int val) {
     LOG("propSetPause");
     props.propPause =  (uint8_t) val & 0x000000ff;
     setSavePending();
}

void propSetLcdAutoOff(int val) {
     LOG("propSetLcdAutoOff");
     props.propLcdAutoOff =  (uint8_t) val & 0x000000ff;
     setSavePending();
}

void propSetBle(int val) {
      LOG("propSetBle");
      props.propBle =  (uint8_t) val & 0x000000ff;
      setSavePending();
}


// ---------- Getters ------------
int propGetVent() {
//    LOG("propGetVent");
    return props.propVent;
}

int propGetBps() {
    LOG("propGetBps");
    return props.propBps;
}

int propGetDutyCycle() {
     LOG("propGetDutyCycle");
     return props.propDutyCycle;
}

int propGetPause() {
     LOG("propGetPause");
     return props.propPause;
}

int propGetLcdAutoOff() {
     LOG("propGetLcdAutoOff");
     return props.propLcdAutoOff;
}

int propGetBle() {
      LOG("propGetBle");
      return props.propBle;
}




//---------------- in case we decide to do a Wear leveling
#if 0

#define EEPROM_SIZE 512
#define MAX_NUM_RECORDS (EEPROM_SIZE / sizeof(PROPS_T))


static PROPS_T temp_props;
static int next_record_idx = 0; // next record index for writing

void propInit()
{
  // ---- find a valid last record ----
  int i;
  PROPS_T * prop_ptr = 0;
  CHECK_T check;
  
  for (i=0; i<MAX_NUM_RECORDS; i++) {
    // check if record if empty
    getRecord(prop_ptr, &temp_props );
    check = checkRecord(prop_ptr);
    if ( check == EMPTY ) {
      if (i != 0) {
        next_record_idx = i;
        // -------------- Lets fill our parameters with the previous GOOD record
        prop_ptr--; 
        getRecord(prop_ptr, &props );
        LOG("Get parameters from EEPROM");
        return;
      }
      else {
        // -------------- seems that the EEPROM is empty
        LOG("EEPROM is empty... use defaults");
        return;
      }
    }
    if (check == BAD) {
        LOG("EEPROM with bad record");
        return;
    }
    
    prop_ptr++; // check next
  }

  // -------------- Lets fill our parameters with the very last record
  prop_ptr--; 
  getRecord(prop_ptr, &props );
  LOG("Get parameters from EEPROM last record");
  return;

}
#endif
