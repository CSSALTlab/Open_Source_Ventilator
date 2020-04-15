
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


#include "hal.h"
#include "event.h"
#include "properties.h"
#include "pressure.h"

#ifndef LCD_CFG_I2C
  #include "LcdMv.h"
#endif


#include "EEPROM.h"



#ifdef WATCHDOG_ENABLE
  #if defined(__AVR__)
    #include <avr/wdt.h>
  #else 
    error "MPU not supported... either add support or disable watchdog."
  #endif
#endif


//---------- Constants ---------
#define EEPROM_DATA_BLOCK_ADDRESS 0

static MONITOR_LET_T monitor_led_speed = MONITOR_LED_NORMAL;

//-------- variables --------
static uint64_t tm_led;
static bool alarm = false;
static int alarm_phase = 0;
static uint64_t tm_alarm;


static uint64_t tm_key_sampling;

static char lcdBuffer [LCD_NUM_ROWS][LCD_NUM_COLS];
static int cursor_col = 0, cursor_row = 0;


static int led_state = 0;

#define FREQ1 440
#define FREQ2 740
#define FREQ3 880

//--------- local prototypes ------
static void motorInit();

void halBeepAlarmOnOff( bool on)
{
  if (on == true) {
    alarm = true;
    alarm_phase = 0;
    tm_alarm = halStartTimerRef();
    // turn tone on
    alarm_phase = 0;
    tone(ALARM_SOUND_PIN, FREQ1);

  }
  else {
    alarm = false;
    noTone(ALARM_SOUND_PIN);
    // turn tone off
  }
}

//----------- Locals -------------

  #ifdef LCD_CFG_I2C
    LiquidCrystal_I2C lcd(0x27,4,4);
  #else
    LcdMv         lcd(  LCD_CFG_RS, 
                        LCD_CFG_E, LCD_CFG_D4, 
                        LCD_CFG_D5, 
                        LCD_CFG_D6, 
                        LCD_CFG_D7);
  #endif

#define TM_WAIT_TO_ENABLE_WATCHDOG 3000

uint64_t tm_wdt = 0;
static int wdt_st;

//-------------------------------------------------------  
//-------         Milliseconds Timer
//-------------------------------------------------------  

uint64_t halStartTimerRef()
{
    static uint32_t low32, high32 = 0;
    uint32_t new_low32 = millis();
    if (new_low32 < low32) high32++;
    low32 = new_low32;
    return (uint64_t) high32 << 32 | low32;
}

bool halCheckTimerExpired(uint64_t timerRef, uint64_t time)
{
    uint64_t now = halStartTimerRef();
    if (timerRef + time < now)
        return true;
    return false;
}

//-------------------------------------------------------  
//------- Microsecond timer implementation (needed for motor support)
//-------------------------------------------------------  
#ifdef ENABLE_MICROSEC_TIMER

#define OVER_32BITS 4294967296
static uint64_t   microFreeRunningTimer;
static uint64_t   lastMicros; // to check overflow

static void updateMicroFreeRunningTimer()
{
  uint64_t m = micros();
  uint64_t elapse;
  
  if (m < lastMicros) {
    LOG("micro overflow"); // happens every 70 minutes
    elapse = (m + OVER_32BITS) - lastMicros;
  }
  else {
    elapse = m - lastMicros;
  }
  
  microFreeRunningTimer += elapse;
  lastMicros = m;
}

uint64_t halStartMicroTimerRef()
{
  return microFreeRunningTimer;
}

bool halCheckMicroTimerExpired(uint64_t microTimerRef, uint64_t time)
{
  if ( (microTimerRef + time) < microFreeRunningTimer) // lapseMicroTime in microseconds
    return true;
  return false;
}
#endif // ENABLE_MICROSEC_TIMER
//-------------------------------------------------------  



static void initWdt(uint8_t reset_val)
{
#ifdef WATCHDOG_ENABLE
  wdt_st = 0;
  tm_wdt = halStartTimerRef();

  // the following line always return zero as bootloader clears the bit.
  // see hack at: https://www.reddit.com/r/arduino/comments/29kev1/a_question_about_the_mcusr_and_the_wdrf_after_a/
  //if ( MCUSR & (1<<WDRF) ) { // if we are starting dua watchdog recover LED will be fast
  if (reset_val & (1<<WDRF) ) {
    halSetMonitorLED(MONITOR_LED_FAST);
  }
  else{
    halSetMonitorLED(MONITOR_LED_NORMAL);
  }
  wdt_disable(); // keep WDT disable for a couple seconds
#else
  halSetMonitorLED(MONITOR_LED_SLOW);
#endif
}
static void loopWdt()
{
#ifdef WATCHDOG_ENABLE
  if (wdt_st == 0) { // wait to enable WDT 
    if (halCheckTimerExpired(tm_wdt, TM_WAIT_TO_ENABLE_WATCHDOG)) {
        tm_wdt = halStartTimerRef();
        wdt_st = 1;
        wdt_enable(WDTO_1S); //WDTO_2S Note: LCD Library is TOO SLOW... need to fix to get real time and lower WDT

        /* WDT possible values for ATMega 8, 168, 328, 1280, 2560
             WDTO_15MS, WDTO_30MS, WDTO_60MS, WDTO_120MS, WDTO_250MS, WDTO_500MS, WDTO_1S, WDTO_2S
           WDT possible values for  ATMega 168, 328, 1280, 2560
             WDTO_4S, WDTO_8S */
    }
    return;
  }
  //------- if we are here then WDT is enabled... kick it
  wdt_reset();
#endif
}

  
void halInit(uint8_t reset_val) {
  int r,c;
#ifdef DEBUG_SERIAL_LOGS
  Serial.begin(9600);
  LOG("Starting...");
#endif
  pinMode(MONITOR_LED_PIN, OUTPUT);
  tm_led = halStartTimerRef();
  
  propInit();
#ifdef LCD_CFG_I2C
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
#else
  #if (LCD_CFG_4_ROWS == 1)
    r = 4;
    #if (LCD_CFG_20_COLS == 1)
        c = 20;
    #else
        c = 16;
    #endif
  #else
    r = 2;
    #if (LCD_CFG_20_COLS == 1)
        c = 20;
    #else
        c = 16;
    #endif
  #endif
  lcd.begin(c, r);
  lcd.setFrameBuffer( (uint8_t *) lcdBuffer, r, c);
#endif
  halLcdClear();

  // -----  keys -------
  pinMode(KEY_SET_PIN, INPUT_PULLUP);           // set pin to input
  pinMode(KEY_INCREMENT_PIN, INPUT_PULLUP);           // set pin to input
  pinMode(KEY_DECREMENT_PIN, INPUT_PULLUP);           // set pin to input

// ------ valves -------
  pinMode(VALVE_IN_PIN, OUTPUT);           // set pin to input
  pinMode(VALVE_OUT_PIN, OUTPUT);           // set pin to input
  halValveInOff();
  halValveOutOff();

  tm_key_sampling = halStartTimerRef();
  initWdt(reset_val);
  pressInit();
  motorInit();
}

static void testKey()
{
  #if 1
  digitalWrite(VALVE_IN_PIN, digitalRead(KEY_SET_PIN));
  #else
    digitalWrite(VALVE_IN_PIN, digitalRead(KEY_DECREMENT_PIN));
  #endif
  digitalWrite(VALVE_OUT_PIN, digitalRead(KEY_INCREMENT_PIN));
}

void halSetMonitorLED (MONITOR_LET_T speed)
{
  monitor_led_speed = speed;
}
MONITOR_LET_T halGetMonitorLED ()
{
  return monitor_led_speed;
}

void halBlinkLED()
{
  uint64_t tm;
  if (monitor_led_speed == MONITOR_LED_FAST) {
    tm = TM_MONITOR_LED_FAST;
  }
  else if (monitor_led_speed == MONITOR_LED_SLOW) {
    tm = TM_MONITOR_LED_SLOW;
  }
  else {
    tm = TM_MONITOR_LED_NORMAL;
  }
  
    if (halCheckTimerExpired(tm_led, tm)) {
        tm_led = halStartTimerRef();


        if (led_state) {
          led_state = 0;
          digitalWrite(MONITOR_LED_PIN, LOW);
        }
        else {
          led_state = 1;
          digitalWrite(MONITOR_LED_PIN, HIGH);
        }
        
    }
}

static void alarmToggler()
{
    if (alarm != true) return;
    
    if (halCheckTimerExpired(tm_alarm, TM_ALARM_PERIOD / 5)) {
      tm_alarm = halStartTimerRef();
      switch (alarm_phase) {
        case 0:
          noTone(ALARM_SOUND_PIN);
          tone(ALARM_SOUND_PIN, FREQ2);
          break;
        case 1:
          noTone(ALARM_SOUND_PIN);
          tone(ALARM_SOUND_PIN, FREQ3);
          break;
        case 2:
          noTone(ALARM_SOUND_PIN);
          break;
        case 6:
          tone(ALARM_SOUND_PIN, FREQ1);
          alarm_phase = 0;
          return;
        default:
          break;
      }
      alarm_phase++;
    }
}

//-------- display --------

static void lcdUpdate()
{
    int i,r;
    char *s, *d;
    char out[LCD_NUM_COLS + 1];

    for (r=0; r<LCD_NUM_ROWS; r++) {
        d = out;
        s = &lcdBuffer [r][0];
        for (i=0; i<LCD_NUM_COLS; i++) {
            *d++ = *s++;
        }
        *d++ = 0;
      
#ifdef LCD_CFG_I2C          // only for I2C as LcdMv updates by refresh
        lcd.setCursor(0,r);
        lcd.print(out);
#endif
      
    }
}

void halLcdClear()
{
    memset(lcdBuffer, 0x20, sizeof(lcdBuffer));
    cursor_col = 0;
    cursor_row = 0;

}

void halLcdSetCursor(int col, int row)
{
    if ( cursor_col >= LCD_NUM_COLS) {
        LOG("halLcdSetCursor: bad cursor_col");
        return;
    }
    if ( cursor_row >= LCD_NUM_ROWS) {
        LOG("halLcdSetCursor: bad cursor_row");
        return;
    }
    cursor_col = col;
    cursor_row = row;

}

void halLcdWrite(const char * txt)
{
  int n;
  if ( cursor_col >= LCD_NUM_COLS) {
      LOG("halLcdWrite: bad cursor_col");
      return;
  }
  if ( cursor_row >= LCD_NUM_ROWS) {
      LOG("halLcdWrite: bad cursor_row");
      return;
  }
  n = strlen(txt);
  if (n > ( LCD_NUM_COLS - cursor_col)) {
      //LOG("halLcdWrite: clipping");
      n = LCD_NUM_COLS - cursor_col;
  }
  memcpy(&lcdBuffer[cursor_row][cursor_col], txt, n);
  // TODO: row overflow check or clipping
  lcdUpdate();
}

void halLcdWrite(int col, int row, const char * txt)
{
    halLcdSetCursor(col, row);
    halLcdWrite(txt);
}

//---------- valves Real
void halValveInOn()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_IN_PIN, LOW);
#else
    digitalWrite(VALVE_IN_PIN, HIGH);
#endif
}
void halValveInOff()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_IN_PIN, HIGH);
#else
    digitalWrite(VALVE_IN_PIN, LOW);
#endif
}
void halValveOutOn()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_OUT_PIN, LOW);
#else
    digitalWrite(VALVE_OUT_PIN, HIGH);
#endif
}
void halValveOutOff()
{
#ifdef VALVE_ACTIVE_LOW
    digitalWrite(VALVE_OUT_PIN, HIGH);
#else
    digitalWrite(VALVE_OUT_PIN, LOW);
#endif
}

//---------- Stepper Motor ---------
static void motorInit() 
{
#ifdef STEPPER_MOTOR_STEP_PIN
  pinMode(STEPPER_MOTOR_STEP_PIN, OUTPUT);
  pinMode(STEPPER_MOTOR_DIR_PIN, OUTPUT);
#ifdef STEPPER_MOTOR_EOC_PIN
  pinMode(STEPPER_MOTOR_EOC_PIN, INPUT_PULLUP);
#endif
  halMotorStep(false);
#endif
}

void halMotorStep(bool on)
{
#ifdef STEPPER_MOTOR_STEP_PIN
  digitalWrite(STEPPER_MOTOR_STEP_PIN, on); 
#endif
}

void halMotorDir(bool dir)
{
#ifdef STEPPER_MOTOR_STEP_PIN
  #ifndef STEPPER_MOTOR_INVERT_DIR
    digitalWrite(STEPPER_MOTOR_DIR_PIN, dir);
  #else
    digitalWrite(STEPPER_MOTOR_DIR_PIN, !dir);
  #endif
#endif
}

bool halMotorEOC()
{
#ifdef STEPPER_MOTOR_STEP_PIN
  #ifdef STEPPER_MOTOR_EOC_PIN
    return digitalRead(STEPPER_MOTOR_EOC_PIN);
  #else
    return false;
  #endif
#endif
}

//---------- Analog pressure sensor -----------
uint16_t halGetAnalogPressure()
{
  return (uint16_t) analogRead(PRESSURE_SENSOR_PIN);  //Raw digital input from pressure sensor
}

//---------- Analog pressure sensor -----------
uint16_t halGetAnalogFlow()
{
  return (uint16_t) analogRead(FLOW_SENSOR_PIN);  //Raw digital input from pressure sensor
}


//--------- Save/Restore data in non-volatil storage
bool halSaveDataBlock(uint8_t * data, int _size)
{
  unsigned int i, eeprom_addr = EEPROM_DATA_BLOCK_ADDRESS;
  LOG("halSaveDataBlock:");
  for (i=0; i< _size; i++) {
    //LOGV("addr = %d, d = 0x%x", eeprom_addr, *data);
    EEPROM.update(eeprom_addr, *data);
    eeprom_addr++, data++;
  }
}

bool halRestoreDataBlock(uint8_t * data, int _size)
{
  unsigned int i, eeprom_addr = EEPROM_DATA_BLOCK_ADDRESS;
  LOG("halRestoreDataBlock:");
  for (i=0; i< _size; i++) {
    *data = EEPROM.read(eeprom_addr);
    //LOGV("addr = %d, d = 0x%x", eeprom_addr, *data);
    eeprom_addr++, data++;

  }
}


//---------------- process keys ----------
#define   DEBOUNCING_N    4
typedef struct keys_st {
  int state; // 0-> released
  int count;
  int pin;
  int keyCode;
} keys_t;

static keys_t keys[3] = {
  {0, 0, KEY_DECREMENT_PIN, KEY_DECREMENT},
  {0, 0, KEY_INCREMENT_PIN, KEY_INCREMENT},
  {0, 0, KEY_SET_PIN, KEY_SET},
};

static void processKeys()
{
    int i;
    if ( halCheckTimerExpired(tm_key_sampling, TM_KEY_SAMPLING)) {
        tm_key_sampling = halStartTimerRef();
      for (i=0; i<3; i++) {
        if (keys[i].state == 0) {
          // ------- key is release state -------
          if (digitalRead(keys[i].pin) == LOW) { // if key is pressed
            keys[i].count++;
            if (keys[i].count >= DEBOUNCING_N) {
              //declare key pressed
              keys[i].count = 0;
              keys[i].state = 1;
              CEvent::post(EVT_KEY_PRESS, keys[i].keyCode);
            }

          }
          else {
              keys[i].count = 0;
          }
        }
        else {
          // ------- key is pressed state -------
           if (digitalRead(keys[i].pin) == HIGH) { // if key is release
            keys[i].count++;
            if (keys[i].count >= DEBOUNCING_N) {
              //declare key released
              keys[i].count = 0;
              keys[i].state = 0;
              CEvent::post(EVT_KEY_RELEASE, keys[i].keyCode);
            }
          }
          else {
              keys[i].count = 0;
          }       
        }
      }
        
    }
}

void halLoop()
{
  halBlinkLED();
  processKeys();
  propLoop();
  pressLoop();
  alarmToggler();

#ifdef WATCHDOG_ENABLE
  loopWdt();
#endif

#ifdef ENABLE_MICROSEC_TIMER
  updateMicroFreeRunningTimer();
#endif
  
  
#ifndef LCD_CFG_I2C
  lcd.stepRefresh();
#endif

}

void halWriteSerial(char * s)
{
#ifndef VENTSIM
  Serial.print(s);
#endif
}



 
