#ifndef BEENHERE
#include "vent.h"
#endif

struct Button {
  int x, y, w, h;
  char *text;
};

//#define MAX_BUTTONS 16
const struct Button btn_set[MAX_BUTTONS] PROGMEM = { 
//const struct Button  btn_set [] = {
  
  {0, 80, 188, 36,    (char *) "BPM"},
  {192, 80, 60, 36,   (char *) "FST"},
  {256, 80, 60, 36,   (char *) "SLO"},

  {0, 120, 188, 36,   (char *) "PRES"},
  {192, 120, 60, 36,  (char *) "UP"},
  {256, 120, 60, 36,  (char *) "DN"},

  {0, 160, 78, 36,    (char *) "1:1"},
  {80, 160, 78, 36,   (char *) "1:2"},
  {160, 160, 78, 36,  (char *) "1:3"},
  {240, 160, 78, 36,  (char *) "1:4"},

  {0, 200, 158, 36,   (char *) "ON"},
  {160, 200, 158, 36, (char *) "OFF"},
//  {256, 200, 60, 36, (char *) "?"},
};

boolean getButton(char *text, struct Button *b){
  for (int i = 0; i < MAX_BUTTONS; i++){
    memcpy_P(b, btn_set + i, sizeof(struct Button));
    if (!strcmp(text, b->text)){
      return true;
    }
  }
  return false;
}

void tft_graph_clear(){
  quickFill(0, 0, 319, 78, DISPLAY_BLACK); 
  quickFill(50,60, 319,60, DISPLAY_WHITE);
  quickFill(50,40, 319,40, DISPLAY_WHITE);
  quickFill(50,20, 319,20, DISPLAY_WHITE);
  displayText( (char *) "mm", 0,22,40,16, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  
  Serial.println("*********** CLEAR ***************");
}

void tft_graph_update(){
  char buff[5];
  
  int barval = bargraph[current_phase];
  if (barval < 0)
    barval = 0;

  itoa(barval, buff, 10);
  displayText(buff, 0,0,40,18, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
  
  barval /= 5;
  int x = 50 + (current_phase * 5);

  if (barval > 50){
    barval = 50;
    quickFill(x, 60-barval, x+4, 60, DISPLAY_RED);
  }
  else
    quickFill(x, 60-barval, x+4, 60, DISPLAY_YELLOW);

  if (is_pressure_on)
    quickFill(x, 65, x+3, 72, DISPLAY_WHITE);
  else
    quickFill(x, 65, x+3, 72, DISPLAY_DARKGREY);
  
/*
  Serial.print(x);
  Serial.print(":");
  Serial.println(barval);
  */
}
void btnDraw(struct Button *b){
  char buff[10];
  
  if (!strcmp(b->text, "BPM")){
    sprintf(buff, "%d BPM", beats_per_minute);
    displayText(buff, b->x, b->y, b->w, b->h, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_DARKGREY);
  }
  else if (!strcmp(b->text, "PRES")){
    sprintf(buff, "%d mm", pressure);
    displayText(buff, b->x, b->y, b->w, b->h, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_DARKGREY);
  }
  else if (!strcmp(b->text, "1:1") && ie_ratio == 1 || !strcmp(b->text, "1:2") && ie_ratio == 2 ||
  !strcmp(b->text, "1:3") && ie_ratio == 3 || !strcmp(b->text, "1:4") && ie_ratio == 4)
    displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_BLACK, DISPLAY_ORANGE, DISPLAY_DARKGREY);
  else if (!strcmp(b->text, "ON") && vent_running == 1)
    displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_BLACK, DISPLAY_GREEN, DISPLAY_DARKGREY);
  else if (!strcmp(b->text, "OFF") && vent_running == 0)
    displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_BLACK, DISPLAY_RED, DISPLAY_DARKGREY);
  
  
  /*else if(!strcmp(b->text, "VFOB")){
    memset(vfoDisplay, 0, sizeof(vfoDisplay));    
    displayVFO(VFO_B);
  }
  else if ((!strcmp(b->text, "RIT") && ritOn == 1) || 
      (!strcmp(b->text, "USB") && isUSB == 1) || 
      (!strcmp(b->text, "LSB") && isUSB == 0) || 
      (!strcmp(b->text, "SPL") && splitOn == 1))
    displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_BLACK, DISPLAY_ORANGE, DISPLAY_DARKGREY);   
  else if (!strcmp(b->text, "CW") && cwMode == 1)
      displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_BLACK, DISPLAY_ORANGE, DISPLAY_DARKGREY);   
  */
  else 
    displayText(b->text, b->x, b->y, b->w, b->h, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_DARKGREY);
}


void guiUpdate(){
  
  // use the current frequency as the VFO frequency for the active VFO
  displayClear(DISPLAY_NAVY);

  //force the display to refresh everything
  //display all the buttons
  for (int i = 0; i < MAX_BUTTONS; i++){
    struct Button b;
    memcpy_P(&b, btn_set + i, sizeof(struct Button));
    btnDraw(&b);
  }
  displayText( (char *) "sensor is on", 0,0,320,60, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
//  checkCAT(); 
}

void setIERatio(int i){
  Button b;

  ie_ratio = i;
  getButton((char *) "1:1", &b);
  btnDraw(&b);

  getButton((char *) "1:2", &b);
  btnDraw(&b);

  getButton((char *) "1:3", &b);
  btnDraw(&b);

  getButton((char *) "1:4", &b);
  btnDraw(&b);
}
void doCommand(struct Button *b){
  Button b2;
  Serial.println(b->text);

  if (!strcmp(b->text, (char *) "FST") && beats_per_minute < 30){
    beats_per_minute++;
    getButton((char *) "BPM", &b2);
    btnDraw(&b2);
  }
  else if(!strcmp(b->text, (char *) "SLO") && beats_per_minute > 10){
    beats_per_minute--;
    getButton((char *) "BPM", &b2);
    btnDraw(&b2);
  }
  else if (!strcmp(b->text, (char *) "UP") && pressure < 30){
    pressure++;
    getButton((char *) "PRES", &b2);
    btnDraw(&b2);
  }
  else if(!strcmp(b->text, (char *) "DN") && pressure > 0){
    pressure--;
    getButton((char *) "PRES", &b2);
    btnDraw(&b2);
  }
  else if (!strcmp(b->text, (char *) "1:1")){
    setIERatio(1);
  }
  else if (!strcmp(b->text, (char *) "1:2")){
    setIERatio(2);
  }
  else if (!strcmp(b->text, (char *) "1:3")){
    setIERatio(3);
  }
  else if (!strcmp(b->text, (char *) "1:4")){
    setIERatio(4);
  }
  else if (!strcmp(b->text, (char *) "ON")){
    vent_running = 1;
    tft_graph_clear();
    btnDraw(b);
    getButton((char *) "OFF", &b2);
    btnDraw(&b2);
  }
  else if (!strcmp(b->text, (char *) "OFF")){
    vent_running = 0;
    tft_graph_clear();
    btnDraw(b);
    getButton((char *) "ON", &b2);
    btnDraw(&b2);
  }
}

void tft_init(){
  displayInit();
  guiUpdate();
}

void  tft_slice(){

  if (!readTouch())
    return;

  Serial.print("#");
  while(readTouch())
    checkCAT();
  scaleTouch(&ts_point);
  Serial.print(ts_point.x);Serial.print(":");Serial.println(ts_point.y);
 
  /* //debug code
  Serial.print(ts_point.x); Serial.print(' ');Serial.println(ts_point.y);
  */
  int total = sizeof(btn_set)/sizeof(struct Button);
  for (int i = 0; i < MAX_BUTTONS; i++){
    struct Button b;
    Serial.print('.');
    memcpy_P(&b, btn_set + i, sizeof(struct Button));

    int x2 = b.x + b.w;
    int y2 = b.y + b.h;

    if (b.x < ts_point.x && ts_point.x < x2 && 
      b.y < ts_point.y && ts_point.y < y2)
          doCommand(&b);
  }
}

void tft_status(char *text){
  displayText(text, 0,0,320,78, DISPLAY_GREEN, DISPLAY_BLACK, DISPLAY_BLACK);
}
