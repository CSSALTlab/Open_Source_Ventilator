
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

#define TAG1 0xd8
#define TAG2 0x34

typedef struct __attribute__ ((packed))  props_st {
  uint8_t tag1;
  uint8_t tag2;
  
  uint8_t propVent;
  uint8_t propBpm;
  uint8_t propDutyCycle;
  uint16_t propPause;
  uint8_t propLcdAutoOff;
  uint8_t propBle;

  uint8_t propLowPressure;
  uint8_t propHighPressure;
  uint16_t propLowTidal;
  uint16_t propHighTidal;

  uint8_t crc;
} PROPS_T;

static PROPS_T props;
static bool pendingSave = false;
static uint64_t tm_save;

// Note: defaults values will takes place in case the stored parameters are corrupted or empty

const char * propDutyCycleTxt[PROT_DUTY_CYCLE_SIZE] = {
    "  1:1",
    "  1:2",
    "  1:3",
    "  1:4"
};

static void setDefaultValues()
{
  props.tag1             = TAG1;
  props.tag2             = TAG2;
  
  props.propVent         = DEFAULT_VENT;
  props.propBpm          = DEFAULT_BPS;
  props.propDutyCycle    = DEFAULT_DUTY_CYCLE;
  props.propPause        = DEFAULT_PAUSE;
  props.propLcdAutoOff   = DEFAULT_LCD_AUTO_OFF;
  props.propBle          = DEFAULT_BLE;

  props.propLowPressure        = DEFAULT_LOW_PRESSURE;
  props.propHighPressure       = DEFAULT_HIGH_PRESSURE;
  props.propLowTidal           = DEFAULT_LOW_TIDAL;
  props.propHighTidal          = DEFAULT_HIGH_TIDAL;


}

static bool checkRecord(PROPS_T * prop_ptr)
{
  unsigned int i;
  bool is_empty = true; // assume empty

  if ( (prop_ptr->tag1 != TAG1) || (prop_ptr->tag2 != TAG2) ) {
    LOG("checkRecord: bad tag");
    return false;
  }

  // -------- check CRC ---------
  uint16_t crc = crc_8( (uint8_t *) prop_ptr, sizeof(PROPS_T) - 1);
  if (crc != prop_ptr->crc) {
    LOG("checkRecord: bad crc");
    return false;
  }

  return true;
}

void propInit()
{
  halRestoreDataBlock((uint8_t *) &props, sizeof(PROPS_T) );
  if (checkRecord(&props) == false) {
    LOG("EEPROM values not valid, loading default parameters");
    setDefaultValues();
    propSave();
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
  halSaveDataBlock((uint8_t *) &props, sizeof(PROPS_T) );
  pendingSave = false;
}

void propSetVent(int val) {
    //LOG("propSetVent");
    props.propVent = (uint8_t) val & 0x000000ff;
    setSavePending();
}

void propSetBpm(int val) {
    //LOG("propSetBpm");
    props.propBpm = (uint8_t) val & 0x000000ff;
    setSavePending();
}

void propSetDutyCycle(int val) {
     //LOG("propSetDutyCycle");
     props.propDutyCycle =  (uint8_t) val & 0x000000ff;
     setSavePending();
}

void propSetPause(int val) {
     //LOG("propSetPause");
     props.propPause =  (uint16_t) val & 0x0000ffff;
     setSavePending();
}

void propSetLcdAutoOff(int val) {
     //LOG("propSetLcdAutoOff");
     props.propLcdAutoOff =  (uint8_t) val & 0x000000ff;
     setSavePending();
}

void propSetBle(int val) {
      //LOG("propSetBle");
      props.propBle =  (uint8_t) val & 0x000000ff;
      setSavePending();
}

void propSetLowPressure(int val) {
      //LOG("propSetBle");
      props.propLowPressure =  (uint8_t) val & 0x000000ff;
      setSavePending();
}

void propSetHighPressure(int val) {
      //LOG("propSetBle");
      props.propHighPressure =  (uint8_t) val & 0x000000ff;
      setSavePending();
}

void propSetLowTidal(int val) {
      //LOG("propSetBle");
      props.propLowTidal =  (uint16_t) val & 0x0000ffff;
      setSavePending();
}

void propSetHighTidal(int val) {
      //LOG("propSetBle");
      props.propHighTidal =  (uint16_t) val & 0x0000ffff;
      setSavePending();
}

// ---------- Getters ------------
int propGetVent() {
//    LOG("propGetVent");
    return props.propVent;
}

int propGetBpm() {
    //LOG("propGetBpm");
    return props.propBpm;
}

int propGetDutyCycle() {
     //LOG("propGetDutyCycle");
     return props.propDutyCycle;
}

int propGetPause() {
     //LOG("propGetPause");
     return props.propPause;
}

int propGetLcdAutoOff() {
     //LOG("propGetLcdAutoOff");
     return props.propLcdAutoOff;
}

int propGetBle() {
      //LOG("propGetBle");
      return props.propBle;
}

int propGetLowPressure() {
      //LOG("propLowPressure");
      return props.propLowPressure;
}
int propGetHighPressure() {
      //LOG("propHighPressure");
      return props.propHighPressure;
}
int propGetLowTidal() {
      //LOG("propLowTidal");
      return props.propLowTidal;
}
int propGetHighTidal() {
      //LOG("propHighTidal");
      return props.propHighTidal;
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
