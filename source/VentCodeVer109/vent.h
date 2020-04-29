#ifndef BEENHERE
#define BEENHERE                        // Controls data definitions versus declarations

//  LIBRARIES  

#define VERSION                110




//#define DEBUGSERIAL         // MUST be done in order to see any debuggng statements
                            // This is the one that sets up the serial port.
                            
#define DEBUGBMP280           //  To see statements related to BMP280 setup
//#define FLOWMEASURE           //  To see info related to flow measurements
//#define DEBUG              // Turn on to debug, geneal output of lots of sections
//#define SHOWINSTANTFLOW    // to see calculations of instantaneous flow
//#define TOTALTIDAL-DISPLAY // to see the tidal volume calculated each breath
//#define BMP280DEBUG        // to see serial port related to BMP
//#define AIRWAYPRESSURECHECKING   // help me figure out why the valve isn't opening
//#define SERIALPRINT;        // gives a load ofprintouts on pressure monitoring
//#define SMOOTHPRESSURERUN   // Investigating expiratory ringing and assist control ventilation
//#define DIGITALPRESSURESPY    // to see measurements related to digital I2C diff press
//#define ASSISTSPY           // allows visualization of ASSIST breaths, and of the smooth_pressure_run (>2) required to allow them
//#define VOLTAGESPY            // watch what happens with voltage



#define VOLTAGEDIVIDER      // if defined, indicates we have a voltage divider to sense supply problems
#define LOWVOLTAGELIMIT   11.5    // low voltage limit during activation of both solenoids
#define DIVIDER           0.25    // factor by which supply voltage is dropped




#include <Arduino.h>                    // Standard with Arduino IDE
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <AllSensors_DLHR.h>            // https://github.com/jeremycole/AllSensors_DLHR
#include <BMP180I2C.h>                  // Obsolete: replaced with BM280 on next include 
                                        // https://groups.io/g/VentilatorDevelopers/message/1241
#include <sSense-BMx280I2C.h>           // Download link halfway down this page: 
                                        // https://create.arduino.cc/projecthub/dragos-iosub/arduino-bme280-sensor-how-to-115560


//=========================== YOUR MUST SELECT CORRECT PARAMETERS HERE FOR SENSORS AND DISPLAY =========


//=====YOU MUST ENABLE ONE OF THESE TO GET FLOW MEASUREMENTS!!!!===============
//#define ANALOGDIFFTRANSDUCER              // if using the 2kPa analog transdcuer
#define   I2CDIFFTRANSDUCER               // if using the I2C transducer 

//#define STRAIGHTEXPLIMB
#define USHAPEEXPLIMB

#define BMP_ADDRESS 0x77                  // *****GLG  FOR ADAFRUT
//#define BMP_ADDRESS 0x76                  // *****GLG   THE ALTERNATIVE

//============================ DISPLAY PARAMETERS =============================      
// ITEMS TO SET WHEN MOVING CODE TO AND FROM ASHHAR / GORDON
// iN FILE lcd_ui.cpp --- set up the correct WIRING for the LCD
// //static LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //when using ubitx board

                                  
//#define I2CLCDDISPLAY           true      // Set to true if using I2C display device
#define   LCD20x4                 true      // Set to true if using non-I2C 20x4
//#define LCD16x2                 true      // Set to true if using non-I2C 16x2

                                        
#if I2CLCDDISPLAY == true                 // Assumes 20x4 if an I2c display                      
#define LCDDISPLAY              2004
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#else
#include <LiquidCrystal.h>
extern LiquidCrystal lcd;
#if LCD16x2 == true
#define LCDDISPLAY              1602
//static LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR
// done in the .ino file 
#else
#define LCDDISPLAY              2004
//  static LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR
//  done in the .ino file 
#endif

#endif

#define BTN_FN                  (1)
#define BTN_DN                  (2)
#define BTN_UP                  (4)
#define BTN_PW                  (8)

#define CHAR_CARET              (5)
#define CHAR_TWO                (6)
#define CHAR_BACK               (7)

/* --------not sure what these are for....
#define SELECT_BPM              1       // From LCD gui
#define SELECT_PRESSURE         0
#define SELECT_IE               2
#define SELECT_VOLUME           3
#define MAX_SELECTION           3
#define RESET_I2C               4
#define INSP_PAUSE              5
#define SOMETHING_PEEP          6
-----------------------------------------*/



#define CMD_DISPLAY             0
#define CMD_UP                  1
#define CMD_DN                  2
#define CMD_SELECTED            3


//============================ EEPROM PARAMETERS =============================  
#define EEPROM_VENT_ON          48
#define EEPROM_BPM              52
#define EEPROM_PRESSURE         56
#define EEPROM_IERATIO          60
#define EEPROM_PAUSE            64
#define EEPROM_TV               68
#define EEPROM_PEEP             72


#define AVERAGEBINNUMBER        10      // Number of averaging bins for the averaging routine
#define PRESSURESENSORPIN       A6      // pin that the flow pressure sensor is attached to
#define  SUPPLYVOLTAGESENSORPIN  A7      // goes to divide by 4 voltage divider from supply voltage

#define FLOWMAXSCALE            90      //Flow rate design Parameter is 90 Liters/min
#define VSOURCE                 5.0     //Source voltage for differential pressure sensor
#define PTFCC                   40      // Guessing for mine....      63.639Conversion factor for this orifice.
#define MF                      1.0     //Meter factor for calibration
#define PRESSUREREADDELAY       20L     // wait 20 ms between reads


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

#define USE_BMP280  1       /// set to 0 if not using the BMP280



//the lcd_size is 1602 (16x2) or 2004 (20x4)

// alarm declares and variables
extern boolean alarm_high_pressure;
extern boolean alarm_low_volume;
extern boolean alarm_high_volume;

extern int alarm_status;  

extern long alarm_off_until;
extern long alarm_on_until;
extern long alarm_suppress_until;
extern boolean alarm_low_pressure;

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

extern float  supply_voltage;                      // supply voltge 
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

extern int  trigger_cm;  // cm of water below nominal PEEP that patient has to pull; larger number = harder


extern long cTime;                           
extern long next_slice;
extern long now;                                 // wall clock of the system
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


void MyDelay(unsigned long ms);
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

// ANALOG
void measure_atmospheric_pressure();
int  ReadFlowPressureSensor();

// DIGITAL
void DigitalZeroDifferentialTransducer();

void vent_abort();
void vent_start();

/* static */  void make_ticker();
/* static*/  void clear_menu();
/* static */  void display_menu();
/* static */ void animate_ticker();

extern void init_pressure_sensor();         //  Initialize in order to get the atomospheric again!! GLG---
extern void measure_atmospheric_pressure(); // Measure ans store the atmospheric
extern void vent_abort();
extern void vent_init();

void menu_vent_on(struct menu_item *me, int cmd);
void menu_vent_off(struct menu_item *me, int cmd);
void menu_exit(struct menu_item *me, int cmd);
void menu_bpm(struct menu_item *me, int cmd);
void menu_pressure(struct menu_item *me, int cmd);
void menu_volume(struct menu_item *me, int cmd);
void menu_ie_ratio(struct menu_item *me, int cmd);
//void menu_ventilate(struct menu_item *me, int cmd);
void menu_reset_alarms(struct menu_item *me, int cmd);
void menu_reset_i2c(struct menu_item *me, int cmd);
void menu_pause_inhalation(struct menu_item *me, int cmd);
void menu_reset_sensor(struct menu_item *me, int cmd);
void menu_inspiratory_pause(struct menu_item *me, int cmd);
void menu_peep_desired(struct menu_item *me, int cmd);   // GLG -- add a way to change the PEEP setting
void menu_trigger_cm(struct menu_item *me, int cmd);     // GLG -- add a way to adjust trigger sensitivity


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
#endif
