#include <Arduino.h>
#include <LiquidCrystal.h>
#include "vent.h"

LiquidCrystal lcd(8,9,10,11,12,13); //when using ubitx board
//LiquidCrystal lcd(13,12,11,10,9,8);
#define FN A0
#define UP A1
#define DOWN A2
#define ON_OFF A3


#define SELECT_BPM 1
#define SELECT_PRESSURE 0
#define SELECT_IE 2

#define MAX_SELECTION 2
int selected_item = SELECT_BPM;
int lcd_size = 1602;

void lcd_status(char *text){
  if (lcd_size == 1602)
    lcd.setCursor(13,0);
  else
    lcd.setCursor(14,3);
  lcd.print(text);
}

void lcd_message(char *text){
  if (lcd_size == 1602){
    int i, padding = (16 - strlen(text))/2; 
    lcd.setCursor(0,0);
    for (i = 0; i < padding; i++)
      lcd.print(' ');
    lcd.print(text);
    i += strlen(text);
    while(i++v < 16)
      lcd.print(' ');
  }
  else
    lcd.setCursor(10,3);
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
    lcd.createChar(i, framebuffer + (i * 8));
      
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

  char buff[10];
  
  itoa(bargraph[current_phase]/10, buff, 10);
  if (lcd_size == 1602)
    lcd.setCursor(0,0);
  else
   lcd.setCursor(14,0);
  for (int i=0; i < 2-strlen(buff); i++)
    lcd.print(' ');
  lcd.print(bargraph[current_phase]/10);lcd.print("cm");
  lcd.print(' ');
  itoa(vent_temperature, buff, 10);
  strcat(buff, "c");
  if (lcd_size == 1602)
    lcd.setCursor(5,0);
  lcd.print(buff);
}


void paint_lcd(){
  lcd.setCursor(0,2);
  lcd.print((char)0x3E);

  if (lcd_size == 1602){

    //paint the user interface
    lcd.setCursor(0,1);
    if (selected_item == SELECT_PRESSURE) 
      lcd.print('[');
    else
      lcd.print(' ');  
    lcd.print(vent_pressure/10);
    lcd.print("cm");
    
    if (selected_item == SELECT_PRESSURE) 
      lcd.print(']');
    else if (selected_item == SELECT_BPM)
      lcd.print('[');
    else
      lcd.print(' ');
  
    lcd.print(beats_per_minute);
    lcd.print("BMP");
    if (selected_item == SELECT_BPM) 
      lcd.print(']');
    else if (selected_item == SELECT_IE)
      lcd.print('[');
    else
      lcd.print(' ');
    lcd.print("1:"); 
    lcd.print(ie_ratio);  
    if (selected_item == SELECT_IE) 
      lcd.print(']');
    else
      lcd.print(' ');
  }
  else {
    //paint the user interface
    lcd.setCursor(0,0);
    lcd.print(' ');
    lcd.print(vent_pressure/10);
    lcd.print("CM ");
  
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

  lcd_size = display_size;

  Serial.println("Initializing the display");
  //lcd.begin(20, 4);
  if (lcd_size == 1602)
    lcd.begin(16,2);
  else
    lcd.begin(20,4);
  pinMode(FN, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(ON_OFF, INPUT_PULLUP);

  lcd_graph_clear();
  paint_lcd();
}

int getButton(){
  if (digitalRead(FN) == LOW)
    return FN;
  else if(digitalRead(UP) == LOW)
    return UP;
  else if (digitalRead(DOWN) == LOW)
    return DOWN;
  else if (digitalRead(ON_OFF) == LOW)
    return ON_OFF;
  else
    return 0;  
}

void lcd_slice(){
  
  int action = getButton();

  if (!action)
    return; 
  
  Serial.print("Btn:");Serial.println(action);
  
  if (action == ON_OFF){
    if (vent_running)
      vent_abort();
    else
      vent_start();
  }
  if (action == FN){
    selected_item++;
    if (selected_item > MAX_SELECTION)
      selected_item = 0;
  }
  if (action == UP){
    if (selected_item == SELECT_BPM && beats_per_minute < 30)
      beats_per_minute++;
    if (selected_item == SELECT_PRESSURE && vent_pressure < 300)
      vent_pressure += 10;
    if (selected_item == SELECT_IE && ie_ratio < 4)
      ie_ratio++;
  }

  if (action == DOWN){
    if (selected_item == SELECT_BPM && beats_per_minute > 0)
      beats_per_minute--;
    if (selected_item == SELECT_PRESSURE && vent_pressure > 10)
      vent_pressure -= 10;
    if (selected_item == SELECT_IE && ie_ratio > 1)
      ie_ratio--;
  }
  save_settings();
  paint_lcd();
  //debounce
  while(getButton())
    ;
  delay(100);
}
