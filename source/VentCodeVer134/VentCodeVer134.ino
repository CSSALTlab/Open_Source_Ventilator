
/*
 * GLG  5/31/2020  This is Verion 120b from Steve S -- and now being modified by GLG
 * Will call it 120c  when i'm done and then let Steve S upgrade the library portions to be up to date.
 * 122_SDS 5/20
 *  - Added version listing to the lcd menu. Set version in vent.h
 *  - Reverted back to ssense library due to failure of adafruit sensor to initialize with adafruit
 * library
 *  - Added menu item to select PEEP filter type

 121 SDS 5/13  
 -   Reduced cutoff smooth_pressure_run to allow assist breaths to occur sooner
 -   Added plateau and compliance measurements, activated from the menu
        Works by starting an insp hold and extending the duration of the the vent slices
        to allow lungs to settle.
  -  Set i2c flow sensor to use onboard averaging to offload the arduino. 
  - Changed i2c flow sensor read order so measurements occur in background and get
      called when needed. 
*/
//Version 120b  -- received from Steve S with multiple fixes he implemented to straighten out control of the valves
//
//Version 118  5/6/2020   Will add in measurments of compliance and resistance
//
//
// Version 117  5/4/2020   Fix the advancement of phase no. so it can't skip cut_off phase
//                              This version provided Tbottomlimit to count the total amount of
//                              opening of the expiratory valve in two different working routines:
//                              PID -- simple pid revision of the observed presusre to better guide
//                                     the algorithm
//                              ADAPTIVE2 -- a new algorithm that keeps the valve open for Tbottomline
//                                     loops (where each loop was measured at approx 23 milliseconds)
//                                     and more quickly, or more slowly adjusts Tbottomline up or down
//                                     and measures the result ONLY once in phase 18, to adjust
//                              Choice of the filter to use is by #defines in vent.h
//
// Version 116   5/3/2020  -- ASSIST now controllable from menu on/off 
//                              PAUSE now equal to 25% of inspiration
//                              ASSIST should be availablein either pause or non pause (not checkec)
//                              PID filter available (uncomment)
//                              ADAPTIVE filter selected
//                              FIR filter -- not working at moment
//                              Turn PEEP off by setting menu peep to 0 (disables it)
                              


// Version 115  5/2/2020 -- added in filter.cpp with a working adaptive filter.

// Version 112  4/30/2020   Gordon Gibby -- but usingShane S's code for the imprved lcd functions
//

//Version 111   4/30/2020   Gordon Gibby -- includes support for the MVXP7002DP analog digital flow
//                            transducer, using a fudge factor of 2 to make the expired flows appear
//                            reasonnably accurate.   Unable to explain that.....
//                            Also has working code for the I2C transducer....works like a charm
//                            Gordon's fixes to the user interface code with beeps to indicate I2C progress...
//                            Does not yet have Steve S's improvements to saves and rolling menu
//
//                            
// Version 1.6, Apr 15, 2020. Jack Purdum, W8TEE. flash: 23760, SRAM: 1088. Stopped duplication of string
//                            constants, moved includes to where they belong in vent.h, set toggle code for
//                            different displays (untested), moved function prototypes to vent.h, moved
//                            definition of symbolic costants to vent.h, implements #ifdef DEBUG on ALL 
//                            debug code. Set to DEBUG1 if that message not wanted.
// Version 1.0, Apr 15, 2020. Jack Purdum, W8TEE. flash: 20108, SRAM: 1377. All warnings and errors removed
// Apr 14, 2020. Review by Jack Purdum, W8TEE.    flash: 20656, SRAM: 1379

// Ashhar Farhan Code  From April 1 2020
// Modified to work with BMP280 by Gordon Gibby
// Gordon Gibby gives NO PERMISSION FOR ANY USE WHATSOEVER
// without written permission from himself for usage.
//


#ifndef BEENHERE
#include "vent.h"
#endif

// BMP280 library info: https://create.arduino.cc/projecthub/dragos-iosub/arduino-bme280-sensor-how-to-115560
// Another source:   https://groups.io/g/VentilatorDevelopers/message/1224




// ------UNITIALIZED DETECTION OF AN I2C BUS CALL FAILURE
// Following variable is uninitialized ( so may come up randomly )
// Code will set it immediatly to  I2C_READY to indicate normal operation, not involving I2C bus
// Same variable will be set to I2C_BUSY whenever entering a subroutine that has the possibility
// of "blocking" on a failed I2C call" -- then if the watchdog timer resets, and the
// setup() discovers that variable is I2C_BUSY (rare by random chance) the code will presume
// the I2C bus has failed, will notify the user, and will disable calls to the I2C bus
// and operate as well as possible without that information
// This protects the ventilator from failing due to an I2C bus failure
// GLG   04102020




// set i2c_allowed=0 to disable the bus

//  EXAMPLE of Handling I2C bus:
//  if(i2c_allowed==1){
//     i2c_status=I2C_BUSY;  // set the flag to indicate failure if reset happens
//     ....now do i2C business
//     ....finished with I2C business
//     i2c_status=I2C_READY; // reset flag to "working"
//     }
//     else // put in code to provide default value for pressure measurement
//

/*
#include <Wire.h>
#include <avr/wdt.h>            // required for the watchdog timer, see routines below.  
#include <BMP180I2C.h>       // Obsolete: replaced with BM280 on next include 
#include <sSense-BMx280I2C.h>   // Download link halfway down this page: 
// https://create.arduino.cc/projecthub/dragos-iosub/arduino-bme280-sensor-how-to-115560
#include <EEPROM.h>
#include "vent.h"

#define EEPROM_VENT_ON          48
#define EEPROM_BPM              52
#define EEPROM_PRESSURE         56
#define EEPROM_IERATIO          60
#define EEPROM_PAUSE            64
#define EEPROM_TV               68
#define EEPROM_PEEP             72


#define AVERAGEBINNUMBER        10      // Number of averaging bins for the averaging routine
#define PRESSURESENSORPIN       A6      // pin that the flow pressure sensor is attached to
#define FLOWMAXSCALE            90      //Flow rate design Parameter is 90 Liters/min
#define VSOURCE                 5.0     //Source voltage for differential pressure sensor
#define PTFCC                   40    // Guessing for mine....      63.639Conversion factor for this orifice.
#define MF                      1.0     //Meter factor for calibration
#define PRESSUREREADDELAY       20L     // wait 20 ms between reads

//#define INSP_PRESSURE_LIMITS
//#define AIRWAYPRESSURECHECKING
*/

//  For 4x20 display. 0x77 for the Adafruit 2x16 LCD display




//---------------------FLOW MEASUrEMENT SUBRoutINE  (Schmidt / Purdum from 4/5/2020 Version 4
//  Here are the routines and what they will do and return
//  TO USE ANALOG OUTPUT DIFFERENTIAL PRESSURE TRANSDUCER
//  #define ANALOGDIFFTRANSDUCER (in vent.h)  for the MVXP7002DP 2kPa analog transdcuer
//  to watch outputs, #defineANALOGSPY
//
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
//   There are two completely dfferent routines to calculate flow from the I2C-based differentia pressure transducer
//   which returns a float from its libary of routines.
//
//==================================== Global Definitions ==============================================
// moved these to vent.h

float  supply_voltage =   12.0  ;  // iitial value only!
uint8_t i2c_status  __attribute__ ((section (".noinit")));
uint8_t i2c_allowed;   // variable to track whether this bus is allowed

// Variables to track exact peep performancee





int alarm_status            = ALARM_OFF;
char alarm_array[6];          // allows to keep track of 6 different alarms
//  Each is 0  if no alarm there
//  Each is 0x01 if an alarm.
//  Get updated with each breath so transitory existence
//  alarm_array[0] refers to LOW PRESS
//  alarm_array[1] refers to HI PRESS
//  alarm_array[2] refers to LOW VOL
//  alarm_array[3] refers to HI VOL
//  alarm_array[4] refers to LOW VOLTAGE from supply
//  others are undefined at the moment

int atmospheric_pressure    = 0;          // in cmH2O  from the Airway Pressure Unit 
int atmospheric_pressure2   = 0;          // in cmH2O from the Ambient Pressure Unit
int diff_atmospheric_pressure = 0;     // measured difference sensor 1 - sensor 2
                                       // measured at beginning and at ZERO/RESET
byte actual_BPM;                        // Track the actual breath rate, accounts for assisted breaths.  SDS121
long breath_duration        = 0;                 // duration of each breath, used to calculate actual BMP SDS121
long breath_start_time      = 0;               //Time when new breath starts. used to calc duration --SDS121
 
int bargraph[MAX_PHASES];  // bargraph deprecated
int beats_per_minute        = 10;
byte desired_peep            =  5;          // what PEEP we want to happen.
byte vent_assist             = 0;          // 1 = assist; 0 = none     
byte peep_type; // PEEP filter type in menu 0 = Assist2/time cycled  1 = PID SDS 121  //TJ 06.15.2020 peep_type = 0 is ILC, peep_type = 1 is PID 
byte current_phase           = 0;
int current_pressure        = 0;
int cut_off;                              // phase at which we turn off the inspiration
int delP;                                 //Measured differential pressure sensor Flow Delta Pressure in Pa; Purdum/Schmidt
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
int TVhighlimit             = 700;        // high limit TV milliliters. See desired_TV above, too: Purdum/Schmidt
int TVlowlimit              = 300;        // low limit TV milliliters Purdum/Schmidt
int vent_running            = 2;
int vent_pressure_limit     = 40;         // Units are cmH2O (gauge)
int zerodelP;                             // measured delP at presumed zero flow  Purdum/Schmidt  (analog transducer)
int zerorawSensorValue;                   // baseline (analog) raw sensor output (at no pressure no flow); Purdum/Schmidt

unsigned long current_flow_time = 0 ; //  microseconds
unsigned long last_flow_time = 0;   //  microseconds

int  trigger_cm = 8;    // global variable for the depth of cm the patient must pull to get a breath; larger= harder to get assist

// SDS Plateau and compliance variables SDS121
byte compliance;
byte pause_state;
float plat_pressure;
int plat_count;
byte measure_plat;
byte do_measure_plat;
byte measuring_plat;
long save_next_slice;

//  VARIABLES for the digital flow sensor
float  DigitalFlowZero;           // the sensor output when no flow
float  RawDigitalPressure;        // what we get




unsigned long alarm_on_until         = 0UL;
unsigned long alarm_off_until        = 0UL;
unsigned long alarm_suppress_until   = 0UL;
boolean alarm_low_pressure;
boolean alarm_high_pressure;
boolean alarm_low_volume;
boolean alarm_high_volume;


long cTime;                               //time of last computations for the integrator
unsigned long next_slice             = 0L;         // Use 'L' for typoe suffix, not lowercase 'l'--looks like a 1 digit character
unsigned long now;
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
float current_exp_volume;                 // the volume in the most recent tiny slice (approx 23 mSec)

int smooth_pressure_run;          // counter for how tightly the expriatory pressure is kept
int tested_settled_peep=0;            // flag for testing the peep ONCE //TJ 06.15.2020 tested_settled_peep = 0 means not tested; tested_settled_peep = 1 means already tested, don't test again during current loop

//===================================== Define Objects ================================
#if I2CLCDDISPLAY == true
LiquidCrystal_I2C lcd(0x27, 20, 4);      // Define the 20x4 display object
#else
LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR
//LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //            16x2
#endif

//                                Settings for the BMP280 device:
BMx280I2C::Settings settings(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  BMP_ADDRESS                                 // I2C address LCD display
);

BMx280I2C::Settings settings2(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  BMP_AMBIENT                                 // I2C address LCD display
);


BMx280I2C ssenseBMx280(settings);   // for the airway pressure BMP280
BMx280I2C ssenseBMx282(settings2);  // for the ambient pressure BMP280



#ifdef USE_BMP180 // ########## SDS Changes for BMP180
BMP180I2C bmp180(BMP_ADDRESS);    // For the BMP180
#endif

AllSensors_DLHR_F05D_8 gagePressure(&Wire);


/*****
    Purpose: To delay so many milliseconds without blocking interrupts

  Parameter list:
    unsigned long ms      the number of milliseconds to delay

  Return value:
    void

*****/

/*****
  Purpose: Calculate the change in pressure, delP, from the sensor-generated voltage

  Argument list:
    void

  Return value:
    int   ranging from 0 to 1023

  CAUTION:
*****/
float ReadI2CFlowPressureSensor()
{
  // lets average three readings and try that.

  int rdp10;   // ten times the raw digital pressure, but an int
  RawDigitalPressure = 0.0;  // Float to deal with the digital output

  if (i2c_allowed == 1) // Can't even make themeasurement if not allowed....
  {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happens

      gagePressure.readData(true); // Read comes before measure. This will pull the measurement that was started
      // in vent slice or the previous time through this subroutine
      
      //then start a new measurement
      gagePressure.startMeasurement(0xAD); // the 0xAD tells the sensor to read and average 4 readings SDS
      
      if (!isnan(gagePressure.pressure))
      {
        RawDigitalPressure = gagePressure.pressure;
      }
    
    // if we got here, we did all of the measurements
    i2c_status = I2C_READY; // reset flag to "working"
  }  // end of if i2c_allowed
  // Now take out the zero bias
  RawDigitalPressure = RawDigitalPressure - DigitalFlowZero;
  rdp10 = (int) (10 * RawDigitalPressure);
  
  #ifdef FLOWMEASURE
  Serial.print("RawDP: ");
  Serial.println(rdp10);
  #endif


  
  return (RawDigitalPressure);  // Note returns a FLOAT

}

int ReadFlowPressureSensor()
{
  // This routine for the ANALALOG pressure transducer 
  // NOTE defines:
  //#define ANALOGDIFFTRANSDUCER   // if using the MVXP7002DP 2kPa analog transdcuer
  //#define ANALOGSPY  to see actual (0-1023) values measured
  // see very extensive information on this device in vent.h file 
  //
    // lets average 3 readings and try that.
  int q;
  
  
  rawSensorValue = 0;
  for (q = 0; q < 3; q++)
  {
    rawSensorValue        =  rawSensorValue + analogRead(PRESSURESENSORPIN);  // analogRead returns 0-1023 
  }

  rawSensorValue = rawSensorValue / 3;
  
#ifdef ANALOGSPY
  // rawSensorVaue is an int  
  Serial.print(F("Raw Analog (0-1023): "));
  Serial.println(rawSensorValue);
#endif


  //Raw digitized voltage input from pressure sensor  (0-1023 representig 0-5 volts) 
  //ALSO--
  //Calculate GLOBAL int variable, difference between raw 0-1023 and baseline zero flow value, delP
  delP =  rawSensorValue - zerorawSensorValue ; // all INTS and in units of 0-1023 
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
float CalculateI2CInstantFlow()
{
  //  Uses RawDigitalPressure and calculates the associated flow that produced it.
  //  output is global float instantFlowValue

#ifdef STRAIGHTEXPLIMB
  if (RawDigitalPressure > 145) {
    // stuck this in
    instantFlowValue = 0.19 * RawDigitalPressure + 37.0;  // GLG estimate
  } else if (RawDigitalPressure > 120.08) {
    instantFlowValue = 0.29417 * RawDigitalPressure + 34.5361;
  } else if (RawDigitalPressure > 86.29) {
    instantFlowValue = 0.366591 * RawDigitalPressure + 28.28686;
  } else if (RawDigitalPressure > 59.83) {
    instantFlowValue = 0.444917 * RawDigitalPressure + 23.6006;
  } else if (RawDigitalPressure > 38.68) {
    instantFlowValue = 0.617716 * RawDigitalPressure + 16.91676;
  } else if (RawDigitalPressure > 21.52) {
    instantFlowValue = 1.068004 * RawDigitalPressure + 7.22654;
  } else if (RawDigitalPressure > 12.55) {
    instantFlowValue = 1.11842 * RawDigitalPressure + 6.59382;
  }

 /*----------------------------I don't think these are important  
   else if (RawDigitalPressure > 3.43) {
    instantFlowValue = 2.25641 * RawDigitalPressure + 2.690513;
  } else if (RawDigitalPressure > 1.09) {
    instantFlowValue = 12.875 * RawDigitalPressure - 8.88375;
    }
*--------------------------------*/

  else {
    instantFlowValue = 0;
  }           // unit is liters/minute
#endif

#ifdef USHAPEEXPLIMB
if (RawDigitalPressure > 145) {
    // stuck this in
    instantFlowValue = 0.22 * RawDigitalPressure + 31.0;  // GLG estimate
  } else if (RawDigitalPressure > 124.6) {
    instantFlowValue = 0.235 * RawDigitalPressure + 30.96;
  } else if (RawDigitalPressure > 85.02) {
    instantFlowValue = 0.248 * RawDigitalPressure + 29.33;
  } else if (RawDigitalPressure > 52.14) {
    instantFlowValue = 0.297 * RawDigitalPressure + 25.19;
  } else if (RawDigitalPressure > 30.51) {
    instantFlowValue = 0.445 * RawDigitalPressure + 17.12;
  } else if (RawDigitalPressure > 12.39) {
    instantFlowValue = 0.619 * RawDigitalPressure + 12.14;
  } else if (RawDigitalPressure > 5) {
    instantFlowValue = 1.015 * RawDigitalPressure + 7.231;
  }
// I don't trust anything below differentpressure of 5 Pa
  else {
    instantFlowValue = 0;
  }           // unit is liters/minute

#endif



#ifdef DEBUG 
  Serial.print(F("\n RawDigP, Flow l/min)"));
  Serial.print(RawDigitalPressure);
  Serial.print(F("  "));
  Serial.print(instantFlowValue);
  Serial.print(F("\n"));
#endif



  return (instantFlowValue);   // change to millilliters!!!
}





// -------------ANALOG VERSION OF INTANT FLOW CALCULATION---------------------------------------------
//    requires global variabe delP  (int, 0-1023, differntialpressure having substrated the zero-flow number

float CalculateInstantFlow()
{
  

  // needs to receive global variable float delP in units of Pascals.   
  // convert from 0-1023 ints to pressure in Pascals
  // and switch to using global float RawDigitalPressure  to be compatible  with the I2C-based routine  
  // to convert to Pascals:     1023 measurements cover span of 5 volts
  // sensitivity of the MPXV7002DP device is 1mV = 1 Pa
  // see:    https://www.nxp.com/docs/en/data-sheet/MPXV7002.pdf

  RawDigitalPressure  =   (float) delP   * 2*   (5000  /* Pascals */ / 1023 ) ;  // GLG FUDGE FACTOR

  if (RawDigitalPressure > 145) {
    // stuck this in
    instantFlowValue = 0.22 * RawDigitalPressure + 31.0;  // GLG estimate
  } else if (RawDigitalPressure > 124.6) {
    instantFlowValue = 0.235 * RawDigitalPressure + 30.96;
  } else if (RawDigitalPressure > 85.02) {
    instantFlowValue = 0.248 * RawDigitalPressure + 29.33;
  } else if (RawDigitalPressure > 52.14) {
    instantFlowValue = 0.297 * RawDigitalPressure + 25.19;
  } else if (RawDigitalPressure > 30.51) {
    instantFlowValue = 0.445 * RawDigitalPressure + 17.12;
  } else if (RawDigitalPressure > 12.39) {
    instantFlowValue = 0.619 * RawDigitalPressure + 12.14;
  } 

  //With the limited resolution, values below 12 mV (or Pascal, they are the same) are suspect
  //else if (RawDigitalPressure > 5) {
  //  instantFlowValue = 1.015 * RawDigitalPressure + 7.231;
  //}
  // 
  else {
    instantFlowValue = 0;
  }           // unit is liters/minute

  

#ifdef ANALOGSPY
   int intRawDigitalPressure = (int) RawDigitalPressure;
  Serial.print(F("CalculateInstantFlow"));
  Serial.print(F("delP = "));
  Serial.print(delP);
  Serial.print(F("  intRawDigitalPressure = "));
  Serial.println(intRawDigitalPressure);
  Serial.print(F("Instant Flow:  "));
  Serial.println(instantFlowValue);
#endif

  return instantFlowValue;
}


void DigitalZeroDifferentialTransducer()
{
  if (i2c_allowed == 1) {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happens
    DigitalFlowZero = 0;  /// initialize it, we're going to average 4 measurement

        gagePressure.setPressureUnit(AllSensors_DLHR::PressureUnit::PASCAL);
        gagePressure.startMeasurement(0xAD); // SDS 122 change to use built in sensor averaging
        gagePressure.readData(true);
        
        if (!isnan(gagePressure.pressure))
        {
        DigitalFlowZero = gagePressure.pressure;
#ifdef DEBUG  
          Serial.print(F("\nDig Pres= "));
          Serial.println(DigitalFlowZero);
          MyDelay(500);
#endif
        }    

         MyDelay(50);

    i2c_status = I2C_READY; // reset flag to "working"

  }


return;
  
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
  EEPROM.put(EEPROM_PEEP, desired_peep);
  EEPROM.put(EEPROM_ASSIST, vent_assist); // SDS added entry for assist
}

void load_settings() {
  EEPROM.get(EEPROM_VENT_ON, vent_running);
  EEPROM.get(EEPROM_BPM, beats_per_minute);
  EEPROM.get(EEPROM_PRESSURE, vent_pressure_limit);
  EEPROM.get(EEPROM_IERATIO, ie_ratio);
  EEPROM.get(EEPROM_PAUSE, inspiratory_pause);
  EEPROM.get(EEPROM_TV, desired_TV);
  EEPROM.get(EEPROM_PEEP, desired_peep);
  EEPROM.get(EEPROM_ASSIST, vent_assist); // SDS added entry for assist
  
  if ( vent_running != 0) 
    vent_running = 4; 
  if (beats_per_minute < 0 || beats_per_minute > 30)
    beats_per_minute = 10;
  if (vent_pressure_limit < 5 || vent_pressure_limit > 60)
    vent_pressure_limit = 40;
  if (ie_ratio < 1 || ie_ratio > 4)
    ie_ratio = 1;
  if (inspiratory_pause != 1) inspiratory_pause = 0;
  if (desired_TV < 100 || desired_TV > 999) desired_TV = 700; // default TV
  if (desired_peep < 0   || desired_peep > 25) desired_peep = 5; // default peep
}

void update_status2(char *text) {
  lcd_status(text);
}
void log_message(char *text) {
  Serial.println(text);
}

#ifdef USE_BMP180   // ########## SDS Changes for BMP180

void init_bmp180() {
  //begin() initializes the interface, checks the sensor ID and reads the calibration parameters.

  unsigned long start = millis();

  while (true) {           // routine to initialize sensor...

    if (!bmp180.begin()) {   // Hasn't initialized yet...
      if (millis() - start > 2000UL) {    // If sensor doesn't init in 2 seconds...
        pressure_sensor_present = 0;
        lcd.print("Error: 180 sensor");   // Tell them bad sensor and go home
        return;
      }
    } else {
      break;
    }
  }

  //enable ultra high resolution mode for pressure measurements
  // bmp180.setSamplingMode(BMP180MI::MODE_UHR);
  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);
}
#endif

////////////////// Initiaization of the AMBIENT  bmp280/////////////////////////////

// @@@@@@@@@@  SDS changes

void init_bmp282() {
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  unsigned long start = millis();
  wdt_reset();
  if (i2c_allowed == 1) {

#ifdef DEBUGBMP280
    Serial.print(F( "\n\nWill read BMx280 amb  i2c_allowed = "));
    Serial.println(i2c_allowed);
#endif


    while (true) {  // routine to initialize sensor...
      if (!ssenseBMx282.begin()) {
        if (millis() - start > 2000UL) {   // If sensor doesn't init in 2 seconds...
#ifdef DEBUGBMP280
          Serial.print(F("No BMP AMB."));  // Tell them bad sensor and go home
#endif

          lcd.setCursor(0, 0);
          lcd.print(F("No init of BMx280 amb"));   // Tell them bad sensor and go home
          return;
        }
      } else {
        break;

      }
    }

  }

  /*
     while (!bmeAmb.begin(BMP_AMBIENT, &Wire)) {

        ++passCount;
        wdt_reset();
        MyDelay(250L);
        if (passCount > 16) {             // Try initializing it for 4 seconds
          wdt_reset();
          #ifdef DEBUGBMP280
          Serial.print(F("No BMP AMB."));
          #endif

          lcd.setCursor(0, 0);
          lcd.print(F("No init of BMx280 amb"));
          return;
        }
      }
  */

}


///////////////////////////////
void init_bmp280() {
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
      BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  unsigned long start = millis();
  wdt_reset();

  if (i2c_allowed == 1) {
#ifdef DEBUGBMP280
    Serial.print(F( "\n\nWill read BMx280 airway  i2c_allowed = "));
    Serial.println(i2c_allowed);
#endif

    while (true) {
      if (!ssenseBMx280.begin()) {
        if (millis() - start > 2000UL) {
#ifdef DEBUGBMP280
          Serial.print(F("No BMP airway"));
#endif
          lcd.setCursor(0, 0);
          lcd.print(F("No init AW sens"));
          return;
        }

      } else {
        break;
      }
    }
  }

  /*
    while (!bmeAirway.begin(BMP_ADDRESS, &Wire)) {

    ++passCount;
    wdt_reset();
    MyDelay(250L);
    if (passCount > 16) {             // Try initializing it for 4 seconds
      wdt_reset();
      #ifdef DEBUGBMP280
      Serial.print(F("No BMP airway"));
       #endif
      lcd.setCursor(0, 0);
      lcd.print(F("No init AW sens"));
      return;
    }
    }
  */

}

// @@@@@@@@@@  END SDS changes


void init_pressure_sensor() {
  //i2c_status = I2C_READY;  // ***COMMENT OUT FOR RELEASE***
  wdt_reset();
  if (i2c_allowed == 1) {

    i2c_status = I2C_BUSY; // set flag to discover a possible failure

#ifdef USE_BMP180  // ########## SDS Changes for BMP180
    init_bmp180();
#else
    init_bmp280();
#ifdef BMP280AMBIENT
    // initialize that one also
    init_bmp282();
#endif
#endif
    // If we made it to here, the I2C bus worked OK
    wdt_reset();
    i2c_status = I2C_READY; // reset the warning flag
    i2c_allowed = 1;
  }
  else {
#ifdef DEBUG
    Serial.print(F("init_pressure_sensor skipped the BMP's\n"));
#endif
  }

}

#ifdef USE_BMP180 //########## SDS Changes BMP 180

int measure_bmp180() {
  //start a temperature measurement
  if (!bmp180.measureTemperature()) {
    lcd_status((char *)"No Sensor");
    return (0);
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true.
  do {
    MyDelay(0);
  } while (!bmp180.hasValue());
  bmp180.getTemperature();

  //start a pressure measurement. pressure measurements depend on temperature measurement, you should only start a pressure
  //measurement immediately after a temperature measurement.
  if (!bmp180.measurePressure()) {
    Serial.println((char *)"Sensor Err2");
    return -1;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true.
  do {
    MyDelay(0);
  } while (!bmp180.hasValue());

  long p = (long)bmp180.getPressure();
  long m = ((102l * p) / 1000l);
  return m;
}

#endif

////////////////////////2nd BMP Measure ///////////////////////////////

// @@@@@@@@@@ SDS changes

int measure_bmp282()  {
   unsigned long testpressure;  // Since testpressure is a local variable, it can be the same for
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  //both sensors

  i2c_status = I2C_BUSY; // set the flag to indicate failure
  // will be reset at end of measure_pressure() fxn

  ssenseBMx282.read(fpressure, temp, hum, tempUnit, presUnit);
 // testpressure = (unsigned long) fressure;
  testpressure = fpressure / 98;  

#ifdef DEBUGBMP280
  Serial.print ("\n 2nd BMPAirway P cmH2O: ");
  Serial.print (testpressure);
  Serial.print ("\n");
#endif

  return (testpressure);
}


int measure_bmp280() {
unsigned long testpressure;
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  
  i2c_status = I2C_BUSY; // set the flag to indicate failure
  // will be reset at end of measure_pressure() fxn

  ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);
  
  testpressure = fpressure / 98;  

#ifdef DEBUGBMP280
  Serial.print ("\n Airway P cmH2O: ");
  Serial.print (testpressure);
  Serial.print ("\n");
#endif
  return (testpressure);

}

// @@@@@@@@@@ END SDS changes

///////////////////////////////////////////////////////////////////////////////////////


int measure_pressure2() {
  int m;
  if (i2c_allowed == 1) {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happen

#ifdef USE_BMP180  // ########## SDS Changes bmp 180
    m =  measure_bmp180();
#else
    m =  measure_bmp282();
#endif

    if (m == 0) i2c_allowed = 0; // stop using it
    else     i2c_status = I2C_READY; // reset flag to "working"
#ifdef DEBUGBMP280
    Serial.print(F("2ndPress cmH2O: "));
    Serial.println(m);
#endif
    return m;
  }
  else
    m = 999;
  return m;
}


/////////////////////////////////////////////////////////////////////////////////////
int measure_pressure() {

  int m;
  if (i2c_allowed == 1) {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happen

#ifdef USE_BMP180  // ########## SDS Changes bmp 180
    m =  measure_bmp180();
#else
    m =  measure_bmp280();
#endif

    // ******NEWS FLASH*****
    // I pulled the clock line and instead of failing, it
    // returned ZERO.
    //
    if (m == 0) i2c_allowed = 0; // stop using it
    else     i2c_status = I2C_READY; // reset flag to "working"

#ifdef DEBUGBMP280
    Serial.print(F("1stPress cmH2O: "));
    Serial.println(m);
#endif
    return m;
  }
  else
    m = 999;

  return m;

}

/////////////////////////////////////////////////////////////////////////////////
int loop_count = 0;

void pressure_on() {
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, HIGH); 
  is_pressure_on = 1;

  // If this is the start of a breath
  // mark the time and track duration of breaths. -- SDS121
  if (vent_running && current_phase == 0)
  {
    if (breath_start_time > 0) // Wait until breathing has started to begin measuring breath duration
    {
      breath_duration = (now - breath_start_time); //Calculate the time since the start of the previous breath
      calc_actual_BPM(); // Calculate the average and BMP
    }

    breath_start_time = now; // Mark the starting time of this breath

  }
}

// Added to seperate closing insp valve and controlling exp valve SDS120b
void stop_insp() {
digitalWrite(MOTOR_A, LOW);  // Stop the inspiration pronto!!!!  
}

//////////////////////////PRESSURE OFF //////////////////////////////////////
void pressure_off() {   // now implements both inspiratory pause AND PEEP

#ifdef DEBUG 
  Serial.print(F("\n i2c_allowed, i2c_status:  "));
  Serial.print(i2c_allowed);
  Serial.print(F(" ,"));
  Serial.print(i2c_status);
#endif

  
  if ( (i2c_allowed == 0)  || (desired_peep==0) || (peakinspiratorypressure < lowpressurelimit) ) {

    // This allows a "no peep" selection of desired_peep = 0;
    // and keeps the system functioning if the i2C bus has been disabled or the pressure line is blocked
    //or disconnected 
    // In this case---there is no pressure monitoring so no electronic control of peep
    // 
    digitalWrite(MOTOR_B, LOW);
    if(exp_valve_first_open==0){
        exp_valve_first_open = 1;  // note that it has opened
        exp_valve_first_open_time= millis();  // note the time it opened. 
    }

    
    // GLG0601  what about the case that we wish to allow assist...but we have set peep to 0?    
    // unless we set some other values, the peep will never be measured in this case, and assist won't work.
    if( desired_peep==0  && i2c_allowed==1   && (millis()>exp_valve_first_open_time + 200)  && exp_valve_last_closed==0  ) 
      {    
      Serial.println(F("EValve pseudo closed for PEEP"));
      exp_valve_last_closed = 1;  // GLG121a this got set to 0 at cut_off
      exp_valve_last_closed_time = millis();   // GLG121a time of closure
        }
    is_pressure_on = 0;
    return;
  }
  else {  // NOW we need to look at PEEP
    // Need to find the exact moment that the expiration valve first gets opened.
    // char not_opened_yet get set to 1 at current_phase==0;   gets set to 0 at first moment of opening and time opening recorded in 
    // unsigned long exp_valve_first_open_time  (milliseondss)
 
    #ifdef PEEPFILTERPRESSURESPY
    Serial.print(F("\npressure_off: peep_filter_pressure: "));
    Serial.print(peep_filter_pressure);
    Serial.println("");
    #endif

// SDS120b remvoed peakinspiratorypressure check since it's already done above
//peep_filter_pressure can only have values of desired_peep+5 or desired_peep-5. This value is set in filter.

    if (peep_filter_pressure > desired_peep){
      digitalWrite(MOTOR_B, LOW); // drop the pressure
      if(exp_valve_first_open==0){
        exp_valve_first_open = 1;  // note that it has opened
        exp_valve_first_open_time= millis();  // note the time it opened.  
        
      }
    }
// SDS120b remvoed peakinspiratorypressure check since it's already done above
    if (peep_filter_pressure <= desired_peep) 
    {
      digitalWrite(MOTOR_B, HIGH); //don't drop thepressure
      Serial.println(F("EValve closed for PEEP")); // GLG0531 -- has been VERY helpufl in debugging
     // GLG121a  -- if we have not previously closed it now we need to mark it
        if(exp_valve_last_closed==0)  // GLG121a
          {
          exp_valve_last_closed = 1;  // GLG121a this got set to 0 at cut_off
          exp_valve_last_closed_time = millis();   // GLG121a time of closure
          }

     
    }
    
  }  // end of the "else"  that we don't need to keep the expiratory valve closed....


  //-----------------------------------------------------------

  is_pressure_on = 0;

}

void measure_atmospheric_pressure2() {
  wdt_reset();
  // ########## SDS removed averaging since done in chip

   atmospheric_pressure2 = measure_pressure2();
#ifdef DEBUGBMP280
  Serial.print(F("Atm press2= "));
  Serial.print(atmospheric_pressure2);
#endif
  wdt_reset();
}
  
//////////////////////////////////////////

void measure_atmospheric_pressure() {
  //if pressure is on, turn it off and wait for 4 seconds for the lungs and compressor to deflate

byte previous_peep = desired_peep; //remember the peep SDS120b
desired_peep = 0; // set peep to 0 so pressure_off will actually drop the prssure

#ifndef BMP280AMBIENT
  if (is_pressure_on) {
    pressure_off();
    MyDelay(1000);  // MUST LET PATIENT EXHALE!!!
    wdt_reset();
    MyDelay(1000);
    wdt_reset();
    MyDelay(1000);
    wdt_reset();
    MyDelay(1000);
    wdt_reset();
  }
#endif

desired_peep = previous_peep; // set peep back to previous SDS120b

  atmospheric_pressure = measure_pressure();  // SDS122 Averaging peformed on sensor board
#ifdef DEBUG
  Serial.print(F("Atm press= "));
  Serial.print(atmospheric_pressure);
#endif
}

/*
   This has to be tweaked
*/

void check_pressure_limits() {

  if (i2c_allowed == 0) {
    peakinspiratorypressure = 0; // dummy pressure
    Serial.print(F("No Airway Press Poss\n"));
    return;
  }

  //First check for over-pressure

  // run a comparison to build up the peak inspiratory pressure
  // will have to zero this at the beginning of each respiratory cycle
  // somewhere???
  if (current_pressure > peakinspiratorypressure)
    peakinspiratorypressure = current_pressure;


  /*----------------HIGH PRESSURE ALARM CHECKING-----------------------*/
  if (peakinspiratorypressure > vent_pressure_limit)  {  // check against the real pressure limit
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);// get both valves open!!
    // open expiratory valve and close inspiratory valve
    Serial.print(F("\nHIGH PRESSURE\n"));
    alarm(ALARM_FAST);
    alarm_high_pressure = true;
    update_status2( (char*) "HI");
    alarm_array[1] = 1; // set the alarm array
  }
  else {
    alarm_high_pressure = false;
    update_status2( (char*) " ");
    //   Not sure when to reset this at present....
    //   May not be triggered until slice 10....
    //   needs to SHOW!!!   before it gets reset
    //   alarm_array[1]= 0;  // reset the alarm array high pressure entry
  }
  
#ifdef DEBUG 
  Serial.print(F("\ncurrent_phase: , cut_off"));
  Serial.print(current_phase);
  Serial.print(F(","));
  Serial.print(cut_off);
  Serial.print(F("\ncheck pres limits\n"));
  Serial.print(F("cur_pres, peak meas, lowlimit "));
  Serial.print(current_pressure);
  Serial.print(F(" "));
  Serial.print(peakinspiratorypressure);
  Serial.print(F(" "));
  Serial.print(lowpressurelimit);
  Serial.print(F("\n "));
#endif


  //check only if the current phase is at the end of inspiration
#ifdef DEBUG 
  Serial.print(F("current_phase = "));
  Serial.print(current_phase);
  Serial.print(F("cut_off  "));
  Serial.println(cut_off);
#endif

  if ( current_phase == (cut_off + 1)) {

    // first check for failure to reach low pressure limit
    /*------------------LOW PRESSURE ALARM CHECKING ---------------------*/
    // We can't check for this if there is no i2c bus working.....

    if(i2c_allowed==1)
    {
    if (peakinspiratorypressure < lowpressurelimit)
    { // we are at the end of inspiration and never saw an acceptable pressure
      alarm(ALARM_FAST);
      update_status2( (char*) "LO");
      alarm_low_pressure = true;
      alarm_array[0] = 1;  // 0 slot is for LO PRESS

#ifdef DEBUG 
      Serial.print(F("current_phase = "));
      Serial.print(current_phase);
      Serial.print(F("LO ALARM\n\n"));
      Serial.print(F("Pk Insp P: "));
      Serial.print(peakinspiratorypressure);
      Serial.print(F("Lo P Lim: "));
      Serial.println(lowpressurelimit);
#endif
    }
    }  // end of if i2c_allowed test
    
    if (peakinspiratorypressure > lowpressurelimit) {
      alarm(ALARM_OFF);
      update_status2( (char*) "ON");
      alarm_low_pressure = false;
      alarm_array[0] =  0;    // zero the inspiratory alarm

#ifdef DEBUG 
      Serial.println(F("No low pressure\n"));
#endif
    }
  }  // end of checks on inspiratory low and high pressure

  /*---------------GLG I don't understand --------
    else if (pressure_peak < (pressure * 12)/10){
    alarm(ALARM_OFF);
    update_status2( (char*) "ON");
    --------------  */
}

// %%%%%%%%%% SDS code for actual BPM (to account for spontaneous breaths
#define bpm_span_size   5 // Number of breaths to average

long bpm_span[bpm_span_size]; // Index to hold breath durations for averaging
int bpm_INDEX = 0;            // Size of index
long bpm_SUM;                 // Sum of breath durations in index
byte bpm_count;               // track number of breaths for averaging

void calc_actual_BPM()   // -- SDS
{

  // This code makes an index of the last n number breath durations (set by defining bpm_span_size)
  //each time a new duration is added the oldest value is discarded

  bpm_SUM = bpm_SUM - bpm_span[bpm_INDEX];    // subtract the oldest duration from the total
  bpm_span[bpm_INDEX] = breath_duration;      // Add the newest duration to the index
  bpm_SUM = bpm_SUM + breath_duration;        // Add the newest duration to the total
  bpm_INDEX = (bpm_INDEX + 1) % bpm_span_size; // Step the index forward & wrap back to 0 when reaches end of index


  // If we've just started and don't have all n breaths in the index, then average the number
  // of breaths we do have.
  if ( bpm_count < bpm_span_size)
  {
    bpm_count++; // Track the number of breaths
  }

  // SUM / span_size = mean breath duration (total breath duration / number of values)
  // this is then cast as a float rather than long
  // average breath per minute = 1 min / duration of breath, rounded to full integer
  actual_BPM = round(60000 / static_cast<float>(bpm_SUM / bpm_count));

  /*
    Serial.print("Breath Num: ");
    Serial.println(bpm_count);
    Serial.print("Breath Duration: ");
    Serial.println(breath_duration);
    Serial.print("Sum: ");
    Serial.println(bpm_SUM);

    Serial.print("Measured BPM: ");
    Serial.println(actual_BPM);
    Serial.println();  */

}

// %%%%%%%%%%%%%%% End BMP measurement


//The following is test code made by Travis Johnson 6/11/2020
//It is an attempt made to move from "vent_slice" phase and to clock/time based events.
bool active_inspiration = 0;
bool inspiration = 0;
bool inspiratory_pause = 0;
bool exhalation = 1;

int total_breath_time; //in ms
int active_inspiration_time;
int inspiratory_pause_time;
int inspiratory_time;
int exhalation_time;
int exhalation_time_from_PEEP;

void vent_Timer() {
	total_breath_time = (60 * 1000) / beats_per_minute;
	inspiratory_time = total_breath_time / (1 + ie_ratio);
	exhalation_time = total_breath_time - inspiratory_time;

	if (inspiratory_pause) {
		active_inspiration_time = inspiratory_time * 0.75;
	}
	else {
		active_inspiration_time = inspiratory_time;
	}
	/*
	Serial.print(F("\nTime Based Vent (6.11.2020):  "));
	Serial.print(F("total_breath_time"));
	Serial.print(total_breath_time);
	Serial.print(F(" "));
	Serial.print()
	*/

	int startTime = millis();
	int currentTime = millis();
	int current_breath_time = 0;
	while ((millis()- startTime) < total_breath_time) {
		
		if ((millis() - startTime) < active_inspiration_time) {
			digitalWrite(MOTOR_A, LOW);
			digitalWrite(MOTOR_B, HIGH); //is it low open/ high closed for both valves?
		}
		else {
			if ((millis() - startTime) < inspiratory_time) {
				digitalWrite(MOTOR_B, HIGH);
			}
			else {
				digitalWrite(MOTOR_A, HIGH);
				digitalWrite(MOTOR_B, LOW);
			}
		}
		startTime = millis();
	}
}




void vent_slice() {
int p;  // to allow pressure to be measured


// SDS120b Addition to have seperate variables for insp valve closure and exp valve opening
// cut_off represents the beginning of exhalation. If there's no inspiratory pause, then cut_off
//also represents the end of inspiration, however, if there is a pause, then we need a seperate variable
//to mark closing of the inspiratory valve.
// GLG0531 -- note that both of these are phase counts, and that end_insp will 
// generally be <- the beginning of exhalation (cut_off);   

// 1F -- calculation of cut_off, without PAUSE considered yet...  GLG0531
cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1); // Calculate cut_off
cut_off = cut_off - 1; //TJ 6.11.2020
/*--------meaning of cut_off-------------*/
// The phases that are part of inspiration are 
// 0,1,2,3.....cut_off-1     It does not incude the phase cut_off
// Add these up and you get n = cut_off phases
// At the first edge of the phase==cut_off....you close the inspiratory valve
// and if there is no PAUSE you also open the expiratory valve 
//

  byte end_insp = cut_off; // Save the value for insp valve closure since cut_off could be modified by the
  // if statement below
// if insp pause enabled, modify cut_off
if (inspiratory_pause == 1)
{ 
	cut_off = cut_off + cut_off / 3; //TJ 6.11.2020 changed division from 4 to 3. Verification to be done 6/12/2020
}

// cut_off_pause_time is used to apply uniform adjustments to pause duraton throughout the code
// in places where we need to know inspiratory valve closure

// 2F -- measure pressure  GLG0531

  // SDS added this to account for diff_atmospheric_pressure, otherwise diff isn't taken into
  //account until the end of the breath
#ifdef BMP280AMBIENT
  atmospheric_pressure = atmospheric_pressure2 + diff_atmospheric_pressure;
#endif

  current_pressure = measure_pressure() - atmospheric_pressure  ;    // MEASURE AIRWAY PRESSURE TJ 6.15.2020 - current_pressure is GAUGE pressure
   

  // THE FIRST PART of this subroutine gets carried out many, many times
  // during the expiratory phase.....
  // the latter half, is ONLY carried out when we reach the time boundary at the end of
  // one slice and the beginning of the next.

// 3F -- return if moot   GLG0531
  if (!vent_running) {
    alarm(ALARM_OFF);
    return;
  }

// 4F -- and Steve, as I, has eliminated the redundant caluclation  GLG0531
  // remember, "cut_off" is a PHASE (or or slice) number....out of 20 currently  (MAX_PHASES=20)
  

/*------------5F-------INITALIZE STATE VARIABLES  GLG0531  -------------------------------*/

  // Set up a counter to keep track of how settled the expiratory pressure is--do it right at the transition
  if (current_phase == cut_off) {
    total_exp_valve_open_time=0;   // use as a counter of total time spent with exp valve open
    tested_settled_peep = 0;      // set the flag so we will test the peep ONCE
    loopcounter=0;   // havent made any loops through the expiratory limb;   
    smooth_pressure_run = 0;
    #ifdef I2CDIFFTRANSDUCER
    gagePressure.startMeasurement(0xAD); // Send message to take start first flow measurement SDS
    #endif
    last_flow_time = micros();   //  microseconds-- get a baseline for this cycle to start
  }                               // remember this portion of code is done many many times, not just at the end of a slice
                                  // so it will be done THROUGHOUT the timeslice cut_off....
                                  // and exhalation begins onthe cut_off+1 slice

/*---------------------BPM Measurement-------------------------------*/

if (do_measure_plat == 1 && current_phase > cut_off + 2) // Allow part of the pause time for the lung to settle, then begin
                                                                 // measuring and averaging pressure SDS121
{
  plat_pressure += current_pressure; // Add current pressure to the total
  plat_count++; // Track how many values are being added
  measure_plat = 2; // At the end of the this exhale, we will have data to print below
}


if (do_measure_plat == 1 && current_phase > cut_off + 3 ) // Done measuring plat  -- SDS121
{
  plat_pressure = round(plat_pressure / plat_count); // Average all the values collected
  do_measure_plat = 0; 
  inspiratory_pause = pause_state; // Reset insp pause to the state before we measured the plat 
  plat_count = 0; // Reset the counter
  measuring_plat = 0; // reset to zero for next measurement. This will also allow slice_time to return to normal
  next_slice = save_next_slice; // This allows us to return to normal timing on current slice before it's calculated again
}

/*---------6F----------CHECK FOR OVERPRESSURE --------GLG0531-------*/
                                  

/*---------7F ----------TRIGGER ASSIST BREATHING SECTION -----------*/

// modified for new cut_off SDS120b
// GLG0531 looks fine
  if ((current_phase >= cut_off) && (vent_assist==1) ) { // we only look for inspiratory effort during expiration
  // GLG0531 - rewritten so calculations of the smooth_pressure_run 
  // can begin right at the beginning of true expiration. 
    
      // WE ARE IN EXHALATION
      // GLG0601 
        #ifdef ASSISTSPY
        Serial.print(F("\nSMR,i2C,tested,decision:  "));
        Serial.print(smooth_pressure_run);
        Serial.print(F(" "));
        Serial.print(i2c_allowed);
        Serial.print(F(" "));
        Serial.print(tested_settled_peep);
        Serial.print(F(" "));
        Serial.println(current_pressure - desired_peep - trigger_cm);
        
        #endif
   
    if(   smooth_pressure_run>2  &&   i2c_allowed==1  && (tested_settled_peep==1) )   // GLG121a   new criteria for being willing to look for patient effort....
   
                                                      // calibrated for approx 60 milliseconds of steady expiratory presure
      // no further need for checking run lengths
      // can't fiddle with Assisted breaths if the airway pressure isn't possible
    {
      // we are now in the plateau portion of the expiration, so quit working on the pressure.....
      // search for an inspiratory breath.
      // TRIGGER IS NOW ADJUSTABLE -- higher the trigger_cm the more difficult to trigger

      if ( current_pressure <  (desired_peep - trigger_cm) )  { // this adjusts for peep 
        #ifdef ASSISTSPY
        Serial.print(F("\n\n**ASSISTED**\n\n"));
        #endif
       // jam us to the end of this slide immediately
        current_phase = 0; // force us into the beginning of an inspiratory pressure state
        next_slice = millis() - 1  ; // force us into a decision point (end of time slice)
      }

    }
    else {
      // try to get past that 30 milliseconds of solidly steady pessure....
      // 04282020 -- originally, fast cycle measuremnts through the expiratory loop were being made every 3 milliseocnds
      //             but with the addition of triplicate FLow measuremtns (of differential prssures across obstruciton)  that is slowed down 
      //             to about every 23 milliseconds.   So I need to change the requirement from a run of 10 within this limit (30 milliseconds)
      //             to more lke 3.....
      //
      if (current_pressure > (desired_peep + 7) || (current_pressure < (desired_peep - 7)) ) smooth_pressure_run = 0; // zero if way out of bounds.
      if ((current_pressure > desired_peep - 4) && (current_pressure < (desired_peep + 4) )  )  smooth_pressure_run++; // increment if tight control
    }

#ifdef SMOOTHPRESSURERUN
    Serial.print(F("\nSmPressRun: "));
    Serial.print(smooth_pressure_run);
#endif

  }  // END of the ASSIST VENTILATION code  

/*--------------------------------END OF ASSIST VENTILATION-----------------------------------------*/

/*---------6F -- in my original postion ----------------------------*/

// Make Quick Check on overpressure
if (current_phase < cut_off)
    // GLG0531 with the redefinition of cut_off to be uniquly when the 
    // expiratory valve opens, there is no longer a need to add extra 
    // phases to cut_off   
  {
    if ( current_pressure > vent_pressure_limit)  {  // check against the real pressure limit
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);// get both valves open!!
    // open expiratory valve and close inspiratory valve
    Serial.print(F("\nHIGH PRESSURE\n"));
    alarm(ALARM_FAST);
    alarm_high_pressure = true;
    update_status2( (char*) "HI");
    if(current_phase< (cut_off-2) )current_phase++;  // jump ahead!!
                                                    // but never skip important phase cut_off
    alarm_array[1] = 1; // set the alarm array
    }

  }  // end of checking during inspiration for over pressure 


/* ---------8F---------------PEEP CONTROL--------------------*/

  // ATTEMPT PEEP CONTROL------------------------------------------------

// GLG0531  Steve's simplifiation here due to new definition of cut_off is correct

   
  if ( current_phase >= cut_off) { // Note that if we have jammed an inspiratio to start, this will be avoided
    // GLG0531 -- exhalation begins at the start of current_phase==cut_off, not
    // at the start of cut_off+1
    // current_pressure = measure_pressure() - atmospheric_pressure;  // already got tht done above!! doesn't need repeating
    // IF WE ARE IN HERE:  it is time to open the expiratory valve....
    #ifdef FILTERSPY
    Serial.print(F("\n\nBefore filter: "));
    Serial.println(current_pressure);
    
    #endif

    // to use the filters, keep these lines uncommented
    // Note that we use the simpler PIDfilter if inspiratory_assist is available, because it
    // doesn't need to see the pressure long ways into expiration
    // The TimeCycledFilter DOES need to see a steady pressure there, which 
    // a person breathing would mess up
    //

    //////////////////   FILTERS    ////////////////////////////////////////////////////////////
    
    if(peep_type==0) peep_filter_pressure = TimeCycledFilter(current_pressure); // SDS121 allows filter to be set from menu //TJ 06.15.2020 peep_type is the type of control (PID,ILC) used to control PEEP. 
    else peep_filter_pressure = PIDfilter(current_pressure);
    
 /*   Not needed, already occurs in pressure_off() SDS120b
   if (exp_valve_first_open == 0)   // we are here for the first time....
      {
      exp_valve_first_open =1;  // set the market that we opened it
      exp_valve_first_open_time=millis();   // unsigned long to keep that time
      }
*/
    
    // DONT MESS WITH current_pressure!  that way overpressure tests don't get misled..... 
    
    
    #ifdef FILTERSPY
    Serial.print(F("current_phase:"));
    Serial.println(current_phase);
    #endif

  // EXPIRATORY FLOW MEASUREMENTS --------------------------------------
    
    DoExhaledTidalVolume();  // this will get done MANY TIMES per slice!!
  
  }


  /////-------------------------DIVIDING LINE IN THIS SUBROUTINE ------------------------------------
  /////  THINGS ABOVE get done every run through (approixmately every 23 milliseconds with 4 current I2C measurements
  //     THINGS BELOW get done only on a time_slice interval
  

  /*------------------HERE WE STOP UNLESS AT A TIME SLICE ENDPOINT-------------------------------*/
  // Note that it may go through many times before it ever goes any further.

  if (next_slice > now) {    // TEST LOAFING TIME
    //  With MAX_PHASES (number of slices to a ventilatory cycle) at 20, system printed 8 dots at resp rate of 28
    //  With MAX_PHASES (number of slices to a ventilatory cycle at 60....  did not print any dots MA -- unable to keep up.
    //  So the maximum MAX_PHASES isn't that much....
    return;
  }


  // GLG121a If we are HERE, then we are at the beginning of a new slice (phase).
  // (In other words, now is greater or equal to next_slice )
  // GLG0531:  next_slice gets set forward in time on the first run through
  // this portion of code, and thus it won't happen except on the leading edge
  // of a new phase.  
  #ifdef NEXTSLICESPY
  Serial.print(F("Slow section, current_phase: "));
  Serial.print(current_phase);
  Serial.print(F("  now: "));
  Serial.println(now);
  #endif
  
  
 // cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);   // no longer needed here, already done above SDS120b

/*-------1D----------------Pressure limit checking -- both high and low --------------------------*/  

  check_pressure_limits();  // check this at the end of every slice

/*-------2D--------------STATE VARIABLES INTILIALZTION AT PHASE 0--------------------------------*/

  // Zero the peak inspiratory pressure if this is our very first time
  // in this cycle.
  if (current_phase == 0) {
    peakinspiratorypressure = 0;
    // GLG0531 -- have to get next_slice actually linked in real time with now!!!
    next_slice = now + (60000UL / ((unsigned long)beats_per_minute * (unsigned long)MAX_PHASES) )  ;  
    // zero two of the alarm values
    alarm_array[0] = 0; // zero the low pressure alarm, which gets displayed LATE
    alarm_array[1] = 0; // zero the high pressure alarm, which gets displayed LATE
     // preset the variable to track when the expiratory valve first opens
    exp_valve_first_open = 0;
    
    
    //  If i2c isn't working, there are few alarms we can measure, so give a short done to tell we are ventlating anyway at 400 Hz
    if(i2c_allowed==0) {
      tone(ALARM_PIN,400,250);   // 1/4 second
      wdt_reset();
      }
    
  } // for 0th phase

// SDS120b
 if ( current_phase >= end_insp ) {
      stop_insp();
 }

/*------3D------------------------CHECK SUPPLY VOLTAGE HERE, IN INSPIRATION, MAXIMUM LOAD ON SUPPLY --------------*/

// Check the supply voltage in phases 2 and 3
#ifdef VOLTAGEDIVIDER
  if(current_phase ==2 || current_phase==3)  // we check the supply voltage here, because it is UNDER LOAD
  {
  // check the voltage.
  supply_voltage = (analogRead(SUPPLYVOLTAGESENSORPIN)) *.00489/DIVIDER;  // 5volts/1023 slots =  .00489 per slot  Typically A7
  if(supply_voltage < LOWVOLTAGELIMIT){
        alarm_array[4] = 1;
        alarm(ALARM_SLOW);
        }
  else alarm_array[4] = 0;
  
  }

#endif

/*-----4d--------- ACTUALLY CAUSE AIR TO GO INTO THE PATIENT --------------------------*/

  // If we are in inspiration....
  // GLG0531 - the criteria here will likely get changed such that if 
  // a high pressure alarm has been set, during this cycle, we do NOT
  // restart an inspiration...
  if ((current_phase < end_insp)  && (peakinspiratorypressure < vent_pressure_limit ) )  {
    pressure_on();     /// THIS IS WHAT IS TURNING THE PRESSURE BACK ON DESPItE AN ALARM!!!
                       // but allows pressure to be off during insp pause SDS120b
    exLitersPerCycle = 0 ; // initalize for  computations next exhalation
  }
  //  During the expiratory phase we need to measure
  //  1)  differential pressure across an obstruction
  //  2)  compute the flow rate
  //  3)  multiply by the time slice to get volume in that breath
  //  4)  keep accumulating it.
  //  5)  zero it out during inspirations.

/*-------5D-----------------STATE VARIABLE INITIALIZATIONS AT CUT OFF------------------------*/


  // mark that we have not yet finally closed the valve in expiration for ADAPTIVE2
  // GLG0531 -- I'm going to change this initialization to occur 1 phase before
  // actual beginning of inspiration because the fast code above would 
  // have already been run once and that might be an issue....
  if(current_phase==cut_off-1) {
    exp_valve_last_closed = 0;    // 1 or greater  = we closed the valve in expiration to maintain peep and will not reopen it
    exp_valve_first_open= 0;
    wastedloops= 0;
    //loops_since_major_jump=0  ;   // track when we made a major jump in the exp open time 
                                  // GLG0602  I think you can only set this to 0 at the beginning on vent_on...
                                  // not on every breath, because if we are at condition whee valve just isn't closing,
                                  // it won't get checked but ONCE A BREATH....
  }

if (do_measure_plat == 1 && current_phase == cut_off){
  measuring_plat = 1;
}

  /*-----------6D------------actually open the expiratory valve (or at least consider)------------------*/
  
  if (current_phase >= cut_off) {
                      // pressure first possibly gets turned off at slice cut_off +1
    pressure_off();   // Note that pressure_off() now correctly handles both inspiratory pause AND PEEP
    // we are in EXHALATIION....should be no additional INSPIRATORY FLOW...



#ifdef DEBUG  
    Serial.print(F("Accumuating exLitersPerCycle"));
    Serial.println((long) exLitersPerCycle );
#endif
  }  // end of things to do AFTER cut_off (hence in exhalation)



  // this is what sets the TIME WIDTH of a slice --
  // the 60 000 l   is 60 thousand (LONG)  milliseconds
  // at 10 breatghs per minute and MAX_PHASES = 40, each slide is 150ms
  // at 30 breaths per minute, it would be 50 msec


//GLG0531 -- I'll move this to later  -- way down at the end of the subroutine
// next_slice = next_slice + 60000L / ((long)beats_per_minute * (long)MAX_PHASES);



  /*
    #ifdef TIMESLICE-DISPLAY
    Serial.print("Now, nextslice: ");
    Serial.print(now);
    Serial.print("  ");
    Serial.print(next_slice);
    Serial.print("\n");
    #endif
  */

  
  // SDS Removed    P = measure pressure() since we can just call it directly from the equation

// Why are we calculating pressure again, we are already doing it at the beginning of every slice
 
 // bargraph[current_phase] = (int)(p - atmospheric_pressure);  // I don't think we need the bargraph
  bargraph[current_phase] = current_pressure;
//  current_pressure = bargraph[current_phase];

  //current_pressure = (int) (p- atmospheric_pressure);// calcuate the guage pressure
  pressure_total += current_pressure;
  lcd_graph_update();

/*-------------7D -- moved to here to be more appropriately placed  ADVANCE PHASE ---------------*/
// GLG0531 -- Steve has my earlier coding position here   
// GLG0531 -- moved it to later so that phase only gets advanced at the end of the entire list of items to be checked. 
  
 
/*---------8D-------------Zero certain alarm variables just before the end of the respiratory cycle--------------------*/



  // RESET the volume alarms only just befoe the end of the cycle so they show for a cycle
 if (current_phase == MAX_PHASES - 1) {
    alarm_array[3] = 0; // reset the HI VOL alarm just before it will get caluclated again
    alarm_array[2] = 0; // reset the LO VOL alarm just before it will get calculated again
  }

/*-------9D--------------CONCLUDE AND MOVE TO NEXT CYCLE------------------------------------------------------*/


 if (current_phase >= MAX_PHASES - 1) { //   ***end of the entire respiratory cycle***  (minus 1 to account for exhalation - TJ)

 // So, for example, if MAX_PHaSES is 20, we will execute respiratory cycle for phases 0 through 19
  // at phase 20 -- and at the first moment of phase 20 we will end up HERE
  // we want to get all endof cycle items done and cut this short because there have already been 20 
  // fully carried out slices BEFORE now.

   #ifdef CYCLEDIAGNOSTICOVERVIEW
      Serial.println(F("\n******CYCLE OVERVIEW ********"));
      Serial.print(F("diff_atmospheric_pressure, atmospheric_pressure = "));
      Serial.print(diff_atmospheric_pressure);
      Serial.print(F(" ,"));
      Serial.println(atmospheric_pressure);
      Serial.print(F("Tbottomlimit = "));
      Serial.println(Tbottomlimit);
      Serial.print(F("Desired PEEP: "));
      Serial.println(desired_peep);
      Serial.print(F("loopcounter: "));
      Serial.println(loopcounter);
      Serial.print(F("Curr press end of cycle: "));
      Serial.println(current_pressure);
      Serial.println(F("-------------------------"));
      #endif


   
      #ifdef BMP280AMBIENT
      measure_atmospheric_pressure2();   // measures the ambient pressure in the 2nd unit
      atmospheric_pressure=atmospheric_pressure2 + diff_atmospheric_pressure;
          #ifdef AMBIENTPRESSURESPY
          Serial.print(F("diff_atmospheric_pressure, atmospheric_pressure = "));
          Serial.print(diff_atmospheric_pressure);
          Serial.print(F(" ,"));
          Serial.println(atmospheric_pressure);
          #endif
      #endif
    
    exmLPerCycle = (int) exLitersPerCycle; // note exmLPerCycle= INT

     if (measure_plat ==1) // If measure plat chosen in menu, wait till end of current breath, then activate do_measure_plat -- SDS
    {
      do_measure_plat = 1;
      pause_state = inspiratory_pause; // Remember the current inspiratory pause state so we can reset it later
      inspiratory_pause = 1; // Turn on insp pause
      plat_count = 0; // Set the counter
      plat_pressure = 0; // Zero out the measured plat
    }

    if (measure_plat == 2) // This was set to 2 when plat measurement was finished -- SDS121
    { 
      compliance = exLitersPerCycle / (plat_pressure - desired_peep); // Compliance = delta volume / delta airway pressure
      measure_plat = 3; // 3 activates tracking of display
      lcd.setCursor(0,2);
      lcd.print("Plat "); 
      lcd.print(plat_pressure);
      lcd.print(" Comp ");
      lcd.print(compliance); // Display measurements

      Serial.print("Plat Pressure: ");
      Serial.println(plat_pressure);
    }
    
    if (measure_plat == 3)  // Allow values to be displayed for x number breaths, then clear -- SDS121
    {
      plat_count++; // Reuse counter
        if (plat_count >= 3) // Allow measurements to display for 3 breaths
        {measure_plat = 0; // reset so it's ready for the next measurement
         plat_count = 0;
         lcd.setCursor(0,2);
         lcd.print("                 ");
         }    
    }

    #ifdef FLOWMEASUREBREATH
    Serial.print(F("Last CycleVol:  "));
    Serial.println(exmLPerCycle);
    #endif

    
    if (exmLPerCycle < 0) exmLPerCycle = 0; // dont allow negative numbers

    // Check tidal volume limits
    if (exmLPerCycle > (desired_TV + 100)) {
      alarm(ALARM_SLOW);
      update_status2( (char*) "V^");
      alarm_high_volume = true;
      alarm_array[3] = 1; // set the alarm array
      alarm_array[2] = 0; // can't be low if HIGH
    }
    else {
      alarm_high_volume = false;
      //alarm_array[3]=0;  // beset just before MAX PHASES
    }

    if (exmLPerCycle < (desired_TV - 100)) {

      // we can't really measure any of this if we are using i2c flow transducer and it isn't working...

      #ifdef I2CDIFFTRANSDUCER
      if(i2c_allowed==1) {
          alarm(ALARM_FAST);
          update_status2( (char*) "Vv");
          alarm_low_volume = true;
          alarm_array[2] = 1; //set the alarm array for low volume
          }
     #endif

     #ifdef ANALOGDIFFTRANSDUCER
          // we should still be able to test for this and thus the low volume alarms are useful
          alarm(ALARM_FAST);
          update_status2( (char*) "Vv");
          alarm_low_volume = true;
          alarm_array[2] = 1; //set the alarm array for low volume

     #endif
    }
    else
    {
      alarm_low_volume = false;
      //alarm_array[2]= 0;  // reset just before MAX_PHASES
    }

    lcd_graph_clear();
    current_phase = 0;  // <<--- THIS is what causes us, on the first time through here at 
                       // current_phase==20 to get our work done (beginning of that slice) and go immediately to the beginning
                       // of the next respiratory cycle-- not waiting for the entire width of the slice
    // DONT increment next_slice -- inestead let the code run back through this
    // and do it on THAT newly begun respiratory cycle,
    // starting at current_phase==0
   
    return;
    }

/*-------------7D -- moved to here to be more appropriately placed  ADVANCE PHASE ---------------*/
// Note that we only get to this code when we have hit a discrete time slice boundary, 
// because of an if/return sequence above.

if (measuring_plat == 0){ // If we're actively measuring the plat, use longer slices SDS121
next_slice = next_slice + 60000L / ((long)beats_per_minute * (long)MAX_PHASES);
} else {
save_next_slice = next_slice; // Save the current next slice before changing
next_slice = now + 4000L; //If measuring plat use slice time of 4s -- SDS
}
    // GLG0602   that line accidentally got put inside the #ifdef....it needs to be ablways done.

  
  
  #ifdef NEXTSLICESPY
  Serial.print(F("Next_slice: "));
  Serial.print(next_slice);
  Serial.print(F(" but after increment: "));
  Serial.println(next_slice);
  #endif
  
  
  // GLG0531 - steve S's method of getting  this to the proper settings 
  current_phase++;
  

  return;

}

/*---------------------------------------------------------*/

void DoExhaledTidalVolume()
{
  float diffpressure;  // There really isn't a need to make this an array
  unsigned long  time_width; // the amount of time since we did the last measurement in microsedsonds

  /*----------------The different values and units

     float  DigitalFlowZero;           // the sensor output when no flow
     float  RawDigitalPressure;        // what we get
     float exLitersPerCycle;           // where it gets accumullated is a FLOAT
     #define ANALOGDIFFTRANSDUCER   // if using the 2kPa analog transdcuer
     #define   I2CDIFFTRANSDUCER     // if using the I2C transducer
     float diffpressure[MAX_PHASES];
  */




  current_flow_time = micros();  // get current time in microseconds
  time_width =  current_flow_time - last_flow_time;  // subtract last time
  last_flow_time =  current_flow_time;  // store it


#ifdef ANALOGDIFFTRANSDUCER
  delP =  ReadFlowPressureSensor() - zerorawSensorValue ; // all INTS and in units of 0-1023
  CalculateInstantFlow();   // returns float instant flow 
  //diffpressure = (float)  ReadFlowPressureSensor() - (float) zerorawSensorValue;   // Function returns an INT
  // these are scaled now 0-1023 for a 5 volt span, and the unit is approximately Pascals
   exLitersPerCycle = exLitersPerCycle  +  time_width * (( instantFlowValue  / 60000 ) );
#endif

#ifdef I2CDIFFTRANSDUCER
  diffpressure = ReadI2CFlowPressureSensor();
#endif 

#ifdef DEBUG 
  Serial.print(F("\nDPres: "));
  Serial.print(diffpressure);
#endif

  /*
    #ifdef DELP-DISPLAY
      Serial.print ("Differential pressure:   ");
      Serial.print (delP);
      Serial.print ("\n");
    #endif
  */
  // Now calculate the FLOW for that pressure difference:
  // time_width is measured in MICROSECONDS
  // instantFlow value is in LITERS per MINUTE
  // 1 Minute has 60 million microseconds.
  // milliters =  1000 * liters
  // liters =    liters/minute   *   (1min/60000000 microseconds) * time_width in microsedons)
  // mL =    liters/minute * time_width / 60 000;

  // here the instantFlowvalue is Liters/minute

#ifdef I2CDIFFTRANSDUCER
    CalculateI2CInstantFlow();
#endif
    current_exp_volume = time_width * (( instantFlowValue  / 60000 ) );
    exLitersPerCycle = exLitersPerCycle  +  current_exp_volume;

#ifdef  TIMEWIDTHSPY
Serial.print(F("Twid: "));
Serial.println(time_width);
#endif


#ifdef  FLOWMEASURE2
Serial.print(F("T.Wid, I.Flow, exLPC: "));
Serial.print(time_width);
Serial.print(F(" "));
Serial.print(instantFlowValue);
Serial.print(F(" "));
Serial.println(exLitersPerCycle);
#endif
  
 

#ifdef DEBUG 
  Serial.print(F("\nI2cInstantFlow: "));
  Serial.print(instantFlowValue);  // liter/minute
  Serial.print(F("\nCum L/cycle: "));
  Serial.print(exLitersPerCycle);
#endif





}


/*---------------------------------------------------------*/

void  vent_start() {
  current_phase = 0;
  next_slice = millis();  // GLG0531   Add in an intialization for next_slice
  
  vent_running = 4;
}

//at times the motor/valve maybe stuck, try turning it off twice
void vent_abort() {
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);// get both valves open!!
  //  pressure_off();
  current_phase = 0;
  vent_running = 0;
  is_pressure_on = 0; // GLG ---
  MyDelay(200);
  //  pressure_off();
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);// get both valves open!!
}

// 
// vent_init will properly turn off valves, and measure either one or two sensors,
// store the difference, set up atmospheric_pressure,  works whether BMP280AMBIENT exists or not
// 

void vent_init() {

  log_message( (char*) "vent_init: sensors");
  init_pressure_sensor();
                  // note that mesure_atmospheric_pressure will turn off valve x 4 seconds. 
  MyDelay(1000);
  wdt_reset();   // MUST be certain that pressure is not applied!
  MyDelay(1000);
  wdt_reset();
  

  log_message( (char*) "Cal sens");
  measure_atmospheric_pressure();

  #ifdef BMP280AMBIENT
  measure_atmospheric_pressure2();   // measures the ambient pressure in the 2nd unit
  // store the tracking differnce between these two....
  diff_atmospheric_pressure = atmospheric_pressure - atmospheric_pressure2;  
  // exists other than 0 only if BMP280AMBIENT is defined.  Otherwise remainds 0.   
  // software throughout system uses the first one  atmospheric_pressure   for all calculations of gauge pressure
  // REMEASURE THIS DIFFERENCE by calling vent_init()   whenever a ZERO/RESET is done.   
  #endif
  
  MyDelay(100);  // I don't think this delay is necessary so I reduced it dramaticallly
  log_message( (char*) "Vent rdy");
}



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

//####################################    Starting Code #######################################
void setup() {
  // put your setup code here, to run once:
  // turn on the watchdog FIRST THING
  setup_watchdog();   // see elsewhere for explanation.
  tone(ALARM_PIN,500,250);
  wdt_reset();
  MyDelay(250);
  wdt_reset();   // resets the watchdog timer
  
  #ifdef DEBUGSERIAL  
  Serial.begin(115200);
  while (!Serial);
  // GLG0602  Print out the version number
  
  Serial.print(F("****VERSION:  "));
  Serial.print(VERSION);
  Serial.println(F("****"));
  #endif 
  //  JAM THIS IN DEVELOPMET SO WE CAN KEEP GOING
    // i2c_status = I2C_READY;  // COMMENT  OUT FOR SHIPPING
    // i2c_allowed = 1;  // set to use it

  #ifdef DEBUGBMP280  
  Serial.print(F("I2C Sensor status:"));
  #endif  
  switch (i2c_status) {
    case I2C_READY:
      #ifdef DEBUGBMP280      
      Serial.println(F("Ready"));
      #endif      
      break;
    case I2C_BUSY:
      #ifdef DEBUGBMP280      
      Serial.println(F("Unresponsive from previous bootup"));
      #endif      
      i2c_allowed = 0; // disable the I2C Buss!!!!
      for(int z=0;z<3;z++)  // send three dashes to say the I2C is busted.....
      {
      tone(ALARM_PIN,250,1000);  // low dash
      wdt_reset();
      MyDelay(1250);
      wdt_reset();
      }
      
      break;
    default:
      #ifdef DEBUGBMP280      
      Serial.println(F("Cold start"));
      #endif      
      i2c_allowed = 1; // Allow the bus! on COLD START
      i2c_status = I2C_READY;
  }

#if I2CLCDDISPLAY == true
  lcd.begin();
  lcd.backlight();
#else
  lcd_init(LCDDISPLAY);
#endif


  for (int t = 0; t < 6; t++)
    alarm_array[t] = 0;   // zero out the char alarm_array values

  alarm_high_pressure = false;
  alarm_high_volume = false;
  alarm_low_pressure = false;
  alarm_low_volume = false;

  load_settings();
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);

  Wire.begin();
  init_pressure_sensor();
  vent_init();  // This will zero the BMP280
  // ----------------Set up for FLOW MEASUREMENTS -------------------
#ifdef DEBUG
  Serial.println("FlowZero");
#endif
// TONE to announce Flow Zeroing begins
tone(ALARM_PIN,500,250);
MyDelay(250);  // keep tones from running into each other. 
wdt_reset();

  Serial.println("Done BME settings");



  //Begin to set up the Digital Flow Sensor transducer (differential I2C pressure
  //  EXAMPLE of Handling I2C bus:
  //  if(i2c_allowed==1){
  //     i2c_status=I2C_BUSY;  // set the flag to indicate failure if reset happens
  //     ....now do i2C business
  //     ....finished with I2C business
  //     i2c_status=I2C_READY; // reset flag to "working"
  //     }
  //     else // put in code to provide default value for pressure measurement
  //
  //#define ANALOGDIFFTRANSDUCER   // if using the 2kPa analog transdcuer
  //#define   I2CDIFFTRANSDUCER     // if using the I2C transducer
  //   SET UP THE I2C DIFF TRANSDUCER AND GET THE ZERO WHILE AT IT

#ifdef  I2CDIFFTRANSDUCER
 DigitalZeroDifferentialTransducer();
#endif   // end of the digital flow transducer 


#ifdef ANALOGDIFFTRANSDUCER   /// if we are using ANALOG, then zero that also!!  
  analogReference(DEFAULT);
  // read the baseline voltage reading from the differntial pressure
  zerorawSensorValue = 0;
  for (int qq = 0; qq < 5; qq++) {
    zerorawSensorValue = zerorawSensorValue + ReadFlowPressureSensor();
  }
  zerorawSensorValue = zerorawSensorValue / 5;

#ifdef DEBUG  
  Serial.print(F("Read baseline zero flow output:"));
  Serial.println(zerorawSensorValue);
#endif
#endif


  cTime = millis();
  // ------------end of FLOW MEASUREMENT SETUPS
#ifdef DEBUG  
  Serial.println("Setup done");
#endif
if(i2c_allowed==1) 
tone(ALARM_PIN,1000,1000); // end of setup -- 1 second hi tone
                           // if I2C working, lower if not
else tone(ALARM_PIN,400,1000);
wdt_reset();
MyDelay(1000);
wdt_reset();

Tbottomlimit = 20;   // a guess.   
#ifdef FILTERSPY
Serial.print(F("Tbottomlimit: "));
Serial.println(Tbottomlimit);
#endif


}

//  Reset the watchdog timer.   If this isn't done every so often, the watchdog
//  will automatically reset the hardware.   At that point we examine a non-initialzed
//  variable to determine if the reset had to do with a I2C call failure.... and if
//  it did, we will have to disable all calls to the I2C devices.
//  For further information on the watchdog timer, see:
//  https://forum.arduino.cc/index.php?action=dlattach;topic=63651.0;attach=3585
//  or other such tutorials.

// Ashhar had a delay of 100 msec here but I think that is WAY too long to
// allow slices of 50 msec to be handled.   So I'll change it to 10 mSec


void loop() {
  now = millis();

  wdt_reset();   // resets the watchdog timer
  vent_slice();
  //vent_Timer();
  alarm_slice();
  lcd_slice();
  //Serial.println(measure_pressure());
  // MyDelay(5);
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
