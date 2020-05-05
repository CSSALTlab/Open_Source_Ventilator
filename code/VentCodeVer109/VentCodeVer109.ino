//  Version 108   4/28/2020
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

float  supply_voltage =   12.0  ;  // iitial value only!
uint8_t i2c_status  __attribute__ ((section (".noinit")));
uint8_t i2c_allowed;   // variable to track whether this bus is allowed

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
//  alarm_array[4] refers to LOW VOLTAGE from supply
//  others are undefined at the moment

int atmospheric_pressure    = 0;          // in cmH2O
int bargraph[MAX_PHASES];  // bargraph deprecated
int beats_per_minute        = 10;
int desired_peep = 5;                     // what PEEP we want to happen.
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
int TVhighlimit             = 700;        // high limit TV milliliters. See desired_TV above, too: Purdum/Schmidt
int TVlowlimit              = 300;        // low limit TV milliliters Purdum/Schmidt
int vent_running            = 2;
int vent_pressure_limit     = 40;         // Units are cmH2O (gauge)
int zerodelP;                             // measured delP at presumed zero flow  Purdum/Schmidt  (analog transducer)
int zerorawSensorValue;                   // baseline (analog) raw sensor output (at no pressure no flow); Purdum/Schmidt

unsigned long current_flow_time = 0 ; //  microseconds
unsigned long last_flow_time = 0;   //  microseconds

int  trigger_cm = 8;    // global variable for the depth of cm the patient must pull to get a breath; larger= harder to get assist



//  VARIABLES for the digital flow sensor
float  DigitalFlowZero;           // the sensor output when no flow
float  RawDigitalPressure;        // what we get




long alarm_on_until         = 0L;
long alarm_off_until        = 0L;
long alarm_suppress_until   = 0L;
boolean alarm_low_pressure;
boolean alarm_high_pressure;
boolean alarm_low_volume;
boolean alarm_high_volume;


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

static  int smooth_pressure_run;          // counter for how tightly the expriatory pressure is kept

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


BMx280I2C ssenseBMx280(settings);


BMP180I2C bmp180(BMP_ADDRESS);    // For the BMP180

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
  int q;
  int rdp10;   // ten times the raw digital pressure, but an int
  RawDigitalPressure = 0.0;  // Float to deal with the digital output

  if (i2c_allowed == 1) // Can't even make themeasurement if not allowed....
  {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happens

    for (q = 0; q < 3; q++)
    {
      gagePressure.startMeasurement();
      gagePressure.readData(true);
      if (!isnan(gagePressure.pressure))
      {
        RawDigitalPressure = RawDigitalPressure + gagePressure.pressure;
      }
    }
    // if we got here, we did all of the measurements
    i2c_status = I2C_READY; // reset flag to "working"
  }  // end of if i2c_allowed
  RawDigitalPressure = RawDigitalPressure / 3; // take the average
  // Now take out the zero bias
  RawDigitalPressure = RawDigitalPressure - DigitalFlowZero;
  rdp10 = (int) (10*RawDigitalPressure);
  
  #ifdef FLOWMEASURE
  Serial.print("RawDP: ");
  Serial.println(rdp10);
  #endif
  
  return (RawDigitalPressure);  // Note returns a FLOAT

}

int ReadFlowPressureSensor()
{

  // NOTE defines:
  //#define ANALOGDIFFTRANSDUCER   // if using the 2kPa analog transdcuer
  //#define   I2CDIFFTRANSDUCER     // if using the I2C transducer
  //  EXAMPLE of Handling I2C bus:
  //  if(i2c_allowed==1){
  //     i2c_status=I2C_BUSY;  // set the flag to indicate failure if reset happens
  //     ....now do i2C business
  //     ....finished with I2C business
  //     i2c_status=I2C_READY; // reset flag to "working"
  //     }
  //     else // put in code to provide default value for pressure measurement


  // lets average 3 readings and try that.
  int q;
  rawSensorValue = 0;
  for (q = 0; q < 3; q++)
  {
    rawSensorValue        =  rawSensorValue + analogRead(PRESSURESENSORPIN);
  }

  rawSensorValue = rawSensorValue / 3;

#ifdef DEBUG
  Serial.print(F("Raw Sensor value: "));
  Serial.print(rawSensorValue);
  Serial.print(F("\n"));
  MyDelay(500);
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
float CalculateI2CInstantFlow()
{
  //  Uses RawDigitalPressure and calculates the associated flow that produced it.

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

// ------------------------------------------------------------------------

float CalculateInstantFlow()
{
  float correction;                 // Used in calculation to avoid square root of negative number
  if (delP < 0)
    correction = -1.0;
  else
    correction = 1.0;
  instantFlowValue = MF * PTFCC * sqrt(delP * correction);
  instantFlowValue *= correction;

#ifdef DEBUG 
  Serial.print(F("CalculateInstantFlow"));
  Serial.print(F("    delP = "));
  Serial.print(delP);
  Serial.print(F("    instantFlowValue = "));
  Serial.println(instantFlowValue);
#endif

  return instantFlowValue;
}


void DigitalZeroDifferentialTransducer()
{
  if (i2c_allowed == 1) {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happens
    DigitalFlowZero = 0;  /// initialize it, we're going to average 4 measurement
    for( int q=0;q<4;q++)
      {
    
        gagePressure.setPressureUnit(AllSensors_DLHR::PressureUnit::PASCAL);
        gagePressure.startMeasurement();
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

        }
        DigitalFlowZero = DigitalFlowZero/4;   // take the average
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

}

void load_settings() {
  EEPROM.get(EEPROM_VENT_ON, vent_running);
  EEPROM.get(EEPROM_BPM, beats_per_minute);
  EEPROM.get(EEPROM_PRESSURE, vent_pressure_limit);
  EEPROM.get(EEPROM_IERATIO, ie_ratio);
  EEPROM.get(EEPROM_PAUSE, inspiratory_pause);
  EEPROM.get(EEPROM_TV, desired_TV);
  EEPROM.get(EEPROM_PEEP, desired_peep);

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
  if (desired_peep < 3   || desired_peep > 25) desired_peep = 5; // default peep
}

void update_status2(char *text) {
  lcd_status(text);
}
void log_message(char *text) {
  Serial.println(text);
}

void init_bmp180() {
  //begin() initializes the interface, checks the sensor ID and reads the calibration parameters.
  if (!bmp180.begin()) {
    pressure_sensor_present = 0;
    while (1);
  }

  //enable ultra high resolution mode for pressure measurements
  // bmp180.setSamplingMode(BMP180MI::MODE_UHR);
  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);

}

void init_bmp280() {
  int passCount = 0;
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);

  if (i2c_allowed == 1) {
#ifdef DEBUG   
    Serial.print(F( "\n\nWill read BMx280  i2c_allowed = "));
    Serial.println(i2c_allowed);
#endif
    
    while (!ssenseBMx280.begin()) {
      ++passCount;
      MyDelay(250L);
      if (passCount > 16) {             // Try initializing it for 4 seconds
#ifdef DEBUG
        Serial.print(F("No BMP."));
#endif
        lcd.setCursor(0, 0);
        lcd.print(F("No init of BMx280"));
        return;
      }
    }
    switch (ssenseBMx280.chipModel()) {
      case BME280::ChipModel_BME280:
#ifdef DEBUG
        Serial.print(F("BME280(hum) " ));
#endif
        break;
      case BME280::ChipModel_BMP280:
        // insert any desired code to notify that BMP280 (no humidity) detected
#ifdef  DEBUG
        Serial.print(F("BMP280(nohum) " ));
#endif
        break;
      default:
        // insert any code to notify that the unknown sensor found.
        lcd.print(ssenseBMx280.chipModel());
#ifdef DEBUG        
       Serial.println(F("Sensor ssenseBMx280 not found."));
#endif        
        break;
    }
    ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);

  } // end of skipping everything if i2c_allowed==0

}

void init_pressure_sensor() {
  //i2c_status = I2C_READY;  // ***COMMENT OUT FOR RELEASE***

  if (i2c_allowed == 1) {

    i2c_status = I2C_BUSY; // set flag to discover a possible failure
    if (USE_BMP280)
      init_bmp280();
    else
      init_bmp180();
    // If we made it to here, the I2C bus worked OK
    i2c_status = I2C_READY; // reset the warning flag
    i2c_allowed = 1;
  }
  else {
#ifdef DEBUG 
    Serial.print(F("init_pressure_sensor skipped the BMP's\n"));
#endif
  }

}

//returns the absolute pressure in mmH20
//based on the formula at https://www.convertunits.com/from/mmH2O/to/Pa
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




//returns the absolute pressure in cmH2O
//based on the formula at https://www.convertunits.com/from/mmH2O/to/Pa
int measure_bmp280() {
  unsigned long testpressure;
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);

  i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happens

  ssenseBMx280.read(fpressure, temp, hum, tempUnit, presUnit);
  testpressure = (unsigned long) fpressure;
  testpressure = testpressure / 98;  // cm H2O

#ifdef DEBUG 
  Serial.print ("\n Airway P cmH2O: ");
  Serial.print (testpressure);
  Serial.print ("\n");
#endif


  i2c_status = I2C_READY; // reset flag to "working"  GLG---
  return (testpressure);

}

int measure_pressure() {

  long m;
  if (i2c_allowed == 1) {
    i2c_status = I2C_BUSY; // set the flag to indicate failure if reset happen
    if (USE_BMP280)
      m =  measure_bmp280();
    else
      m =  measure_bmp180();

    // ******NEWS FLASH*****
    // I pulled the clock line and instead of failing, it
    // returned ZERO.
    //
    if (m == 0) i2c_allowed = 0; // stop using it
    else     i2c_status = I2C_READY; // reset flag to "working"

#ifdef DEBUG 
    Serial.print(F("1stPress cmH2O: "));
    Serial.println(m);
#endif
    return m;
  }
  else
    m = 999;

  return m;

  //     return 999;  not sure why this is needed???
}

int loop_count = 0;

void pressure_on() {
  digitalWrite(MOTOR_A, HIGH);
  digitalWrite(MOTOR_B, HIGH);
  is_pressure_on = 1;
}

void pressure_off() {   // now implements both inspiratory pause AND PEEP

  digitalWrite(MOTOR_A, LOW);  // Stop the inspiration pronto!!!!
  if (i2c_allowed == 0) digitalWrite(MOTOR_B, LOW);

#ifdef DEBUG 
  Serial.print(F("\n i2c_allowed, i2c_status:  "));
  Serial.print(i2c_allowed);
  Serial.print(F(" ,"));
  Serial.print(i2c_status);

#endif

  if (i2c_allowed == 0) {

    // turn off the Motor_B
    digitalWrite(MOTOR_B, LOW);
    is_pressure_on = 0;
    return;
  }


  // CAUTION:: If the peak inspiratory pressure never made it above lowpressurelimit
  // CAUTION:: We may have a disconnected or blocked airway pressure line -- MUST ALLOW TO EXHALE!!!
  //           SO DONT KEEP EXHALATION CLOSED IF PRESSURE IS LOW!!!
  // put in a delay to make them happy
  if (inspiratory_pause == 1 && (current_phase < (cut_off + 4) )  && (peakinspiratorypressure > lowpressurelimit) )digitalWrite(MOTOR_B, HIGH);
  else {  // NOW we need to look at PEEP
    if (current_pressure > desired_peep  || (peakinspiratorypressure < lowpressurelimit) ) digitalWrite(MOTOR_B, LOW); // drop the pressure
    if (current_pressure <= desired_peep   && (peakinspiratorypressure > lowpressurelimit) ) digitalWrite(MOTOR_B, HIGH); //don't drop thepressure
    // Note a tiny bit of hysteress
  }


  //-----------------------------------------------------------

  is_pressure_on = 0;

}

void measure_atmospheric_pressure() {
  //if pressure is on, turn it off and wait for 4 seconds for the lungs and compressor to deflate
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

  //now calibrate for 1.0 seconds
  long accumulated_pressure = 0;
  for (int i = 0; i < 10; i++) {
    accumulated_pressure += measure_pressure();
    wdt_reset();
    MyDelay(100);
  }

  atmospheric_pressure = accumulated_pressure / 10;
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

void vent_slice() {
int p;  // to allow pressure to be measured

  // long differentialpressure;

  cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);   // no longer needed here
  current_pressure= measure_pressure() - atmospheric_pressure  ;    // MEASURE AIRWAY PRESSURE
   

  // THE FIRST PART of this subroutine gets carried out many, many times
  // during the expiratory phase.....
  // the latter half, is ONLY carried out when we reach the time boundary at the end of
  // one slice and the beginning of the next.


  if (!vent_running) {
    alarm(ALARM_OFF);
    return;
  }

  // remember, "cut_off" is a PHASE (or or slice) number....out of 20 currently  (MAX_PHASES=20)
  // have to calculate cut_off becuse it may not be known
  cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);


/*-------------------------TRIGGER ASSIST BREATHING SECTION-------------------------------*/

  // Set up a counter to keep track of how settled the expiratory pressure is--do it right at the transition
  if (current_phase == cut_off) {
    smooth_pressure_run = 0;
    last_flow_time = micros();   //  microseconds-- get a baseline for this cycle to start
  }                               // remember this portion of code is done many many times, not just at the end of a slice
                                  // so it will be done THROUGHOUT the timeslice cut_off....
                                  // and exhalation begins onthe cut_off+1 slice
                                  


  if ( current_phase > cut_off  && inspiratory_pause == 0  ) { // we only look for inspiratory effort during expiration
    // for simplicity I'm not offering this with inspiratory pause also
    // WE ARE IN EXHALATION
    // Have to measure the pressure....
   

    if (smooth_pressure_run > 10 && i2c_allowed == 1 ) // in other words, we are past the rapid ringing and into steady decay
                                                      // calibrated for approx 60 milliseconds of steady expiratory presure
      // no further need for checking run lengths
      // can't fiddle with Assisted breaths if the airway pressure isn't possible
    {
      // we are now in the plateau portion of the expiration, so quit working on the pressure.....
      // search for an inspiratory breath.
      // TRIGGER IS NOW ADJUSTABLE -- higher the trigger_cm the more difficult to trigger

      if ( current_pressure <  (desired_peep - trigger_cm) )  { // this needs to be adjusted for peep!!!!
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

#ifdef ASSISTSPY
    Serial.print(F("\nSmPressRun: "));
    Serial.print(smooth_pressure_run);
#endif

  }  // END of the ASSIST VENTILATION code  

/*----------------------------------------------END OF ASSIST VENTILATION-----------------------------------------*/


// Make Quick Check on overpressure
if (current_phase < cut_off+4)
  {
    if ( current_pressure > vent_pressure_limit)  {  // check against the real pressure limit
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);// get both valves open!!
    // open expiratory valve and close inspiratory valve
    Serial.print(F("\nHIGH PRESSURE\n"));
    alarm(ALARM_FAST);
    alarm_high_pressure = true;
    update_status2( (char*) "HI");
    current_phase++;  // jump ahead!!
    alarm_array[1] = 1; // set the alarm array
    }

  }  // end of checking during inspiration for over pressure 

  // ATTEMPT PEEP CONTROL------------------------------------------------
  if (current_phase > cut_off) { // Note that if we have jammed an inspiratio to start, this will be avoided
    // current_pressure = measure_pressure() - atmospheric_pressure;  // already got tht done above!! doesn't need repeating
    pressure_off();  // that routine has it written in now.
    // NOW lets' carry out the Expiratory Flow Measurements
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


  cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio)) / (ie_ratio + 1);   // no longer needed here

  check_pressure_limits();  // check this at the end of every slice


  // Zero the peak inspiratory pressure if this is our very first time
  // in this cycle.
  if (current_phase == 0) {
    peakinspiratorypressure = 0;
    // zero two of the alarm values
    alarm_array[0] = 0; // zero the low pressure alarm, which gets displayed LATE
    alarm_array[1] = 0; // zero the high pressure alarm, which gets displayed LATE

    //  If i2c isn't working, there are few alarms we can measure, so give a short done to tell we are ventlating anyway at 400 Hz
    if(i2c_allowed==0) {
      tone(ALARM_PIN,400,250);   // 1/4 second
      wdt_reset();
    }
  }

// Check the supply voltage in phases 2 and 3
#ifdef VOLTAGEDIVIDER
  if(current_phase ==2 || current_phase==3)
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




  //we move to the next slice
  if ((current_phase < cut_off) && (peakinspiratorypressure < vent_pressure_limit ) )  {
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
  p = measure_pressure();


  bargraph[current_phase] = (int)(p - atmospheric_pressure);  // I don't think we need the bargraph
  current_pressure = bargraph[current_phase];

  //current_pressure = (int) (p- atmospheric_pressure);// calcuate the guage pressure
  pressure_total += current_pressure;
  lcd_graph_update();
  current_phase++;

  // RESET the volume alarms only just befoe the end of the cycle so they show for a cycle
  if (current_phase == MAX_PHASES - 1) {
    alarm_array[3] = 0; // reset the HI VOL alarm just before it will get caluclated again
    alarm_array[2] = 0; // reset the LO VOL alarm just before it will get calculated again
  }

  if (current_phase >= MAX_PHASES) { // end of the entire respiratory cycle

    //  CORRECTION TIME!!!! GLG
    //  No longer needed after rarranging the expiratory flow path.....
  

    
    exmLPerCycle = (int) exLitersPerCycle; // note exmLPerCycle= INT
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
    current_phase = 0;
  }

}

/*---------------------------------------------------------*/

void DoExhaledTidalVolume()
{
  float diffpressure;  // There really isn't a need to make this an array
  unsigned long  time_width; // the amount of time since we did the last measurement

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
  diffpressure =  ReadFlowPressureSensor();
  exLitersPerCycle = exLitersPerCycle  +   (  CalculateInstantFlow() * 60 / ( (float) (beats_per_minute *  MAX_PHASES))  )  ;
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
  CalculateI2CInstantFlow();
   exLitersPerCycle = exLitersPerCycle  +  time_width * (( instantFlowValue  / 60000 ) );

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

void vent_init() {

  log_message( (char*) "vent_init: sensors");
  init_pressure_sensor();
  MyDelay(1000);
  wdt_reset();   // MUST be certain that pressure is not applied!
  MyDelay(1000);
  wdt_reset();
  MyDelay(1000);
  wdt_reset();
  MyDelay(1000);
  wdt_reset();

  log_message( (char*) "Cal sens");
  measure_atmospheric_pressure();
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
  //  setup_watchdog();   // see elsewhere for explanation.
  tone(ALARM_PIN,500,250);
  wdt_reset();
  MyDelay(250);
  wdt_reset();   // resets the watchdog timer
  
#ifdef DEBUGSERIAL  
  Serial.begin(115200);
  while (!Serial);
#endif 
  //  JAM THIS SO WE CAN KEEP GOING
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


  exhale            = false;
  lastMode          = false;

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
