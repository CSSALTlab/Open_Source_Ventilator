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


// ITEMS TO SET WHEN MOVING CODE TO AND FROM ASHHAR / GORDON
// iN FILE lcd_ui.cpp --- set up the correct WIRING for the LCD
// //static LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //when using ubitx board
//  static LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR
//
//  in vent.h
//  #define USE_BMP280   1     // set to 0 if not using theBMP280



#define USE_BMP280  1       /// set to 0 if not using the BMP280


extern void init_pressure_sensor();  //  Initialize in order to get the atomospheric again!! GLG---
extern void measure_atmospheric_pressure(); // Measure ans store the atmospheric
extern void vent_abort();
extern void vent_init();


//the lcd_size is 1602 (16x2) or 2004 (20x4)

// alarm declares and variables
extern int alarm_status;  
extern long alarm_off_until;
extern long alarm_on_until;
extern long alarm_suppress_until;
extern boolean alarm_low_pressure;
extern boolean alarm_high_pressure;
extern boolean alarm_low_volume;
extern boolean alarm_high_volume;

extern char alarm_array[];          // allows to keep track of 6 different alarms 
                    //  Each is 0  if no alarm there
                    //  Each is 0x01 if an alarm.
                    //  Get updated with each breath so transitory existence
                    //  alarm_array[0] refers to LOW PRESS
                    //  alarm_array[1] refers to HI PRESS
                    //  alarm_array[2] refers to LOW VOL
                    //  alarm_array[3] refers to HI VOL
                    //  others are undefined at the moment
                            



//I2C failure detection system 
#define I2C_BUSY  (0x99)
#define I2C_READY (0x38)
extern uint8_t i2c_status  __attribute__ ((section (".noinit")));
extern uint8_t i2c_allowed;                    // variable to track whether this bus is allowed


// global ventilator declares
extern bool exhale;                              //Boolean to indicate if on the exhale cycle
extern bool lastMode;                            //Boolean for switching from Inhale to exhale

extern int beats_per_minute;            
extern int atmospheric_pressure;        
extern int bargraph[];   // Deprecated
extern int beats_per_minute;
extern int desired_peep;   // the desired peep vaue. 
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

extern long cTime;                           
extern long next_slice;
extern long now;      // wall clock of the system
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

void vent_abort();
void vent_start();

#endif

// set I2CBusAllowed=0 to disable the bus

//  EXAMPLE of Handling I2C bus:
//  if(I2CBusAllowed==1){
//     i2c_status=0x99;  // set the flag to indicate failure if reset happens
//     ....now do i2C business
//     ....finished with I2C business
//     i2c_status=0x38; // reset flag to "working"
//     }
//     else // put in code to provide default value for pressure measurement
//


//  With MAX_PHASES (number of slices to a ventilatory cycle) at 20, a request for a .  system printed 8 dots at resp rate of 28 
  //  With MAX_PHASES (number of slices to a ventilatory cycle at 60....  did not print any dots MA -- unable to keep up.
  //  So the maximum MAX_PHASES isn't that much....
