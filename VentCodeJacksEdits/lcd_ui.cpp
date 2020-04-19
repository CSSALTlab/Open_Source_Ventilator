// NO PERMISSION TO USE THIS CODE FOR ANY PURPOSE WITHOUT
// WRITTEN PERMISSION FROM GORDON GIBBY
//
#ifndef BEENHERE
#include "vent.h"
#endif


/*****
  Purpose: Sets the lcd status

  Argument list:
    char *text      status message to display
    
  Return value:
    void
    
  CAUTION:
*****/
void lcd_status(char *text){
  if (lcd_size == LCDDISPLAY)
    lcd.setCursor(13,0);
  else
    lcd.setCursor(14,3);
  lcd.print(text);
}


/*****
  Purpose: Sets the lcd status

  Argument list:
    char *text      message to display
    
  Return value:
    void
    
  CAUTION:
*****/
void lcd_message(char *text)
{
  if (lcd_size == 1602){
    int i, padding = (16 - strlen(text))/2; 
    lcd.setCursor(0,0);
    for (i = 0; i < padding; i++)
      lcd.print(' ');
    lcd.print(text);
    i += strlen(text);
    while(i++ < 16)
      lcd.print(' ');
  } else {
    lcd.setCursor(10,3);
  }
  lcd.print(text);
  for (int i = strlen(text) + 10; i < 20; i++)
    lcd.print(' ');
}

/* The graph is displayed as a 40 x 8 bitmap defined as a framebuffer of custom letters
 * (We saw this first used by Hans Summers in his QSX transceiver prototype.
 * 
 * The framebuffer is transferred to the LCD as a series of maximum of 8 custom letters
 * The framebuffer is organized as follows:
 * 1. Each character is made u of 8 bytes, each byte represents one row of the character
 * 2. The bytes are interpreted as painting the character top to down 
 * 3. Each row byte of the character is show on the screen as (left to right)D0 D1 D2 D3 D4 D5
 */

char framebuffer[64];

/* The entire bitmap is dumped here.
 *  We do this by defining the bitmap as a 40 x 8 pixel
 */
void lcd_refresh(){
  for (int i =0; i < 8; i++)
    lcd.createChar(i, (uint8_t*) (framebuffer + (i * 8)));
      
  lcd.setCursor(8,0);
  for (int i = 0; i < 5; i++)
    lcd.write((byte)i);
}

void lcd_pixel(int x, int y){

  int column = x / 5;
  int byte_offset = (column * 8) + y;

  int bitmask = 1 << (4 - (x % 5));
   
  framebuffer[byte_offset] += bitmask;
}

void lcd_graph_clear(){
  memset(framebuffer, 0x0, sizeof(framebuffer));
  lcd_refresh();  
}

void lcd_graph_update(){

  //reset the framebuffer
  memset(framebuffer, 0x0, sizeof(framebuffer));

  if (vent_running){

    
    for (int i = 0; i < current_phase; i+=2){
      int b = bargraph[i] / 40;
      if (b > 7)
        b = 7;
      if (b < 0)
        b = 0;
      for(int j = 7; j >= 7-b; j--)
        lcd_pixel(i/2, j);
        //lcd_pixel(i, 7-b);
    }
   
    
    lcd_refresh();
  }

  char buff[17];
  char buff2[6];  // temp storage

  itoa(bargraph[current_phase]/10, buff, 10);  // bargaph deprecated  

 // This is sorta stupid -- we write out the values but instantly change to alarms if need be.    
 // There has to be a better way but this will probably work
 // GLG
 
  if (lcd_size == LCDDISPLAY)
    lcd.setCursor(0,0);
  else
   lcd.setCursor(14,0);
  for (int i=0; i < 2-strlen(buff); i++)
    lcd.print(' ');
    itoa(peakinspiratorypressure,buff, 10);  // print the peak inspiratory pressure
    strcat(buff,"cm");
    itoa(exmLPerCycle,buff2,10);  // print out the expiratory tidal volume detected
    strcat(buff,buff2);   // add it to the display
    strcat(buff,"mL");
    lcd.print(buff);



  lcd.setCursor(0,0); 
  if(current_phase < 4 && alarm_array[0]==1) lcd.print("***LO PRESS***");  // print LOW press alarms for slices 0-3
  if(current_phase > 3  && current_phase < 8 && alarm_array[1]==1) lcd.print("***HI PRESS***");  // print high press alarms slices 4,5,6,7
  if(current_phase > 7  && current_phase < 12 && alarm_array[2] ==1 ) lcd.print("***LO VOL***");  // print low volume alarms slices 8,9,10,11
  if(current_phase >11  && current_phase < 16 && alarm_array[3] ==1)  lcd.print("***HI VOL***");  // print high volume alarms slices 12, 13, 14, 15


  
  
}


void paint_lcd(){
  // PAINTS THE SECOND LINE  OF the disPlay
  lcd.setCursor(0,2);
  lcd.print((char)0x3E);

  if (lcd_size == LCDDISPLAY){
    // GLG:  THIS is the size that I'm developing for right now......

    
    //paint the user interface  -- ths part does the SECOND LINE
    
    lcd.setCursor(0,1);
    if (selected_item == SELECT_PRESSURE) 
      lcd.print('[');
    else
      lcd.print(' ');  
      lcd.print(vent_pressure_limit);
      
 //   lcd.print(vent_pressure/10);
 //lcd.print("cm");   // stop printing the cm to save space
    
    if (selected_item == SELECT_PRESSURE) 
      lcd.print(']');
    else if (selected_item == SELECT_BPM)
      lcd.print('[');
    else
      lcd.print(' ');
  
    lcd.print(beats_per_minute);
//    lcd.print("BPM");  // Stop printing the BPM to save space   
    if (selected_item == SELECT_BPM) 
      lcd.print(']');
    else if (selected_item == SELECT_IE)
      lcd.print('[');
    else
      lcd.print(' ');
    if(inspiratory_pause==1) lcd.print("1*"); 
    if(inspiratory_pause!=1) lcd.print("1:");
    lcd.print(ie_ratio);  
    if (selected_item == SELECT_IE) 
      lcd.print(']');
      else if (selected_item == SELECT_VOLUME)
        lcd.print('[');
    else
      lcd.print(' ');

      lcd.print(desired_TV);
   if(selected_item == SELECT_VOLUME) lcd.print(']');
   else lcd.print(' ');

  // Add in a placeholder for tidal volume
     lcd.print(" 700 ");   // test value 
  }
  else {
    //paint the user interface
    lcd.setCursor(0,0);
    lcd.print(' ');
//    lcd.print(vent_pressure/10);
//   lcd.print("CM ");
  
    lcd.setCursor(0,1); 
    lcd.print(' ');
    lcd.print(beats_per_minute);
    lcd.print("BPM ");
  
    lcd.setCursor(0,2);
    lcd.print(" 1:"); 
    lcd.print(ie_ratio);  
  
    lcd.setCursor(0, selected_item);
    lcd.print('>');
  }
}

void lcd_init(int display_size){

//  lcd_size = display_size;

  Serial.println("Initializing the display");
/*
  //lcd.begin(20, 4);
  if (lcd_size == 1602) {
    lcd.begin(16,2);
  } else {
    lcd.begin();
    lcd.backlight();
  }
*/  
  pinMode(FN,     INPUT_PULLUP);
  pinMode(UP,     INPUT_PULLUP);
  pinMode(DOWN,   INPUT_PULLUP);
  pinMode(ON_OFF, INPUT_PULLUP);

   lcd_graph_clear();
  paint_lcd();
}

int getButton(){
//  if (digitalRead(FN)==LOW && digitalRead(UP)==LOW) return RESET_I2C; // don't think this works
//  if (digitalRead(FN) == LOW && digitalRead(DOWN) == LOW) // don't think this works. 
//    return SELECT_MUTE;
  if (digitalRead(FN) == LOW)
    return FN;
  else if(digitalRead(UP) == LOW)
    return UP;
  else if (digitalRead(DOWN) == LOW)
    return DOWN;
  else if (digitalRead(ON_OFF) == LOW)   // ON_OFF  allows 2nd button 
        // Press and hold ON_OFF; then more than 100 mSec later hit the 2nd button and hold until acknowledged.
     { 
              digitalWrite(MOTOR_A, LOW);
              digitalWrite(MOTOR_B, LOW);// get both valves open!!
           //  vent_abort();   // unfortunately this completely stops ventilation  
            MyDelay(700);   // allow a second button to be pressed
            if(digitalRead(UP)==LOW) {
            inspiratory_pause=1;  // turn on inspiratory pause
            lcd_message( (char*) "PAUS-ON");
#ifdef DEBUG
            Serial.print("InspiratoryPause Turned On \n");
#endif
             wdt_reset();
             MyDelay(1000);
            }
            
            else if (digitalRead(FN)==LOW) {
            lcd_message( (char*) "NOPAUS");
            inspiratory_pause=0;  // turn off inspiratory pause 
#ifdef DEBUG
            Serial.print("Inspiratory Pause OFF \n");
#endif
       
            wdt_reset();
            MyDelay(1000); 
            lcd_refresh();   // see what this does             
            }

            else if (digitalRead(DOWN)==LOW) {
                I2CBusFailFlag=0x38;  // Reset so that if program restarts will find I2C presumed working
                I2CBusAllowed=1;  // Indicate that program is allowed to make calls to I2C bus
            lcd_message( (char*) "I2CRESET");
#ifdef DEBUG
            Serial.print("I2C was reset \n");
#endif
            wdt_reset();
            MyDelay(1000);              
            }

            

      else  return ON_OFF;
     }
    
  else
    return 0;  
}

void lcd_slice(){
  
  int action = getButton();
  int q;  // local counter

  if (!action)
    return; 

#ifdef DEBUG  
  Serial.print("Btn:");Serial.println(action);
#endif

  if (action==RESET_I2C) {   // requires two buttons:   FN and UP
    I2CBusFailFlag=0x38;  // Reset so that if program restarts will find I2C presumed working
    I2CBusAllowed=1;  // Indicate that program is allowed to make calls to I2C bus
#ifdef DEBUG
   Serial.print("Resetting I2C");
#endif
  }

/*  if (action == SELECT_MUTE){
    alarm_silence(60);
    return;
  }
*/  
  if (action == ON_OFF){

    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);  // open both valves
    if (vent_running)
      {

      
      for(q=0;q<8;q++) {
        wdt_reset();
        MyDelay(1000);
      }
      wdt_reset();
      // now rezero things
      measure_atmospheric_pressure();  /// Rezero the atmospheric pressure 
      wdt_reset();
      zerorawSensorValue=0;
  for(int qq=0;qq<5;qq++){
  zerorawSensorValue = zerorawSensorValue + ReadFlowPressureSensor();  
  }
  zerorawSensorValue = zerorawSensorValue/5;
      vent_start();
      }
      
    else
      vent_start();
  }
  if (action == FN){
    
    if (vent_running && alarm_get_status() != ALARM_OFF){
      alarm_silence(60);
      //GLG --- eliminate the alrm
      alarm_status = ALARM_OFF;
      alarm_on_until = 0;
    alarm_off_until = 0;
#ifdef DEBUG
      Serial.println("*silencing the alarm");
#endif

    }
      
    selected_item++;
    if (selected_item > MAX_SELECTION)
      selected_item = 0;
  }
  if (action == UP){
    if (selected_item == SELECT_BPM && beats_per_minute < 30)
      beats_per_minute++;
    if (selected_item == SELECT_PRESSURE && (vent_pressure_limit < 60))
      vent_pressure_limit += 5;
    if (selected_item == SELECT_IE && ie_ratio < 4)
      ie_ratio++;
    if (selected_item == SELECT_VOLUME  && desired_TV<900) desired_TV=desired_TV+TV_INCREMENT;  
    
  }

  if (action == DOWN){
    if (selected_item == SELECT_BPM && beats_per_minute > 10)
      beats_per_minute--;
   if (selected_item == SELECT_PRESSURE && vent_pressure_limit > 9)
    vent_pressure_limit -= 5;
    if (selected_item == SELECT_IE && ie_ratio > 1)
      ie_ratio--;
    if (selected_item == SELECT_VOLUME  && desired_TV>200) desired_TV=desired_TV-TV_INCREMENT; 
  }
  save_settings();
  paint_lcd();
  //debounce
  MyDelay(5); // I don't think this delay is even needed.  
  while(getButton()){
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);// get both valves open!!  
  }
       // I think this is allowing for the entire thing to be brought to a stop; 
  
}
