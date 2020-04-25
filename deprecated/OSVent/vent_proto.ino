#include <Wire.h>
#include <EEPROM.h>
#include <BMP180I2C.h>
#include <avr/wdt.h>

#include "nano_gui.h"
#include "vent.h"
#define I2C_ADDRESS 0x77

//create an BMP180 object using the I2C interface
BMP180I2C bmp180(I2C_ADDRESS);
#define MOTOR_A 2
#define MOTOR_B 3

int use_tft = 0;

#define EEPROM_VENT_ON  48
#define EEPROM_BPM      52
#define EEPROM_PRESSURE 56
#define EEPROM_IERATIO  60

int beats_per_minute = 10;
int ie_ratio = 1; //this is the denominator
int vent_pressure = 160; //this is in mm
int vent_temperature;
int vent_running = 1;
int pressure_sensor_present = 0;

//draw the graph
int bargraph[MAX_PHASES];
int current_phase = 0;
long next_slice = 0;
int is_pressure_on = 0;
long  atmospheric_pressure = 0;
int current_pressure = 0;
long pressure_total = 0;
long pressure_peak = 0;

byte automatic_pressure_level = false;

void checkCAT(){
  
}

void save_settings(){
  EEPROM.put(EEPROM_VENT_ON, vent_running);
  EEPROM.put(EEPROM_BPM, beats_per_minute);
  EEPROM.put(EEPROM_PRESSURE, vent_pressure);
  EEPROM.put(EEPROM_IERATIO, ie_ratio);    
}

void load_settings(){
  EEPROM.get(EEPROM_VENT_ON, vent_running);
  EEPROM.get(EEPROM_BPM, beats_per_minute);
  EEPROM.get(EEPROM_PRESSURE, vent_pressure);
  EEPROM.get(EEPROM_IERATIO, ie_ratio);
  if (vent_running != 1){
    Serial.println("*default vent off");
    vent_running = 0;
  }
  if (beats_per_minute < 0 || beats_per_minute > 30){
    Serial.println("*default 10/bpm");
    beats_per_minute = 10;
  }
  if (vent_pressure < 50 || vent_pressure > 300){
    vent_pressure = 150;
    Serial.println("*default pressure 15 cm");    
  }
  if (ie_ratio < 1 || ie_ratio > 4){
    Serial.println("*default I/E ration = 1:1");
    ie_ratio = 1;   
  }
}

void set_status(char *text){
  if (use_tft)
    displayText(text, 0,0,320,78, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  else
    lcd_status(text);
}

void show_message(char *text){
  if (use_tft)
    displayText(text, 0,0,320,78, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  else
    lcd_message(text);
  Serial.println(text);
}

//returns the absolute pressure in mmH20
//based on the formula at https://www.convertunits.com/from/mmH2O/to/Pa
long measure_pressure(){   
   
  //start a temperature measurement
  if (!bmp180.measureTemperature())
  {
    show_message("No Sensor");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true. 
  do
  {
    delay(0);
  } while (!bmp180.hasValue());
  vent_temperature = (int)bmp180.getTemperature();
  Serial.println(vent_temperature);

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

  long p = (long)bmp180.getPressure();  
  return ((102l * p)/1000l);
}

void measure_atmospheric_pressure(){

  //if pressure is on, turn it off and wait for 8 seconds for the lungs and compressor to deflate
  if (is_pressure_on){
    pressure_off();
    delay(8000);
  }
  //now calibrate for 5 seconds
  atmospheric_pressure = 0;
  for (int i = 0; i < 10; i++){
    atmospheric_pressure += measure_pressure();
    delay(200);
  }
  //now take the average
  atmospheric_pressure /= 10;
}

int loop_count = 0;

void pressure_on(){
  if (is_pressure_on == 0){
    digitalWrite(MOTOR_A, HIGH);
    digitalWrite(MOTOR_B, LOW);
    is_pressure_on = 1;
    Serial.print(millis());
    Serial.println(": on");
  }
}

void pressure_off(){
  if (is_pressure_on != 0){
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, HIGH);
    is_pressure_on = 0;  
    Serial.print(millis());
    Serial.println(": off");
  }
}


/*
 * This has to be tweaked
 */
void check_pressure_limits(){
  //check for over-pressure
  if (current_pressure > (vent_pressure * 15)/10){
    alarm(ALARM_FAST);
    set_status("HI");
  }
    
  if (current_phase == 0){
    pressure_total = 0;
  }

  //check only if the current phase is ending
  if (current_phase != MAX_PHASES - 1)
    return;
  if ((pressure_total * ie_ratio * 2)/MAX_PHASES < vent_pressure ){
    alarm(ALARM_SLOW);
    set_status("LO");
  }     //tujrn off only if the peak pressure isn't too much either
  else if (pressure_peak < (vent_pressure * 12)/10){
    alarm(ALARM_OFF);
    set_status("ON");
  }
  
}

void setup_watchdog(void){
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
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set Watchdog settings:
  WDTCSR =(1<<WDIE) | (1<<WDE) |(1<<WDP3) | (0<<WDP2) | (0<<WDP1) |(1<<WDP0);
  sei();
}


void vent_slice(){
  long now, p;

  if (!vent_running){
    alarm(ALARM_OFF);
    show_message(">>OSVent Ready<<");
    return;
  }

  now = millis();
  if (next_slice > now)
    return;

  int cut_off = MAX_PHASES - (MAX_PHASES * (ie_ratio))/(ie_ratio + 1);

  check_pressure_limits();
    
  //we move to the next slice
  if (current_phase < cut_off)
    pressure_on();
  if (current_phase > cut_off)
    pressure_off();
    
  next_slice = now + 60000l / ((long)beats_per_minute * (long)MAX_PHASES);

  p = measure_pressure();  
  bargraph[current_phase] = (int)(p - atmospheric_pressure);
  current_pressure = bargraph[current_phase];
  pressure_total += current_pressure;
  if (use_tft)
    tft_graph_update();
  else 
    lcd_graph_update();
  current_phase++;
  
  if (current_phase >= MAX_PHASES){
   if (use_tft)
      tft_graph_clear();
    else
      lcd_graph_clear();
    current_phase = 0;    
  }
 // Serial.print("tot:");
 // Serial.println(pressure_total);
}

void  vent_start(){
  current_phase = 0;
  vent_running = 1;
}

//at times the motor/valve maybe stuck, try turning it off twice
void vent_abort(){
  pressure_off();
  current_phase = 0;
  vent_running = 0;
  delay(200);
  pressure_off();
}
void vent_init(){

  //premptively give this error
  show_message("No Sensor");

  //begin() initializes the interface, checks the sensor ID and reads the calibration parameters.  
  if (!bmp180.begin()){
    pressure_sensor_present = 0;
    while (1);
  }

  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);

  show_message("Sensor OK");
  measure_atmospheric_pressure();
  show_message("Ready");  
}

void setup() {
  // put your setup code here, to run once:

  setup_watchdog();

  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);
  
  Serial.begin(9600);
  while (!Serial);

  load_settings();

  Wire.begin();

  if (use_tft)
    tft_init();
  else
    lcd_init(1602);  


  vent_init();
  //setupTouch();
  Serial.println("Setup done");
}

void loop() {
  vent_slice();
  alarm_slice();
  wdt_reset();
  if (use_tft)
    tft_slice();
  else
    lcd_slice();
  //Serial.println(measure_pressure());
  delay(100);
}
