#ifndef BEENHERE
#define BEENHERE                        // Controls data definitions versus declarations

#define VERSION                 1.00
//#define JACKSTEST                       // Used to toggle Jack's 4z20 display. Comment out for display other than 4x20
#define DEBUG                           // Comment out when done debugging

#include <Arduino.h>                    // Standard with Arduino IDE
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "nano_gui.h"

                                        // Display configuration stuff...
#ifdef JACKSTEST                        
#define LCDDISPLAY              2004
extern LiquidCrystal_I2C lcd;
#else
#define LCDDISPLAY              1602
extern LiquidCrystal lcd;
#endif


//                                      Nano Pin Settings
#define ALARM_PIN               6
#define FN                      A0
#define UP                      A1
#define DOWN                    A2
#define ON_OFF                  A3
#define TFT_CS                  10        
#define TFT_RS                  9
#define CS_PIN                  8       //this is the pin to select the touch controller on spi interface
                                        // MOSI=11, MISO=12, SCK=13
                                        
#define SELECT_BPM              1       // From LCD gui
#define SELECT_PRESSURE         0
#define SELECT_IE               2
#define SELECT_VOLUME           3
#define MAX_SELECTION           3
#define RESET_I2C               4
#define INSP_PAUSE              5

#define MAX_BUTTONS             16    
#define BUTTON_SELECTED         1

#define ALARM_FAST              2
#define ALARM_SLOW              1
#define ALARM_OFF               0

#define SLOPE_X                 32
#define SLOPE_Y                 36
#define OFFSET_X                40
#define OFFSET_Y                44

#define MOTOR_A                 2
#define MOTOR_B                 3

#define MAX_PHASES              20      // At 40 the machne could barely keep up
#define TV_INCREMENT            50      // 50 cc tidal volume changes


//the lcd_size is 1602 (16x2) or 2004 (20x4)

//==================================== Global Declarations ==============================================
extern uint8_t I2CBusFailFlag  __attribute__ ((section (".noinit")));
extern uint8_t I2CBusAllowed;                    // variable to track whether this bus is allowed

extern bool exhale;                              //Boolean to indicate if on the exhale cycle
extern bool lastMode;                            //Boolean for switching from Inhale to exhale

extern int alarm_status;  
extern char alarm_array[];          // allows to keep track of 6 different alarms 
                    //  Each is 0  if no alarm there
                    //  Each is 0x01 if an alarm.
                    //  Get updated with each breath so transitory existence
                    //  alarm_array[0] refers to LOW PRESS
                    //  alarm_array[1] refers to HI PRESS
                    //  alarm_array[2] refers to LOW VOL
                    //  alarm_array[3] refers to HI VOL
                    //  others are undefined at the moment
                            
extern int beats_per_minute;            
extern int atmospheric_pressure;        
extern int bargraph[];   // Deprecated
extern int beats_per_minute;
extern int current_phase;
extern int current_pressure;
extern int cut_off;                              
extern int delP;                                 
extern int desired_TV;        
extern int exmLPerCycle;                         
extern int inspiratory_pause;          
extern int is_pressure_on;
extern int ie_ratio;    
extern int is_ringing;      
extern int lowpressurelimit;         
extern int lcd_size;
extern int mmH2O;
extern int rawSensorValue;
extern int pressure;
extern int peakinspiratorypressure;              
extern int pressure_sensor_present;
extern int selected_item;
extern int TVhighlimit;        
extern int TVlowlimit;
extern int use_tft;
extern int vent_running;
extern int vent_on;
extern int vent_off;
extern int vent_pressure_limit; 
extern int zerodelP; 
extern int zerorawSensorValue; 
extern int measured_peep;   

extern long alarm_off_until;
extern long alarm_on_until;
extern long alarm_suppress_until;
extern long cTime;                           
extern long next_slice;
extern long now;
extern long pressure_total;
extern long pressure_peak;
extern long randomNumber;                  ;




extern float exLitersPerCycle;                   // Liters Per exhale Cycle
extern float fpressure;                          // ambient pressure
extern float hum;                                // Humidity 

extern float inLitersPerCycle;                   // Liters Per Inhale Cycle
extern float instantFlowValue;                   // Instantaneous Flow
extern float pressurecmH2O;
extern float pressureSensorVoltage;              // Measured differential pressure sensor voltage. Purdum/Schmidt
extern float temp;                               // Temporary working variable


//======================================= Function Prototypes ================================

void MyDelay(unsigned long ms);    // Jack's non-blocking delay function
void alarm(int action);
void alarm_slice();
void alarm_silence(int silence_in_seconds);
int  alarm_get_status();

void checkCAT();

void lcd_init(int display_size);
void lcd_slice();
void lcd_status(char *text);
void lcd_log(char *text);
void lcd_graph_update();
void lcd_graph_clear();
void save_settings();
void measure_atmospheric_pressure();
int  ReadFlowPressureSensor();

void guiUpdate();
void checkCAT();
void checkTouch();

void tft_init();
void tft_slice();
void tft_status(char *text);
void tft_graph_update();
void tft_graph_clear();

void vent_abort();
void vent_start();

#endif

// set I2CBusAllowed=0 to disable the bus

//  EXAMPLE of Handling I2C bus:
//  if(I2CBusAllowed==1){
//     I2CBusFailFlag=0x99;  // set the flag to indicate failure if reset happens
//     ....now do i2C business
//     ....finished with I2C business
//     I2CBusFailFlag=0x38; // reset flag to "working"
//     }
//     else // put in code to provide default value for pressure measurement
//


//  With MAX_PHASES (number of slices to a ventilatory cycle) at 20, a request for a .  system printed 8 dots at resp rate of 28 
  //  With MAX_PHASES (number of slices to a ventilatory cycle at 60....  did not print any dots MA -- unable to keep up.
  //  So the maximum MAX_PHASES isn't that much....
