// NO PERMISSION TO USE THIS CODE FOR ANY PURPOSE WITHOUT
// WRITTEN PERMISSION FROM GORDON GIBBY
//
#ifndef BEENHERE
#include "vent.h"
#endif

// the lcd should be refered to the main ino. it is specific this implementation
//LiquidCrystal lcd(13,12,11,10,9,8);

#define BTN_FN (1)
#define BTN_DN (2)
#define BTN_UP (4)
#define BTN_PW (8)

#define CHAR_CARET (5)
#define CHAR_TWO   (6)
#define CHAR_BACK  (7)

#define SELECT_BPM              1       // From LCD gui
#define SELECT_PRESSURE         0
#define SELECT_IE               2
#define SELECT_VOLUME           3
#define MAX_SELECTION           3
#define RESET_I2C               4
#define INSP_PAUSE              5
#define SOMETHING_PEEP          6

static uint8_t button_status = 0;
static long next_button_read_time = 0;   //used to debounce the buttons
//static long menu_display_until = 0;
static int screen_top_line = 0;
int selected_item  = -1;
static int menu_cursor = -1;
static int n_menu_items;                     // count of menu items
static int n_display_rows;                      // display rows
static int n_display_columns;

static char ticker[30];
static int  ticker_index = 0;
static long next_ticker_time = 0;
static void make_ticker();
static void clear_menu();
static void display_menu();
static void animate_ticker();

//static LiquidCrystal lcd(8, 9, 10, 11, 12, 13); //when using ubitx board
 LiquidCrystal lcd(13,12,11,10,9,8);  // when using the GAINESVILLE VENTILATOR


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

#define CMD_DISPLAY 0
#define CMD_UP  1
#define CMD_DN 2
#define CMD_SELECTED 3

void menu_exit(struct menu_item *me, int cmd);
void menu_bpm(struct menu_item *me, int cmd);
void menu_pressure(struct menu_item *me, int cmd);
void menu_volume(struct menu_item *me, int cmd);
void menu_ie_ratio(struct menu_item *me, int cmd);
void menu_ventilate(struct menu_item *me, int cmd);
void menu_reset_alarms(struct menu_item *me, int cmd);
void menu_reset_i2c(struct menu_item *me, int cmd);
void menu_pause_inhalation(struct menu_item *me, int cmd);
void menu_reset_sensor(struct menu_item *me, int cmd);
void menu_inspiratory_pause(struct menu_item *me, int cmd);
void menu_peep_desired(struct menu_item *me, int cmd);   // GLG -- add a way to change the PEEP setting


struct menu_item {
  int id;
  void (*menu_item_handler)(struct menu_item *me, int cmd);
};

// Add your menu items here
struct menu_item menu_list[] = {
  {0, menu_exit},
  {1, menu_bpm},
  {2, menu_pressure},
  {3, menu_peep_desired},
  {4, menu_volume},
  {5, menu_ie_ratio},
  {6, menu_ventilate},
  {7, menu_reset_alarms},  
  {8, menu_inspiratory_pause},
  {9, menu_reset_sensor}
  };


void lcd_clear_screen() {
  //clear the screen
  for (int y = 0; y < n_display_rows; y++) {
    lcd.setCursor(0, y);
    for (int x = 0; x < n_display_columns; x++)
      lcd.print(' ');
  }
  //lcd.setCursor(0,2);
  //lcd.print("1234567890123456");
}
void lcd_status(char *text) {
  if (lcd_size == LCDDISPLAY)
    lcd.setCursor(13, 0);
  else
    lcd.setCursor(14, 3);
  lcd.print(text);
}

//pads up the string with spaces to the end of the line
void lcd_message(char *text) {
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

  //  lcd_size = display_size;

  Serial.println("Initializing the display");
  if (display_size == 2004)
    lcd.begin(20, 4);
  else
    lcd.begin(16, 2);

  n_display_rows    = LCDDISPLAY % 10;
  n_display_columns = LCDDISPLAY / 10;
  n_menu_items = sizeof(menu_list) / sizeof(struct menu_item); //just a way of counting them
  Serial.print("Menu Items Number = ");
  Serial.print(n_menu_items);
  Serial.print("\n");
  
  
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

  lcd_graph_clear();
  lcd_clear_screen();

  //  paint_lcd();
  make_ticker();
  animate_ticker();
}
/*
   Ticker makes it easy for longer messages to scroll
   We use only 12 character long scroll ticker, you can increase it
   in animate ticker
*/

void make_ticker() {
  char num_str[5];

  ticker[0] = 0;

  if (vent_running)
    strcat(ticker, "*ON* ");
  else
    strcat(ticker, "*OFF* ");

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
  if (next_ticker_time > millis())
    return;

  lcd.setCursor(0, n_display_rows - 1);
  if (!vent_running){
    lcd.print("<<VENT OFF>>");
    return;
  }
  
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

  lcd.setCursor(12, 0);
  for (int i = 0; i < 5; i++)
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

  //stop this while the menu is on display
  if (menu_cursor != -1)
    return;

  //reset the framebuffer
  memset(framebuffer, 0x0, sizeof(framebuffer));

  if (vent_running) {


    for (int i = 0; i < current_phase; i++) {
      int b = bargraph[i] / 40;
      if (b > 7)
        b = 7;
      if (b < 0)
        b = 0;
      for (int j = 7; j >= 7 - b; j--)
        lcd_pixel(i, j);
    }
  }
  lcd_refresh();

  char buff[17];
  char buff2[6];  // temp storage

  itoa(bargraph[current_phase] / 10, buff, 10); // bargaph deprecated

  
    if (lcd_size == LCDDISPLAY)
     lcd.setCursor(0, 0);
    else
     lcd.setCursor(14, 0);

    Serial.print("\nPeak Inspiratory Pressure:  ");
    Serial.print(peakinspiratorypressure);
    itoa(peakinspiratorypressure, buff, 10); // print the peak inspiratory pressure GLG---
    
    strcat(buff, "cm");
 //   if (strlen(buff) < 4)
//      lcd.print(' ');
    lcd.print(buff);
    itoa(exmLPerCycle, buff, 10); // print out the expiratory tidal volume detected
    //strcat(buff, buff2);  // add it to the display
    strcat(buff, "mL");
    lcd.print(buff);
    itoa(current_pressure,buff,10);// print the current airway pressure  GLG ---
    strcat(buff,"cm");
    lcd.print(buff);

// NOW HANDLE MAKING ALARMS PAINFULLY OBVIOiuS
lcd.setCursor(0,0); 
  //    LOW pressure alarm become valid after about slice 10;   Display LATE (>7  < 12)  and reset only at slice 0  
  //    HI pressure alarm becomes valid anywhere between 0 and 10  Display LATER (>11 < 16)  but must LATCH -- not be reset until slice 0
  //    LO Volume alarm valid at slice MAX_PHASES -- don't reset until cut_off+4 and display early (<4 )  in the slices
  //    HI Volume alarm valid at slide MAX_PHASES -- don'r reset until cut_off+4 and display early (>3  < 8) in the slices
  if(current_phase < 4 && alarm_array[2]==1) lcd.print("***LO VO***");  // print LOW VOLUME alarms
  if(current_phase > 3  && current_phase < 8 && alarm_array[3]==1) lcd.print("***HI VO***");  // print high press alarms slices 4,5,6,7
  if(current_phase > 7  && current_phase < 12 && alarm_array[0] ==1 ) lcd.print("***LO PR***");  // print low volume alarms slices 8,9,10,11
  if(current_phase >11  && current_phase < 16 && alarm_array[1] ==1)  lcd.print("***HI PR***");  // print high volume alarms slices 12, 13, 14, 15



    
}


/*
   The button update scans to see if any buttons have been lifted or pressed since the lasst scan
   Upon change, the next scan is deferred until 200 msec to allow the buttons to settled down
   from the mechanical bounce
*/
bool button_update() {
  uint8_t new_status = 0;

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

  //debounce, so, read next button update only after 200 msec
  if (new_status != button_status) {
    next_button_read_time = millis() + 200;
    button_status = new_status;
    return true;
  } else
    return false;
}

void menu_exit(struct menu_item *m, int cmd) {

  switch (cmd) {
    case CMD_SELECTED:
      clear_menu();
      break;
    case CMD_DISPLAY:
      lcd.print("Exit \007");
  }
}

void menu_bpm(struct menu_item *m, int cmd) {

  if (cmd == CMD_DN && beats_per_minute > 10){
    beats_per_minute--;
    save_settings();    
  }
  else if (cmd == CMD_UP && beats_per_minute < 30){
    beats_per_minute++;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print("Breaths ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(beats_per_minute);
    lcd.print("/min");
  }
}

void menu_pressure(struct menu_item *m, int cmd) {

  if (cmd == CMD_DN && vent_pressure_limit > 5){
    vent_pressure_limit -= 1;
    save_settings();    
  }
  else if (cmd == CMD_UP && vent_pressure_limit < 60)
    vent_pressure_limit += 1;

  if (cmd == CMD_DISPLAY) {
    lcd.print("Pressure");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(vent_pressure_limit);
    lcd.print("cm");
  }
}

void menu_volume(struct menu_item *m, int cmd) {

  if (cmd == CMD_UP && desired_TV < 900){
    desired_TV += TV_INCREMENT;
     save_settings();    
  }   
  else if (cmd == CMD_DN && desired_TV > 200){
    desired_TV -= TV_INCREMENT;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print("Volume  ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(desired_TV);
    lcd.print("mL ");
  }
}

void menu_ie_ratio(struct menu_item *m, int cmd) {
  if (cmd == CMD_UP && ie_ratio < 4){
    ie_ratio++;
    save_settings();    
  }    
  else if (cmd == CMD_DN && ie_ratio > 1){
    ie_ratio--;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print("IE Ratio");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print("1/");
    lcd.print(ie_ratio);
  }
}

void menu_ventilate(struct menu_item *m, int cmd) {
  
  if (CMD_SELECTED && selected_item == m->id) {
    if (vent_running == 0)
      vent_start();
    else
      vent_abort();

    save_settings(); 
    clear_menu();
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print("Vent    ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (vent_running)
      lcd.print("On");
    else
      lcd.print("Off");
  }
}

void menu_inspiratory_pause(struct menu_item *m, int cmd) {
  if (CMD_SELECTED && selected_item == m->id) {
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
    lcd.print("I-Pause ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');

    if (inspiratory_pause == 1)
      lcd.print("Yes");
    else
      lcd.print("No");
  }
}

void menu_reset_alarms(struct menu_item *m, int cmd) {
  if (CMD_SELECTED && selected_item == m->id) {
    //reset all the alarms
    alarm_silence(60);  // silence them for a minute
    alarm_low_pressure = false;
    alarm_high_pressure = false;
    alarm_low_volume = false;
    alarm_high_volume = false;
    clear_menu();
  }

  if (cmd == CMD_DISPLAY) {
    lcd.print("Alarms   ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
   lcd.print("Reset");
  }
}


void menu_reset_sensor(struct menu_item *m, int cmd) {
  if (CMD_SELECTED && selected_item == m->id) 
    {
    lcd.print("***RESET***");
    vent_abort();
    
    i2c_status = I2C_READY;
    i2c_allowed = 1;
    save_settings(); 
    init_pressure_sensor();
    vent_init();            //  Initialize in order to get the atomospheric again!! GLG---
    measure_atmospheric_pressure();  // measure the atmospheric pressure!! GLG ---
   #ifdef SERIALPRINT
   Serial.print( (char *)"**RESETTING I2C\n");
   #endif
    }
  
  if (cmd == CMD_DISPLAY)
    lcd.print("Reset I2C  ");
    lcd.print("A:");
    lcd.print(i2c_allowed);
    
    
}

/*---------Gibby tries to add a menu item -----------*/
void menu_peep_desired(struct menu_item *m, int cmd) {

  if (cmd == CMD_DN && desired_peep > 3){
    desired_peep--;
    save_settings();    
  }
  else if (cmd == CMD_UP && desired_peep < 25){
    desired_peep++;
    save_settings();    
  }
  
  if (cmd == CMD_DISPLAY) {
    lcd.print("PEEP ");
    if (selected_item == m->id)
      lcd.print((char) CHAR_CARET);
    else
      lcd.print((char)':');
    lcd.print(desired_peep);
    lcd.print("cm");
  }
}




void clear_menu(){
  menu_cursor = -1;
  selected_item = -1; 
  lcd_clear_screen();
}

void display_menu() {
  int n_columns = LCDDISPLAY / 10;
  lcd_menu_characters();                                      //add custom characters

  for (int i = 0; i < n_menu_items; i++) {
    int item_position = i - screen_top_line;
    if (item_position < 0 || item_position >= n_display_rows)
      continue;

    //clear the display to the end of the line
    lcd.setCursor(0, item_position);
    for (int j = 0; j < 16; j++)
      lcd.print(' ');
    lcd.setCursor(0, item_position);
    //show the caret on the menu item if only the item is not selected
    //selected items show the caret _inside_ the item
    if (menu_cursor == i && selected_item != i)
      lcd.print((char)CHAR_CARET);
    else
      lcd.print(" ");
    menu_list[i].menu_item_handler(menu_list + i, CMD_DISPLAY);

  }
}

void lcd_show_alarms() {
  if (!vent_running)
    return;

  //rubout the alarms
  lcd.setCursor(12,1);
  lcd.print("    ");
  
  int trouble = 0;
  if (current_phase < 4 && alarm_low_pressure == true) {
    lcd.setCursor(12, 1);
    lcd.print("Lp");
    trouble++;
  }
  if (current_phase > 3  && current_phase < 8 && alarm_high_pressure == true) {
    lcd.setCursor(12, 1);
    lcd.print("Hp");
    trouble++;
  }
  if (current_phase > 7  && current_phase < 12 && alarm_low_volume == true) {
    lcd.setCursor(14, 1);
    lcd.print("Lv");
    trouble++;
  }
  if (current_phase > 11  && current_phase < 16 && alarm_high_volume == true) {
    lcd.setCursor(14, 1);
    lcd.print("Hv");
    trouble++;
  }

  if (!trouble){
    lcd.setCursor(12, 1);
    lcd.print("-OK-");
  }
}

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
      menu_cursor = 5;  // 5th item is ventilate. this jumps directly to this menu
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
