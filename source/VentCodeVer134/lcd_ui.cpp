// NO PERMISSION TO USE THIS CODE FOR ANY PURPOSE WITHOUT
// WRITTEN PERMISSION FROM GORDON GIBBY
//  
#ifndef BEENHERE
#include "vent.h"
#endif

// the lcd should be refered to the main ino. it is specific this implementation
//LiquidCrystal lcd(13,12,11,10,9,8);

static uint8_t button_status                = 0;
static unsigned long next_button_read_time  = 0UL;   //used to debounce the buttons
static int screen_top_line                  = 0;
int selected_item                           = -1;
static int menu_cursor                      = -1;
static int n_menu_items;                            // count of menu items
static int n_display_rows;                          // display rows
static int n_display_columns;

static char ticker[35];
static int  ticker_index                    = 0;
static unsigned long next_ticker_time       = 0UL;

/*
#if LCDDISPLAY == 2004
LiquidCrystal_I2C lcd(0x27, 20, 4);      // Define the 20x4 display object
#else
//LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //            16x2
#endif
*/
/* About the menu system:
    The menu adjusts itself to a 4 or 2 line display depending upon the
    define LCDDISPLAY in vent.h. For a 20x4 display define it as 2004
    for a 16x2 display, define it as 1602

    The menu system consists of one function that for each item.
    You have write this function and add it to the menu_list array. It
    will automatically get included in the menu.

    Each menu function is called with it's own structure and a cmd
    1. CMD_DISPLAY, it should print out the menu's text and parameters (if any)
    2. CMD_UP : changes the menu item's value up
    3. CMD_DN: changes the menu item's value down
    4. CMD_SELECTED: prompts the menu item to action (eg: exit makes the item exit out of the menu)
*/

struct menu_item {
  int id;
  void (*menu_item_handler)(struct menu_item *me, int cmd);
};

// Add your menu items here
struct menu_item menu_list[] = {
  {0, menu_exit},
  {1, menu_vent_on},
  {2, menu_vent_off},
  {3, menu_bpm},
  {4, menu_pressure},
  {5, menu_peep_desired},
  {6, menu_peep_type},  //TJ 06.12.2020 This is probably what PEEP control algorithm is used (PID, ILC)
  {7, menu_assist},
  {8, menu_trigger_cm},
  {9, menu_volume},
  {10, menu_ie_ratio},
  {11, menu_plat},
  {12, menu_inspiratory_pause},
  {13, menu_reset_alarms},
  {14, menu_reset_sensor}
  };


void lcd_clear_screen() {
  //clear the screen
   wdt_reset();
  for (int y = 0; y < n_display_rows; y++) {
    lcd.setCursor(0, y);
    for (int x = 0; x < n_display_columns; x++)
      lcd.print(' ');
  }
   wdt_reset();
  //lcd.setCursor(0,2);
  //lcd.print("1234567890123456");
}


void lcd_status(char *text) {
  if (lcd_size == LCDDISPLAY)
    lcd.setCursor(0, 2);    // Move text down to bar graph --SDS
  else
    lcd.setCursor(14, 3);

if (menu_cursor == -1)
lcd.print(text);
}

//pads up the string with spaces to the end of the line
void lcd_message(char *text) {
wdt_reset();
  
  if (lcd_size == 1602) {
    int i, padding = (16 - strlen(text)) / 2;
    lcd.setCursor(0, 0);
    for (i = 0; i < padding; i++)
      lcd.print(' ');
    lcd.print(text);
    i += strlen(text);
    while (i++ < 16)
      lcd.print(' ');
  } else {
    lcd.setCursor(10, 3);
  }
  lcd.print(text);
  for (int i = strlen(text) + 10; i < 20; i++)
    lcd.print(' ');
}



void lcd_init(int display_size) {
wdt_reset();
  //  lcd_size = display_size;

  Serial.println(F("Initializing the display"));

#ifndef I2CLCDDISPLAY  // Defined out for I2C SDS
  if (display_size == 2004) {
    lcd.begin(20, 4);
  } else {
    lcd.begin(16, 2);
  }
#endif
// Reorginized these steps a bit to diaplay "Vent Starting" as soon as lcd is ready -- SDS
  
  n_display_rows    = LCDDISPLAY % 10;
  n_display_columns = LCDDISPLAY / 10;
 wdt_reset();
  lcd_graph_clear();
  lcd_clear_screen();

  lcd.setCursor(0,1);            
  lcd.print("Vent Starting");
  
  n_menu_items = sizeof(menu_list) / sizeof(struct menu_item); //just a way of counting them
  Serial.print(F("Menu Items Number = "));
  Serial.println(n_menu_items);

  wdt_reset();
  
  /*
    lcd.begin(20, 4);
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

  wdt_reset();
  //  paint_lcd();
 
// SDS removed make and animate ticker so ticker not displayed when 
// vent off since pressure is not measured anyway. 
// These functions are called in LCD slice when vent on.

   
}


/*
   Ticker makes it easy for longer messages to scroll
   We use only 12 character long scroll ticker, you can increase it
   in animate ticker
*/

void make_ticker() {
 wdt_reset();
  
  char num_str[5];

  ticker[0] = 0;

  if (vent_running)
    strcat(ticker, "*ON* ");
  else
    strcat(ticker, "*OFF* ");

  itoa(beats_per_minute, num_str,10);
  strcat(ticker, num_str);
  strcat(ticker, "BPM ");
  
  itoa(vent_pressure_limit, num_str, 10);
  strcat(ticker, num_str);
  strcat(ticker, "cm ");

  itoa(desired_TV, num_str, 10);
  strcat(ticker, num_str);
  strcat(ticker, "mL ");

  strcat(ticker, "1:");
  itoa(ie_ratio, num_str, 10);
  strcat(ticker, num_str);
  strcat(ticker, " ");

  strcat(ticker, "PEEP");
  itoa(desired_peep, num_str,10);
  strcat(ticker,num_str);
  strcat(ticker," ");
  

  ticker_index = 0;
  next_ticker_time = millis();
}

void animate_ticker() {
 wdt_reset();
  
  if (next_ticker_time > millis())
    return;


  if (!vent_running){       //added trailing space to overwrite "Vent Starting" text after startup -- SDS
    lcd.setCursor(0,1);
    lcd.print(F("<<VENT OFF>> "));
 
    return;
  }
  
  lcd.setCursor(0, n_display_rows - 1);

  
// Make ticker string longer for if using larger screen

  #ifdef LCD16x2
  int len = strlen(ticker);
  int t = ticker_index;
  for (int i = 0; i < 12; i++) {
    if (t >= len)
      t = 0;
    lcd.print(ticker[t++]);
  }
  ticker_index++;
  if (ticker_index >= len)
    ticker_index = 0;
  next_ticker_time = millis() + 400;
#else
  int len = strlen(ticker);
  int t = ticker_index;
  for (int i = 0; i < 20; i++) {
    if (t >= len)
      t = 0;
    lcd.print(ticker[t++]);
  }
  ticker_index++;
  if (ticker_index >= len)
    ticker_index = 0;
  next_ticker_time = millis() + 400;
#endif
  wdt_reset();
}

/*
  create some special characters for the menu system
  these are overwritten when the vent's main display
  is running, so this function is called each time
  the menu is invoked again
*/
void lcd_menu_characters() {
  byte caret[8] = {
    B00000,
    B10100,
    B01010,
    B00101,
    B01010,
    B10100,
    B00000,
  };

  lcd.createChar(CHAR_CARET, caret);

  byte two[8] = {
    B10101,
    B10101,
    B11011,
    B11111,
    B11011,
    B11000,
    B11111,
  };

  lcd.createChar(CHAR_TWO, two);

  byte back[8] = {
    B00100,
    B01110,
    B00100,
    B00100,
    B11100,
    B00000,
    B00000,
  };

  lcd.createChar(CHAR_BACK, back);

}

/* The graph is displayed as a 40 x 8 bitmap defined as a framebuffer of custom letters
   (We saw this first used by Hans Summers in his QSX transceiver prototype.

   The framebuffer is transferred to the LCD as a series of maximum of 8 custom letters
   The framebuffer is organized as follows:
   1. Each character is made u of 8 bytes, each byte represents one row of the character
   2. The bytes are interpreted as painting the character top to down
   3. Each row byte of the character is show on the screen as (left to right)D0 D1 D2 D3 D4 D5
*/

char framebuffer[64];

/* The entire bitmap is dumped here.
    We do this by defining the bitmap as a 40 x 8 pixel
*/
void lcd_refresh() {
  for (int i = 0; i < 8; i++)
    lcd.createChar(i, (uint8_t*) (framebuffer + (i * 8)));

 #ifdef LCD16x2          // Move graph to lower position if larger screen
  lcd.setCursor(12, 0);
  #else
  lcd.setCursor(0, 2);  
#endif
  
  for (int i = 0; i < 5; i++)

if (menu_cursor == -1)    // Stop graph from displaying over menu
    lcd.write((byte)i);
}

void lcd_pixel(int x, int y) {

  int column = x / 5;
  int byte_offset = (column * 8) + y;

  int bitmask = 1 << (4 - (x % 5));

  framebuffer[byte_offset] += bitmask;
}

void lcd_graph_clear() {
  memset(framebuffer, 0x0, sizeof(framebuffer));
  lcd_refresh();
}

void lcd_graph_update() {
wdt_reset();
  //stop this while the menu is on display
  if (menu_cursor != -1) 
    return;

  //reset the framebuffer
  memset(framebuffer, 0x0, sizeof(framebuffer));
  
 char buff[17];
 
  if (vent_running && i2c_allowed) {

  wdt_reset();
    for (int i = 0; i < current_phase; i++) {
      int b = bargraph[i] / 40;
      if (b > 7)
        b = 7;
      if (b < 0)
        b = 0;
      for (int j = 7; j >= 7 - b; j--)
        lcd_pixel(i, j);
    }
    wdt_reset();
      lcd_refresh();

 
  

  itoa(bargraph[current_phase] / 10, buff, 10); // bargaph deprecated
  }


  

lcd.setCursor(0,0); 


#ifdef DEBUG
    Serial.print(F("\nPeak Inspiratory Pressure:  "));
    Serial.print(peakinspiratorypressure);
#endif

 if (!i2c_allowed){
  lcd.print("Err RESET SENSOR");
  }
  
if (i2c_allowed){ 

#ifdef LCD20x4     // If using large screen, space things out and display BPM -- SDS
itoa(peakinspiratorypressure, buff, 10); // print the peak inspiratory pressure GLG---
    
    strcat(buff, "cm ");

    lcd.print(buff);
    itoa(exmLPerCycle, buff, 10); // print out the expiratory tidal volume detected
      // add it to the display
    strcat(buff, "mL ");
    lcd.print(buff);
    itoa(current_pressure,buff,10);// print the current airway pressure  GLG ---
    strcat(buff,"cm ");
    lcd.print(buff);
    itoa(actual_BPM,buff,10);
    strcat(buff,"BPM");
    lcd.print(buff);
  //  if (strlen(buff) < 5) 
    lcd.print("    ");// Clear residual characters when string shortens --SDS

#else
itoa(peakinspiratorypressure, buff, 10); // print the peak inspiratory pressure GLG---
    
    strcat(buff, "cm");

    lcd.print(buff);
    itoa(exmLPerCycle, buff, 10); // print out the expiratory tidal volume detected
      // add it to the display
    strcat(buff, "mL");
    lcd.print(buff);
    itoa(current_pressure,buff,10);// print the current airway pressure  GLG ---
    strcat(buff,"cm");
    lcd.print(buff);
    if (strlen(buff) < 4) // Clear residual characters when string shortens --SDS
    lcd.print(' ');
#endif    
}

// NOW HANDLE MAKING ALARMS PAINFULLY OBVIOiuS

#ifdef LCD16x2
lcd.setCursor(0,0); 
#else
lcd.setCursor(0,1);
#endif


  //    LOW pressure alarm become valid after about slice 10;   Display LATE (>7  < 12)  and reset only at slice 0  
  //    HI pressure alarm becomes valid anywhere between 0 and 10  Display LATER (>11 < 16)  but must LATCH -- not be reset until slice 0
  //    LO Volume alarm valid at slice MAX_PHASES -- don't reset until cut_off+4 and display early (<4 )  in the slices
  //    HI Volume alarm valid at slide MAX_PHASES -- don'r reset until cut_off+4 and display early (>3  < 8) in the slices
  if(current_phase < 4 && alarm_array[2]==1) lcd.print(F("SET TV NOT DELIV"));  // print LOW VOLUME alarms
  if( i2c_allowed==1 &&current_phase > 3  && current_phase < 8 && alarm_array[3]==1) lcd.print(F("TV HIGH"));  // print high press alarms slices 4,5,6,7
  if(i2c_allowed ==1 && current_phase > 7  && current_phase < 12 && alarm_array[0] ==1 ) lcd.print(F("LOW AIRWAY PRES!"));  // print low pressure alarms slices 8,9,10,11
  if(current_phase >11  && current_phase < 16 && alarm_array[1] ==1)  lcd.print(F("OVER PRES ALARM!"));  // print high volume alarms slices 12, 13, 14, 15
  
  #ifdef VOLTAGEDIVIDER
  if(current_phase>15 && alarm_array[4]==1) lcd.print(F("ELECT SUPP FAIL!"));  // print supply voltage failure

  #ifdef VOLTAGESPY
  Serial.print(F("Voltage Pin: "));
  Serial.println(analogRead(SUPPLYVOLTAGESENSORPIN));
  #endif
  #endif
  
    
}


/*
   The button update scans to see if any buttons have been lifted or pressed since the lasst scan
   Upon change, the next scan is deferred until 200 msec to allow the buttons to settled down
   from the mechanical bounce
*/
bool button_update() {
  uint8_t new_status = 0;
wdt_reset();
  if (millis() < next_button_read_time)
    return false;

  if (digitalRead(FN) == LOW)
    new_status |= BTN_FN;
  if (digitalRead(DOWN) == LOW)
    new_status |= BTN_DN;
  if (digitalRead(UP) == LOW)
    new_status |= BTN_UP;
  if (digitalRead(ON_OFF) == LOW)
    new_status |= BTN_PW;

  //debounce, so, read next button update only after 75 msec
  wdt_reset();
  if (new_status != button_status) {
    next_button_read_time = millis() + 75;
    button_status = new_status;
    return true;
  } else


// If > 10s and no button and not on home screen, go back to home screen
if (millis() >= (next_button_read_time + 10000) && new_status == button_status && menu_cursor != -1 )
 {clear_menu(); }

 return false;
}

void menu_exit(struct menu_item *m, int cmd) {
wdt_reset();
  switch (cmd) {
    case CMD_SELECTED:
      clear_menu();
      break;
    case CMD_DISPLAY:
      lcd.print(F("Exit \007   FW "));
      lcd.print(VERSION);  // SDS121 added version display
  }
}

void menu_plat(struct menu_item *m, int cmd){  // -- SDS plat menu

  switch (cmd) {
    case CMD_SELECTED:
      measure_plat = 1;
      clear_menu();
      break;
    case CMD_DISPLAY:
      lcd.print(F("Check Plat"));  
  }
}

void menu_bpm(struct menu_item *m, int cmd) {
wdt_reset();
  if (cmd == CMD_DN && beats_per_minute > 10){
    beats_per_minute--;
    save_settings();    
  }
  else if (cmd == CMD_UP && beats_per_minute < 30){
    beats_per_minute++;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Breaths   "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(beats_per_minute);
    lcd.print(F("/min"));
    
  }
}

void menu_pressure(struct menu_item *m, int cmd) {
wdt_reset();
  if (cmd == CMD_DN && vent_pressure_limit > 15){
    vent_pressure_limit -= 1;
    save_settings();    
  }
  else if (cmd == CMD_UP && vent_pressure_limit < 60){
    vent_pressure_limit += 1;
    save_settings();
  }
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Pressure  "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(vent_pressure_limit);
    lcd.print(F("cm"));
  }
}
/*-----------------------my trigger sensitivity ------------------*/

void menu_trigger_cm(struct menu_item *m, int cmd) {
wdt_reset();
  if (cmd == CMD_DN && trigger_cm > 3){
    trigger_cm -= 1;
    save_settings();    
  }
  else if (cmd == CMD_UP && trigger_cm < 15){
    trigger_cm += 1;
    save_settings();
}
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Asst Trig "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(trigger_cm);
    lcd.print(F("cm"));
  }
}



/* ------------------------end of trigger sensitivity------------------*/


void menu_volume(struct menu_item *m, int cmd) {
wdt_reset();
  if (cmd == CMD_UP && desired_TV < 900){
    desired_TV += TV_INCREMENT;
     save_settings();    
  }   
  else if (cmd == CMD_DN && desired_TV > 200){
    desired_TV -= TV_INCREMENT;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Volume    "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(desired_TV);
    lcd.print(F("mL"));
  }
}

void menu_ie_ratio(struct menu_item *m, int cmd) {
wdt_reset();
  if (cmd == CMD_UP && ie_ratio < 4){
    ie_ratio++;
    save_settings();    
  }    
  else if (cmd == CMD_DN && ie_ratio > 1){
    ie_ratio--;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("IE Ratio  "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(F("1/"));
    lcd.print(ie_ratio);
  }
}

void menu_vent_on(struct menu_item *m, int cmd)
{
  if (cmd == CMD_SELECTED /* && selected_item == m->id*/ ) {
    vent_running= 2;  // set it to TWO
    exp_valve_closure_cycle = 20;  
    next_slice=  millis();  // GLG0531 -- gotta set the next slice!!!!
    loops_since_major_jump=0;  //GLG0602 -- initilize the loops_since_major_jump here!

    
    #ifdef FILTERSPY
    Serial.print(F("exp_valve_closure_cycle: "));
    Serial.println(exp_valve_closure_cycle);
    #endif
  
    lcd_clear_screen();

    clear_menu();   

    //clear_menu();  //duplicate call not needed -- SDS
    return;
    
  }

if (cmd == CMD_DISPLAY) {
    lcd.print(F("Vent->ON "));
    if (selected_item == m->id){
      lcd.print((char) CHAR_CARET);
      //lcd.print(F("ON"));
    }
    else
     // lcd.print((char)':');
    if(vent_running>0) lcd.print(F("ON"));  // Print the ON only when the vent is already ON
    
    }
}

void menu_vent_off(struct menu_item *m, int cmd)
{wdt_reset();
  if (cmd==CMD_SELECTED /* && selected_item == m->id  */ ) {
    
    if(vent_running==1){  // means we managed to decrement down to 1 and this is our last selection!
            vent_abort();
            vent_running=0;
            save_settings();
            clear_menu();   
            return;
            }
    if(vent_running>1) {
          //lcd.print(F("Confirm"));  // WHERE does this print???
          vent_running--;  // decrement it
          }
  }
  
    if (cmd == CMD_DISPLAY) {
    lcd.print(F("Vent->OFF "));
    if (selected_item == m->id) {
      lcd.print((char) CHAR_CARET);
      lcd.print( F("Confirm!"));  
    }
    else lcd.print((char)':');
    if(vent_running<=0) lcd.print(F("OFF"));  // Print the OFF only whe the vent is still running
    }
    
 
}
  
/* ---------------I found a simpler way for me  --------------------
void menu_ventilate(struct menu_item *m, int cmd) {
  
  if (CMD_SELECTED && selected_item == m->id) {
    if(vent_running>=2) 
            {
            Serial.print(">=2Vent_running: ");
            Serial.println(vent_running);
            vent_running--;   // decrement -- will take TWO times to get it done
            lcd.print(F("Confirm"));
            Serial.print(">=2vent_running: ");
            Serial.println(vent_running);
            
            MyDelay(100);
            return;
            }
    
    
    if (vent_running <= 0)  { 
        vent_running= 6; 
        vent_start();
        MyDelay(100);
        return;
        }
        
         if(vent_running==1) 
            {
            Serial.print("1vent_running: ");
            Serial.println(vent_running);
            vent_abort();
            vent_running=0;
            save_settings();
            clear_menu();   
            return;
            }
         
        
      }

  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Vent    "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (vent_running)
      lcd.print(F("On"));
    else
      lcd.print(F("Off"));
  }
}

-----------------------------------------------*/

// PEEP filter selection option SDS121
void menu_peep_type(struct menu_item *m, int cmd) {
if (cmd==CMD_SELECTED /* && selected_item == m->id */ ) {
    if (peep_type == 0){
      peep_type = 1;
      save_settings();    
    }  
    else{
      peep_type = 0;
      save_settings();    
    }
    //deselect!
    selected_item = -1;
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print(F("PEEP TYPE "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (peep_type == 1)
      lcd.print(F("PID"));
    else
      lcd.print(F("Adapt"));
  } 
}

void menu_assist(struct menu_item *m, int cmd) {
  wdt_reset();
if (cmd==CMD_SELECTED /* && selected_item == m->id */ ) {
    if (vent_assist == 0){
      vent_assist = 1;
      save_settings();    
    }  
    else{
      vent_assist = 0;
      save_settings();    
    }
    //deselect!
    selected_item = -1;
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print(F("ASSIST     "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (vent_assist == 1)
      lcd.print(F("Yes"));
    else
      lcd.print(F("No"));
  }
}

void menu_inspiratory_pause(struct menu_item *m, int cmd) {
  wdt_reset();
  if (cmd==CMD_SELECTED /* && selected_item == m->id */ ) {
    if (inspiratory_pause == 0){
      inspiratory_pause = 1;
      save_settings();    
    }  
    else{
      inspiratory_pause = 0;
      save_settings();    
    }
    //deselect!
    selected_item = -1;
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print(F("I-Pause   "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (inspiratory_pause == 1)
      lcd.print(F("Yes"));
    else
      lcd.print(F("No"));
  }
}

void menu_reset_alarms(struct menu_item *m, int cmd) {
  wdt_reset();
  if (cmd==CMD_SELECTED /* && selected_item == m->id */ ) {
    //reset all the alarms
    alarm_silence(60);  // silence them for a minute
    alarm_low_pressure = false;
    alarm_high_pressure = false;
    alarm_low_volume = false;
    alarm_high_volume = false;
    // I may need to silence the alarm_array
    alarm_array[0]=0;
    alarm_array[1]=0;
    alarm_array[2]=0;
    alarm_array[3]=0;
    alarm_array[4]=0;
    
    clear_menu();
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print(F("Alarms    "));
    if (selected_item == m->id) lcd.print((char) CHAR_CARET);
    else lcd.print((char)':');
    lcd.print(F("Reset"));
  }
}

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



void menu_reset_sensor(struct menu_item *m, int cmd) {
  wdt_reset();
  if (cmd==CMD_SELECTED /* && selected_item == m->id*/  ) 
    {
    // lcd.print(F("*RESET/CAL*"));
    vent_abort();
    i2c_status = I2C_READY;  // mark this for a possible fault if it won't reset!!!! 
                                                 // I think this needs to say READY. The purpose of this menu setting is to reset this. If there is a fault, it will be re-set to busy later in the code
    i2c_allowed = 1;
    wdt_reset();
    save_settings(); 
    //  init_pressure_sensor();
    tone(ALARM_PIN,500,250);
    wdt_reset();
    MyDelay(250);
    vent_init();            //  Initialize in order to get the atomospheric again!! GLG---
    wdt_reset();
  // SDS Changes.  Removed call to measure_atmospheric_pressure since 
  // it is already called in vent_init
  

    // What about the differntial pressure transducer???
    // lets at least zero that....
    tone(ALARM_PIN,500,250);
    wdt_reset();
    MyDelay(250);
    #ifdef I2CDIFFTRANSDUCER
    DigitalZeroDifferentialTransducer();  // get it zeroed
    #endif

    //lcd.print(F("DONE "));
    if(i2c_allowed==1) tone(ALARM_PIN,1000,1000);
    else tone(ALARM_PIN,400,1000);
    wdt_reset();
    MyDelay(500);
    wdt_reset();
    
   #ifdef SERIALPRINT
   Serial.print( (char *)"*RESET I2C-zero\n");
   #endif
    }
  
  if (cmd == CMD_DISPLAY){
    lcd.print(F("Zero/Reset"));
    if (selected_item == m->id) lcd.print((char) CHAR_CARET);
    else lcd.print((char)':');
    lcd.print(F("A:"));
    lcd.print(i2c_allowed);
    if(i2c_allowed==1) lcd.print(F(" DONE"));
    
  }
    
}

/*---------Gibby tries to add a menu item -----------*/
void menu_peep_desired(struct menu_item *m, int cmd) {
  wdt_reset();

  if (cmd == CMD_DN && desired_peep >= 1) {  //TJ 06.11.2020 corrected logic 
	  desired_peep--;
    save_settings();    
  }
  else if (cmd == CMD_UP && desired_peep < 25){
    desired_peep++;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print(F("PEEP      "));
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(desired_peep);
    lcd.print(F("cm"));
  }
}


void clear_menu(){
  menu_cursor = -1;
  selected_item = -1; 
  lcd_clear_screen();
}

void display_menu() {
  // int n_columns = LCDDISPLAY / 10;  // apparently never used
  lcd_menu_characters();                                      //add custom characters

// Try silencing the alarm for 60 sec any time a menu is shown.
alarm_silence(60);   


    for (int i = 0; i < n_menu_items; i++) {
    int item_position = i - screen_top_line;
    if (item_position < 0 || item_position >= n_display_rows)
      continue; 

 

    //clear the display to the end of the line
    lcd.setCursor(0, item_position);

#ifdef LCD20x4    
    for (int j = 0; j < 20; j++)
#else
    for (int j = 0; j < 16; j++)
#endif

      lcd.print(' ');
    lcd.setCursor(0, item_position);
    //show the caret on the menu item if only the item is not selected
    //selected items show the caret _inside_ the item
    if (menu_cursor == i && selected_item != i)
      lcd.print((char)CHAR_CARET);
    else
      Serial.print(F(" "));
    menu_list[i].menu_item_handler(menu_list + i, CMD_DISPLAY);

  }
}

#ifdef LCD16x2
void lcd_show_alarms() {
  if (!vent_running) // only displays if vent is running and menu isn't displayed
    return;

  //rubout the alarms
  lcd.setCursor(12,1);
  lcd.print(F("    "));
  
  int trouble = 0;
  if (current_phase < 4 && alarm_low_pressure == true) {
    lcd.setCursor(12, 1);
    lcd.print(F("Lp"));
    trouble++;
  }
  if (current_phase > 3  && current_phase < 8 && alarm_high_pressure == true) {
    lcd.setCursor(12, 1);
    lcd.print(F("Hp"));
    trouble++;
  }
  if (current_phase > 7  && current_phase < 12 && alarm_low_volume == true) {
    lcd.setCursor(14, 1);
    lcd.print(F("Lv"));
    trouble++;
  }
  if (current_phase > 11  && current_phase < 16 && alarm_high_volume == true) {
    lcd.setCursor(14, 1);
    lcd.print(F("Hv"));
    trouble++;
  }

  if (!trouble){
    lcd.setCursor(12, 1);
    lcd.print(F("-OK-"));
  }
}
#else

void lcd_show_alarms() {
  if (!vent_running) // only displays if vent is running 
    return;
    
  //rubout the alarms
  lcd.setCursor(16,2);
  lcd.print(F("    "));
  
  int trouble = 0;
  if (current_phase < 4 && alarm_low_pressure == true) {
    lcd.setCursor(16, 2);
    lcd.print(F("Lp"));
    trouble++;
  }
  if (current_phase > 3  && current_phase < 8 && alarm_high_pressure == true) {
    lcd.setCursor(16, 2);
    lcd.print(F("Hp"));
    trouble++;
  }
  if (current_phase > 7  && current_phase < 12 && alarm_low_volume == true) {
    lcd.setCursor(18, 2);
    lcd.print(F("Lv"));
    trouble++;
  }
  if (current_phase > 11  && current_phase < 16 && alarm_high_volume == true) {
    lcd.setCursor(18, 2);
    lcd.print(F("Hv"));
    trouble++;
  }

  if (!trouble){
    lcd.setCursor(16, 2);
    lcd.print(F("-OK-"));
  }
}
#endif


void lcd_slice() {

  if (menu_cursor == -1) {
    animate_ticker();
    lcd_show_alarms();
  }

  if (!button_update()) {
    return;
  }

  if (button_status & BTN_PW) {
    if (menu_cursor == -1)
      menu_cursor = 1;  // 1st item is ventilate. this jumps directly to this menu
    else
      clear_menu();
  }

  if (menu_cursor == -1) {            //selected_item = -1, menu is not being displayed
    if (!(button_status & BTN_FN))      //if start menu only if FN button is down
      return;
    menu_cursor = 0;
  }
  else if (selected_item == -1) {
    if ((button_status & BTN_DN) && menu_cursor + 1 < n_menu_items)
      menu_cursor++;
      else if ((button_status & BTN_DN) && menu_cursor + 1 >= n_menu_items)
      menu_cursor = 0;
    else if ((button_status & BTN_UP) &&  menu_cursor > 0 )
      menu_cursor--;


    //scroll the selected item to view
    if (menu_cursor < screen_top_line)
      screen_top_line = menu_cursor;
    if (menu_cursor -  screen_top_line >= n_display_rows) {
      screen_top_line = menu_cursor - n_display_rows + 1;
    }
    if (button_status & BTN_FN) {
      menu_list[menu_cursor].menu_item_handler(menu_list + menu_cursor, CMD_SELECTED);
      selected_item = menu_cursor;
    }
  }
  // only if some item is selected
  else if (button_status) {
    if (button_status & BTN_UP)
      menu_list[menu_cursor].menu_item_handler(menu_list + menu_cursor, CMD_UP);
    else if (button_status & BTN_DN)
      menu_list[menu_cursor].menu_item_handler(menu_list + menu_cursor, CMD_DN);
    else if (button_status & BTN_FN)
      selected_item = -1; //deselect
  }
  if (menu_cursor != -1)
    display_menu();

  make_ticker();
}
