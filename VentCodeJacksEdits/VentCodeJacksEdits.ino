// Version 1.0, Apr 15, 2020. Jack Purdum, W8TEE. flash: 20108, SRAM: 1377. All warnings and errors removed
// Apr 14, 2020. Review by Jack Purdum, W8TEE.    flash: 20656, SRAM: 1379

// Ashhar Farhan Code  From April 1 2020
// Modified to work with BMP280 by Gordon Gibby
// Gordon Gibby gives NO PERMISSION FOR ANY USE WHATSOEVER
// without written permission from himself for usage.



#define I2C_ADDRESS 0x77                  //  What exactly is this for??? the BMP280 uses this

//------------------------BMP CHOICES --------------------------------------
//--------------------UNCOMMENT THE ONE YOU NEED----------------------------


#define BMP280
//#define BMP180



#ifdef BMP180
#include <BMP180I2C.h>
    //create an BMP180 object using the I2C interface
    BMP180I2C bmp180(I2C_ADDRESS);
#endif

#ifdef BMP280
#include <sSense-BMx280I2C.h>   // Download link halfway down this page: 
                                // https://create.arduino.cc/projecthub/dragos-iosub/arduino-bme280-sensor-how-to-115560

BMx280I2C::Settings settings(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  I2C_ADDRESS                                 // I2C address LCD display
);
BMx280I2C ssenseBMx280(settings);
#endif




//==================================BMP structs for setups ============================


#ifdef BMP280   
//                                Settings for the BMP280 device:

#endif

#ifdef BMP180

#endif

//=================================BMP Structs for setups===================================


#ifndef BEENHERE
#include "vent.h"
#endif

// BMP280 library info: https://create.arduino.cc/projecthub/dragos-iosub/arduino-bme280-sensor-how-to-115560
// Another source:   https://groups.io/g/VentilatorDevelopers/message/1224

// DEBUG PRINT OUTPUT (SERIAL MONITOR) DEFINES
// Defines for the Schmidt-Purdum Flow Measurement Code
//#define DEBUG              // Turn on to debug
//#define SHOWINSTANTFLOW    // to see calculations of instantaneous flow
//#define TOTALTIDAL-DISPLAY // to see the tidal volume calculated each breath
//#define BMP280DEBUG        // to see serial port related to BMP
#define AIRWAYPRESSURECHECKING   // help me figure out why the valve isn't opening


// ------UNITIALIZED DETECTION OF AN I2C BUS CALL FAILURE
// Following variable is uninitialized ( so may come up randomly )
// Code will set it immediatly to  0x38 to indicate normal operation, not involving I2C bus
// Same variable will be set to 0x99 whenever entering a subroutine that has the possibility
// of "blocking" on a failed I2C call" -- then if the watchdog timer resets, and the
// setup() discovers that variable is 0x99 (rare by random chance) the code will presume
// the I2C bus has failed, will notify the user, and will disable calls to the I2C bus
// and operate as well as possible without that information
// This protects the ventilator from failing due to an I2C bus failure
// GLG   04102020


#define PEEP  10 // GLG test peep    04172020

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


#include <Wire.h>
#include <avr/wdt.h>            // required for the watchdog timer, see routines below.  






#include <EEPROM.h>
#include "nano_gui.h"
#include "vent.h"

#define EEPROM_VENT_ON          48
#define EEPROM_BPM              52
#define EEPROM_PRESSURE         56
#define EEPROM_IERATIO          60
#define EEPROM_PAUSE            64
#define EEPROM_TV               68

#define AVERAGEBINNUMBER        10      // Number of averaging bins for the averaging routine
#define PRESSURESENSORPIN       A6      // pin that the flow pressure sensor is attached to
#define FLOWMAXSCALE            90      //Flow rate design Parameter is 90 Liters/min
#define VSOURCE                 5.0     //Source voltage for differential pressure sensor
#define PTFCC                   90      // Guessing for mine....      63.639Conversion factor for this orifice.
#define MF                      1.0     //Meter factor for calibration
#define PRESSUREREADDELAY       20L     // wait 20 ms between reads

#define INSP_PRESSURE_LIMITS
#define AIRWAYPRESSURECHECKING



//---------------------FLOW MEASUrEMENT SUBRoutINE  (Schmidt / Purdum from 4/5/2020 Version 4
//  Here are the routines and what they will do and return
//  int ReadFlowPressureSensor()    returns an positive-valued int,  from 0 to 1023
//      Note:  zerorawSensorValue will keep the zero-flow value (baseline)
//      it also computes the zero-based global variable  int delP = rawSensorValue-zerorawSensorValue;
//         (delP is zero-based, can be + or - )
//
//   float CalculateInstantFlow() -- uses global variable delP
//      returns float instantFlowValue = MF * PTFCC * sqrt(delP * correction);
//              Note that sqrt takes any type of argument, and returns float -- so this ends up being float
//              and the SIGN of the return value is significant.
//      At the moment I don't know the correct values for those constants....
//      Note that the time slice width in milliseconds is basically the same for all slices.
//      So simply add up all the instantFlowValues and multiply by the total time of expiration to get the total volume
//

//==================================== Global Definitions ==============================================
// moved these to vent.h
uint8_t I2CBusFailFlag  __attribute__ ((section (".noinit")));
uint8_t I2CBusAllowed;   // variable to track whether this bus is allowed

bool exhale;                              //Boolean to indicate if on the exhale cycle
bool lastMode;                            //Boolean for switching from Inhale to exhale

int alarm_status            = ALARM_OFF;
char alarm_array[6];          // allows to keep track of 6 different alarms 
                    //  Each is 0  if no alarm there
                    //  Each is 0x01 if an alarm.
                    //  Get updated with each breath so transitory existence
                    //  alarm_array[0] refers to LOW PRESS
                    //  alarm_array[1] refers to HI PRESS
                    //  alarm_array[2] refers to LOW VOL
                    //  alarm_array[3] refers to HI VOL
                    //  others are undefined at the moment

int atmospheric_pressure    = 0;          // in cmH2O
int bargraph[MAX_PHASES];  // bargraph deprecated 
int beats_per_minute        = 10;
int current_phase           = 0;
int current_pressure        = 0;
int cut_off;                              // phase at which we turn off the inspiration
int delP;                                 //Measured differential pressure sensor Flow Delta Pressure in Kpa; Purdum/Schmidt
int desired_TV              = 700;        // default tidal volume = 700
int exmLPerCycle;                         // int milliliters exhanled
int inspiratory_pause       = 0;          // set to 1 for inspiratory pause
int is_pressure_on          = 0;
int ie_ratio                = 1;          // this is the denominator
int is_ringing              = 0;
int lcd_size                = LCDDISPLAY; // Determined in vent.h
int lowpressurelimit        = 3;          // DEFAULT LOW PRESSURE LIMIT
int mmH2O;
int rawSensorValue          = 0;          // Measured differential pressure sensor raw value in bits. Purdum/Schmidt
int pressure                = 160;        // this is in mm
int peakinspiratorypressure;              // Try to track the peak inspiratory pressure
int pressure_sensor_present = 0;
int selected_item           = SELECT_BPM;
int TVhighlimit             = 700;        // high limit TV milliliters. See desired_TV above, too: Purdum/Schmidt
int TVlowlimit              = 300;        // low limit TV milliliters Purdum/Schmidt
int use_tft                 = 0;          // TFT display flag
int vent_running            = 1;
int vent_pressure_limit     = 40;         // Units are cmH2O (gauge)  
int zerodelP;                             // measured delP at presumed zero flow  Purdum/Schmidt
int zerorawSensorValue;                   // baseline raw sensor output (at no pressure no flow); Purdum/Schmidt

long alarm_on_until         = 0L;
long alarm_off_until        = 0L;
long alarm_suppress_until   = 0L;
long cTime;                               //time of last computations for the integrator
long next_slice             = 0L;         // Use 'L' for typoe suffix, not lowercase 'l'--looks like a 1 digit character
long now;
long pressure_total         = 0L;
long pressure_peak          = 0L;
long randomNumber;                        // Use for testing




float exLitersPerCycle;                   // Liters Per exhale Cycle
float fpressure;                          // ambient pressure
float hum;                                // Humidity 

float inLitersPerCycle;                   // Liters Per Inhale Cycle
float instantFlowValue;                   // Instantaneous Flow
float pressurecmH2O;
float pressureSensorVoltage;              // Measured differential pressure sensor voltage. Purdum/Schmidt
float temp;                               // Temporary working variable

//===================================== Define Objects ================================






#ifdef JACKSTEST
LiquidCrystal_I2C lcd(0x27, 20, 4);           // Define the display object
#else
LiquidCrystal lcd(13,12,11,10,9,8);

#endif
//int lcd_size        = LCDDISPLAY;           // This statement defines your display. See also top of config.h


/*****
    Purpose: To delay so many milliseconds without blocking interrupts

  Parameter list:
    unsigned long ms      the number of milliseconds to delay

  Return value:
    void

*****/
void MyDelay(unsigned long ms)
{
  unsigned long currentMillis  = millis();
  unsigned long previousMillis = millis();

  while (currentMillis - previousMillis < ms) {
    currentMillis = millis();
  }
}


/*****
  Purpose: Calculate the change in pressure, delP, from the sensor-generated voltage

  Argument list:
    void

  Return value:
    int   ranging from 0 to 1023

  CAUTION:
*****/
int ReadFlowPressureSensor()
{
  // lets average 3 readings and try that.
  int q;
  rawSensorValue = 0;
  for (q = 0; q < 3; q++) {
    rawSensorValue        =  rawSensorValue + analogRead(PRESSURESENSORPIN);
  }
  rawSensorValue = rawSensorValue / 3;

#ifdef  RAWSENSOR_DISPLAY
  Serial.print("Raw Sensor value: ");
  Serial.print(rawSensorValue);
  Serial.print("\n");
  delay(500);
#endif

  //Raw digital input from pressure sensor
  //pressureSensorVoltage = ((float) map(rawSensorValue, 0, 1023, 0, 5)) / 100.0;   //Convert raw bits read to volts
  // delP                  = (5.0 * pressureSensorVoltage / VSOURCE) - 2.5;          //Convert volts to kPa  Return delP;
  delP = rawSensorValue - zerorawSensorValue; // adjust for the offset of the sensor based on the baseline that was captured in the setup();
  // later we can make this spiffier and adjust with every breath

  return (rawSensorValue);
}


/*****
  Purpose: Calculate the instantaneous flow using the flow pressure

  Argument list:
    void  --- uses global variable (int) delP which is zero-based

  Return value:
    float       instantaneous flow rate  (sign is meaningful on the return )

  CAUTION:
*****/
float CalculateInstantFlow()
{
  float correction;                 // Used in calculation to avoid square root of negative number
  if (delP < 0)
    correction = -1.0;
  else
    correction = 1.0;
  instantFlowValue = MF * PTFCC * sqrt(delP * correction);
  instantFlowValue *= correction;

#ifdef SHOWINSTANTFLOW
  Serial.print("CalculateInstantFlow");
  Serial.print("    delP = ");
  Serial.print(delP);
  Serial.print("    instantFlowValue = ");
  Serial.println(instantFlowValue);
#endif

  return instantFlowValue;
}



// -----------------------End of FLOW MEASUREMENT SUBROUTINES------------------




void checkCAT() {

}

void save_settings() {
  EEPROM.put(EEPROM_VENT_ON, vent_running);
  EEPROM.put(EEPROM_BPM, beats_per_minute);
  EEPROM.put(EEPROM_PRESSURE, vent_pressure_limit);
  EEPROM.put(EEPROM_IERATIO, ie_ratio);
  EEPROM.put(EEPROM_PAUSE, inspiratory_pause);
  EEPROM.put(EEPROM_TV, desired_TV);

}

void load_settings() {
  EEPROM.get(EEPROM_VENT_ON, vent_running);
  EEPROM.get(EEPROM_BPM, beats_per_minute);
  EEPROM.get(EEPROM_PRESSURE, vent_pressure_limit);
  EEPROM.get(EEPROM_IERATIO, ie_ratio);
  EEPROM.get(EEPROM_PAUSE, inspiratory_pause);
  EEPROM.get(EEPROM_TV, desired_TV);
  if (vent_running != 1)
    vent_running = 0;
  if (beats_per_minute < 0 || beats_per_minute > 30)
    beats_per_minute = 10;
  if (vent_pressure_limit < 5 || vent_pressure_limit > 60)
    vent_pressure_limit = 40;
  if (ie_ratio < 1 || ie_ratio > 4)
    ie_ratio = 1;
  if (inspiratory_pause != 1) inspiratory_pause = 0;
  if (desired_TV < 100 || desired_TV > 999) desired_TV = 700; // default TV

}

void update_status2(char *text) {
  if (use_tft)
    displayText(text, 0, 0, 320, 78, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  else
    lcd_status(text);
}
void log_message(char *text) {
  if (use_tft)
    displayText(text, 0, 0, 320, 78, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  //else
  //  lcd_log(text);
  Serial.println(text);
}

//returns the absolute pressure in mmH20
//based on the formula at https://www.convertunits.com/from/mmH2O/to/Pa


int measure_pressure() {
  unsigned long testpressure;

#ifdef BMP280
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
#endif

  //start a temperature
#ifdef SERIALPRINT
  Serial.print("measure_pressure\n");
#endif

  //  EXAMPLE of Handling I2C bus:
  //  if(I2CBusAllowed==1){
  //     I2CBusFailFlag=0x99;  // set the flag to indicate failure if reset happens
  //     ....now do i2C business
  //     ....finished with I2C business
  //     I2CBusFailFlag=0x38; // reset flag to "working"
  //     }
  //     else  // put in code to provide default value for pressure measurement
  //

  if (I2CBusAllowed == 1) {
    I2CBusFailFlag = 0x99; // set the flag to indicate failure if reset happens


#ifdef BMP280    
    ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);
#endif  // End of the BMP280

#ifdef BMP180
  //start a temperature measurement using BMP180
  if (!bmp180.measureTemperature())
  {
    log_message("Sensor Err1");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
  do
  {
    delay(0);
  } while (!bmp180.hasValue());

  //start a pressure measurement. pressure measurements depend on temperature measurement, you should only start a pressure 
  //measurement immediately after a temperature measurement. 
  if (!bmp180.measurePressure()){
    Serial.println("Sensor Err2");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
  do{
    delay(0);
  } while (!bmp180.hasValue());

  fpressure = ((102l * (long)bmp180.getPressure())/1000l); 
// GLG****   I don't know what TYPE the bmp returns -- so this may need to be fixed 
// GLG****   I don't know the proper conversion here!!!  needs to be PASCALS!!!
// GLG****   I convert pascals into cmH2 just a few lines further down

#endif    // end of the BMP180



    testpressure = (unsigned long) fpressure;
    testpressure = testpressure / 98;  // cm H2O
    // If we got here, we must be fine
    I2CBusFailFlag = 0x38; // ===reset flag to "working"====
  }
  else testpressure = 999; // set measured ambient pressure to 999 cmH2O TO MARK AN ERROR!!!!

  
#ifdef SERIALPRINT
  Serial.print("1stPress cmH2O: ");
  Serial.print(testpressure);
  Serial.print("\n");
#endif
  return (testpressure);
}


int loop_count = 0;

void pressure_on() {
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, HIGH);
  is_pressure_on = 1;
}

void pressure_off() {   // now implements both inspiratory pause AND PEEP 

    digitalWrite(MOTOR_A, LOW);

  // put in a delay to make them happy
   if(inspiratory_pause==1){
   if (current_phase> (cut_off+4) )  digitalWrite(MOTOR_B, LOW);
   }
   else {  // NOW we need to look at PEEP
    if(current_pressure>PEEP) digitalWrite(MOTOR_B, LOW);
    if(current_pressure<PEEP) digitalWrite(MOTOR_B, HIGH);  // a bit of hysteresis here
    
   }
  
  is_pressure_on = 0;  
  
}

void measure_atmospheric_pressure() {
  Serial.print("measure_atmospheric_pressure\n");
  //if pressure is on, turn it off and wait for 8 seconds for the lungs and compressor to deflate
  if (is_pressure_on) {
    pressure_off();
    delay(100);
  }
  //now calibrate for 0.5 seconds
  atmospheric_pressure = 0;
  for (int i = 0; i < 10; i++) {
    atmospheric_pressure += measure_pressure();
    delay(50);
  }
  //now take the average
  atmospheric_pressure /= 10;
#ifdef SERIALPRINT
  Serial.print("Atm press= ");
  Serial.print(atmospheric_pressure);
  Serial.print("\n");
#endif

}

/*
   This has to be tweaked
*/

void check_pressure_limits() {


  //First check for over-pressure


  // run a comparison to build up the peak inspiratory pressure
  // will have to zero this at the beginning of each respiratory cycle
  // somewhere???
  if (current_pressure > peakinspiratorypressure) peakinspiratorypressure = current_pressure;

  if (peakinspiratorypressure > vent_pressure_limit)  {  // check against the real pressure limit
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);// get both valves open!!
     // open expiratory valve and close inspiratory valve
    alarm(ALARM_FAST);
    update_status2( (char*) "HI");
    Serial.print("HI ALARM\n\n");
  }

#ifdef AIRWAYPRESSURECHECKING
  Serial.print("\ncurrent_phase: , cut_off");
  Serial.print(current_phase);
  Serial.print(",");
  Serial.print(cut_off);
  Serial.print("\ncheck pres limits\n");
  Serial.print("cur_pres, peak meas, lowlimit ");
  Serial.print(current_pressure);
  Serial.print(" ");
  Serial.print(peakinspiratorypressure);
  Serial.print(" ");
  Serial.print(lowpressurelimit);
  Serial.print("\n ");
#endif


  //check only if the current phase is at the end of inspiration
#ifdef SERIALPRINT
  Serial.print("current_phase = ");
  Serial.print(current_phase);
  Serial.print("cut_off  ");
  Serial.print(cut_off);
  Serial.print("\n");
#endif

  if ( current_phase == (cut_off + 1)) {

    // first check for failure to reach low pressure limit
    if (peakinspiratorypressure < lowpressurelimit)
    { // we are at the end of inspiration and never saw an acceptable pressure
      alarm(ALARM_SLOW);
      update_status2( (char*) "LO");
      alarm_array[0] = 1;  // 0 slot is for LO PRESS
      
#ifdef INSP_PRESSURE_LIMITS
      Serial.print("current_phase = ");
      Serial.print(current_phase);
      Serial.print("LO ALARM\n\n");
      Serial.print("Pk Insp P: ");
      Serial.print(peakinspiratorypressure);
      Serial.print("Lo P Lim: ");
      Serial.print(lowpressurelimit);
      Serial.print("\n\n");
#endif
    }
    if (peakinspiratorypressure > lowpressurelimit) {
      alarm(ALARM_OFF);
      update_status2( (char*) "ON");
      alarm_array[0] =  0;    // zero the inspiratory alarm 
      
#ifdef INSP_PRESSURE_LIMITS
      Serial.print("No low pressure\n\n");
#endif
    }
  }  // end of checks on inspiratory low and high pressure

  /*---------------GLG I don't understand --------
    else if (pressure_peak < (pressure * 12)/10){
    alarm(ALARM_OFF);
    update_status2( (char*) "ON");
    --------------  */



}

void vent_slice() {
  long now;
  float diffpressure[MAX_PHASES];
  long differentialpressure;





  if (!vent_running) {
    alarm(ALARM_OFF);
    return;
  }

// remember, "cut_off" is a PHASE (or or slice) number....out of 20 currently  (MAX_PHASES=20)
// have to calculate cut_off becuse it may not be known
  cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);
if( current_phase > cut_off  && inspiratory_pause==0  )    // we only look for inspiratory effort during expiration 
    // for simplicity I'm not offering this with inspiratory pause also
    {   // WE ARE IN EXHALATION          
    // Have to measure the pressure....
    int p = measure_pressure();

    if  ((p+1)< atmospheric_pressure)  {   // this needs to be adjusted for peep!!!!
        Serial.print("\n\n**ASSISTED BREATH**\n\n");
        // jam us to the end of this slide immediately
        current_phase=0;   // force us into the beginning of an inspiratory pressure state
        next_slice= millis()-1  ;   // force us into a decision point (end of time slice)      
        }   
}

// ATTEMPT PEEP CONTROL------------------------------------------------
if(current_phase>cut_off)   // Note that if we have jammed an inspiratio to start, this will be avoided
{
  current_pressure =measure_pressure()- atmospheric_pressure;   
  pressure_off();  // that routine has it written in now.   
 
}



  now = millis();
  if (next_slice > now) {    // TEST LOAFING TIME
   Serial.print(".");
    //  With MAX_PHASES (number of slices to a ventilatory cycle) at 20, system printed 8 dots at resp rate of 28
    //  With MAX_PHASES (number of slices to a ventilatory cycle at 60....  did not print any dots MA -- unable to keep up.
    //  So the maximum MAX_PHASES isn't that much....
    return;
  }


  // Note that it may go through many times before it ever goes any further.

  // Zero the peak inspiratory pressure if this is our very first time
  // in this cycle.

  if (current_phase == 0)peakinspiratorypressure = 0;  /// no longer needed here // 

  cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);   // no longer needed here 

  check_pressure_limits();  // check this at the end of every slice

  //we move to the next slice
  if ((current_phase < cut_off) && (peakinspiratorypressure< vent_pressure_limit ) )  {
    pressure_on();     /// THIS IS WHAT IS TURNING THE PRESSURE BACK ON DESPItE AN ALARM!!!
    exLitersPerCycle = 0 ; // initalize for  computations next exhalation
  }
  //  During the expiratory phase we need to measure
  //  1)  differential pressure across an obstruction
  //  2)  compute the flow rate
  //  3)  multiply by the time slice to get volume in that breath
  //  4)  keep accumulating it.
  //  5)  zero it out during inspirations.

  if (current_phase > cut_off) {
    pressure_off();   // Note that pressure_off() now correctly handles both inspiratory pause AND PEEP
    // we are in EXHALATIION

    // I have no idea what the proper constants are here.....
    diffpressure[current_phase] =  ReadFlowPressureSensor();
/*
#ifdef DELP-DISPLAY
    Serial.print ("Differential pressure:   ");
    Serial.print (delP);
    Serial.print ("\n");
#endif
*/
    // Now calculate the FLOW for that pressure difference:
    // The mlliseconds assigned to each slice is 60000l / ((long)beats_per_minute * (long)MAX_PHASES)
    // where 60000 l  is (long) 60000
    // so the SECONDS of each slice is   float  60/(  (float)beats_per_minute  * (float)MAX_PHASES )
    // and we use exLitersPerCycle to build up the expiredLiters per cycle as follows:
    exLitersPerCycle = exLitersPerCycle  +   (  CalculateInstantFlow() * 60 / ( (float) (beats_per_minute *  MAX_PHASES))  )  ;
    // GLG -- Marcelo caught the erroneous PLUS sign where beats_per_minute wa supposed to be multiplied by MAX_PHASES.... fixed now.

#ifdef  ACCUMULATIONDISPLAY
    Serial.print("Accumuating exLitersPerCycle");
    Serial.print((long) exLitersPerCycle );
    Serial.print( "\n");
#endif
  }  // end of things to do AFTER cut_off (hence in exhalation)


  // this is what sets the TIME WIDTH of a slice --
  // the 60 000 l   is 60 thousand (LONG)  milliseconds
  // at 10 breatghs per minute and MAX_PHASES = 40, each slide is 150ms
  // at 30 breaths per minute, it would be 50 msec

  next_slice = now + 60000L / ((long)beats_per_minute * (long)MAX_PHASES);
/*
#ifdef TIMESLICE-DISPLAY
  Serial.print("Now, nextslice: ");
  Serial.print(now);
  Serial.print("  ");
  Serial.print(next_slice);
  Serial.print("\n");
#endif
*/
int  p = measure_pressure();


/*  --------original location and code for first assisted ventilation trial ---------
  //---------------------ASSISTED VENTILATION EXPERIMENT---------------------
  // Lets try to add assisted ventilation
  if ( (current_phase > cut_off) &&(inspiratory_pause == 0) && (p + 1 < atmospheric_pressure) )
  {
    // slam us to the end of the expiratory phase
    current_phase = MAX_PHASES + 1;
    Serial.print("\n\n**ASSISTED BREATH**\n\n");
  }
  // ---------------------END OF ASSISTED VENT EXPErIMENT-----------------------

-------------------------------original assisted ventilation ----------------------*/


  bargraph[current_phase] = (int)(p - atmospheric_pressure);  // I don't think we need the bargraph
  current_pressure = bargraph[current_phase];
  //current_pressure = (int) (p- atmospheric_pressure);// calcuate the guage pressure    
  pressure_total += current_pressure;
  if (use_tft)
    tft_graph_update();
  else
    lcd_graph_update();
  current_phase++;

  if (current_phase >= MAX_PHASES) { // end of the entire respiratory cycle
    exmLPerCycle = (int) exLitersPerCycle; // note exmLPerCycle= INT
    if (exmLPerCycle < 0) exmLPerCycle = 0; // dont allow negative numbers

    // Check tidal volume limits
    if (exmLPerCycle > (desired_TV + 100)) {
      alarm(ALARM_SLOW);
      update_status2( (char*) "V^");
      alarm_array[3]= 1;  // set the alarm array
    } else alarm_array[3]= 0; 
    
    if (exmLPerCycle < (desired_TV - 100)) {
      alarm(ALARM_FAST);
      update_status2( (char*) "Vv");
      alarm_array[2]=1;  // set the alarm array

    }  else alarm_array[2]=0;   // reset the alarm array 
    
/*
#ifdef TOTALTIDAL-DISPLAY
    Serial.print("Tidal Vol This Breath: ");
    Serial.print("exLitersPerCycle");
    Serial.print("  ");
    Serial.println((int) exLitersPerCycle);
    Serial.print("exmLPerCycle: " );
    Serial.print(exmLPerCycle);
    Serial.print("\n\n");
#endif
*/
    // Now WRITE IT OUT To THE DISPLAY!!!
    // try callng this update directly:

    // lcd_graph_update();   // that SHOULD print it out


    if (use_tft)
      tft_graph_clear();
    else
      lcd_graph_clear();
    current_phase = 0;
  }
}



void vent_init() {

  log_message( (char*) "Testing...");

  //begin() initializes the interface, checks the sensor ID and reads the calibration parameters.

#ifdef BMP180   //   SETUP THE BMP180 
    if (!bmp180.begin()){
      pressure_sensor_present = 0;
      while (1);
    }

  //reset sensor to default parameters.
    bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
    bmp180.setSamplingMode(BMP180MI::MODE_UHR);
#endif   // END OF BMP180 SETUP  (BMP280 is inside setup()

  log_message( (char*) "Starting..");
  measure_atmospheric_pressure();
  log_message( (char*) "Ready");
}

void setup() {
  // put your setup code here, to run once:
  // turn on the watchdog FIRST THING
  setup_watchdog();   // see elsewhere for explanation.
  Serial.begin(115200);
  while (!Serial);

#ifdef JACKSTEST                        
  lcd.begin();
  lcd.backlight();
#else
  lcd.begin(16,2);
#endif

  //check on whether this was a reset due to an I2C bus failure
  //  EXAMPLE of Handling I2C bus:
  //  if(I2CBusAllowed==1){
  //     I2CBusFailFlag=0x99;  // set the flag to indicate failure if reset happens
  //     ....now do i2C business
  //     ....finished with I2C business
  //     I2CBusFailFlag=0x38; // reset flag to "working"
  //     }
  //     else // put in code to provide default value for pressure measurement
  //
  //  I2C can now be turned on by a two-button entry (currently ON_OFF & DOWN
  // --------------INTENTIONALLY FORCED WORKING FOR TESTING-----------------------
   I2CBusFailFlag == 0x38;  // ***COMMENT OUT FOR RELEASE***
  // -------------------INTENTIONALLY FORCED WORKING FOR TESTING------------------

  if (I2CBusFailFlag == 0x99) {
    // I2CBus likely failed
    Serial.print("I2C BUS FAILURE!! \n");
    // turn off usage of the I2CBus
    //    I2CBusAllowed=0;  //  ***UN-COMMENT FOR RELEASE ***

    //----------------------INTENTIONALLY FORCED WORKING FOR TESTING-------------
   I2CBusAllowed = 1; // only for my test!  //  ***COMMENT OUT FOR RELEASE***
    //--------------------- INTENTIONALLY FORCED WORKING FOR TESTIG --------------

  }
  else {
    I2CBusFailFlag = 0x38; // reset the flag if not a failure
    I2CBusAllowed = 1;   // allow usage of the I2C Bus
    // must check for this before ANY calls
  }
  // Chance of randomly hitting either value is 1/256

  for(int t=0; t<6; t++) alarm_array[t] = 0;   // zero out the char alarm_array values
  

  load_settings();
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);

  Wire.begin();   // <<<<=========HAS TO BE CALLED BEFORE TRYING TO USE i2C BUS*****************

  if (use_tft)
    tft_init();
  else
    lcd_init(LCDDISPLAY);

  //  EXAMPLE of Handling I2C bus:
  //  if(I2CBusAllowed==1){
  //     I2CBusFailFlag=0x99;  // set the flag to indicate failure if reset happens
  //     ....now do i2C business
  //     ....finished with I2C business
  //     I2CBusFailFlag=0x38; // reset flag to "working"
  //     }
  //     else // put in code to provide default value for pressure measurement
  //


  if (I2CBusAllowed == 1) {
    I2CBusFailFlag = 0x99; // set flag to discover a possible failure
    //----------------GLG set up the BMP280:
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    Serial.print("\n\nAbout to read BMx280   ");
    while (!ssenseBMx280.begin())
    {
      // Insert any code you wish to alert you that
      // the device could not be found.
#ifdef BMP280DEBUG
      Serial.print("BMx280 not found.");
      delay(250);
#endif

    }


//----------------------------BMP SETUP IF REQUIRED--------------------------
#ifdef BMP280
    switch (ssenseBMx280.chipModel())
    {
      case BME280::ChipModel_BME280:
        // insert any desired cocde to notify that BME280 with humidity detected.
        #ifdef BMP280DEBUG
        Serial.print("BME280(hum) " );
        #endif

        break;
      case BME280::ChipModel_BMP280:
        // insert any desired code to notify that BMP280 (no humidity) detected
        #ifdef  BMP280DEBUG
        Serial.print("BMP280(nohum) " );
        #endif
        break;
      default:
        // insert any code to notify that the unknown sensor found.
        Serial.print(" Error \n" );
        break;
    }

    ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);
    // If we made it to here, the I2C bus worked OK
    I2CBusFailFlag == 0x38; // reset the warning flag

    //--------------------------GLG

      } // End of I2C access portion
#endif



//=======================END OF BMP SETUP IF REQUIRED ==================================

  vent_init();
  //setupTouch();

  // ----------------Set up for FLOW MEASUREMENTS -------------------
#ifdef DEBUG
  Serial.begin(115200);          // Set the serial monitor
  Serial.println("\nFlowZero\n");
  //randomSeed(analogRead(A0));
#endif
  analogReference(DEFAULT);
  exhale            = false;
  lastMode          = false;

  
  

  // read the baseline voltage reading from the differntial pressure
  zerorawSensorValue = 0;
  for (int qq = 0; qq < 5; qq++) {
    zerorawSensorValue = zerorawSensorValue + ReadFlowPressureSensor();
  }
  zerorawSensorValue = zerorawSensorValue / 5;


#ifdef  FLOWBASELINE
  Serial.print ("Read baseline zero flow output\n");
  Serial.print (zerorawSensorValue);
#endif

  cTime = millis();
  // ------------end of FLOW MEASUREMENT SETUPS

  Serial.println("Setup done");
  //delay(500);
}

void loop() {

  //  Reset the watchdog timer.   If this isn't done every so often, the watchdog
  //  will automatically reset the hardware.   At that point we examine a non-initialzed
  //  variable to determine if the reset had to do with a I2C call failure.... and if
  //  it did, we will have to disable all calls to the I2C devices.
  //  For further information on the watchdog timer, see:
  //  https://forum.arduino.cc/index.php?action=dlattach;topic=63651.0;attach=3585
  //  or other such tutorials.

  wdt_reset();   // resets the watchdog timer


  vent_slice();
  alarm_slice();
  if (use_tft)
    tft_slice();
  else
    lcd_slice();
  //Serial.println(measure_pressure());
  // Ashhar had a delay of 100 msec here but I think that is WAY too long to
  // allow slices of 50 msec to be handled.   So I'll change it to 10 mSec
  // delay(5);
}

void  vent_start() {
  current_phase = 0;
  vent_running = 1;
}


//at times the motor/valve maybe stuck, try turning it off twice
void vent_abort() {
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);// get both valves open!!
  //  pressure_off();
  current_phase = 0;
  vent_running = 0;
  delay(200);
  //  pressure_off();
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);// get both valves open!!
}

// Watchdog Timer Code (courtesty Ashhar Farhan)
void setup_watchdog(void) {
  cli();
  wdt_reset();
  /*
    WDTCSR configuration:
    WDIE = 1: Interrupt Enable
    WDE = 1 :Reset Enable
    See table for time-out variations:
    WDP3 = 0 :For 1000ms Time-out
    WDP2 = 1 :For 1000ms Time-out
    WDP1 = 1 :For 1000ms Time-out
    WDP0 = 0 :For 1000ms Time-out
  */
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // Set Watchdog settings:
  WDTCSR = (1 << WDIE) | (1 << WDE) | (0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);
  // trying to set it for 2 seconds
  sei();  // R-enable the interrups

}
