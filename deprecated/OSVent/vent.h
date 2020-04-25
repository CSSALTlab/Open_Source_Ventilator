void guiUpdate();
void checkCAT();
void checkTouch();

void tft_init();
void tft_slice();
void tft_status(char *text);
void tft_graph_update();
void tft_graph_clear();

//the lcd_size is 1602 (16x2) or 2004 (20x4)
void lcd_init(int display_size);
void lcd_slice();
void lcd_status(char *text);
void lcd_message(char *text);
void lcd_graph_update();
void lcd_graph_clear();
void save_settings();

extern int vent_on;
extern int vent_off;

extern int beats_per_minute;
extern int ie_ratio; //this is the denominator
extern int vent_pressure;//this is in mm
extern int vent_running;
extern int vent_temperature;
extern int is_pressure_on;
void vent_abort();
void vent_start();


#define MAX_PHASES 40
extern int current_phase;
extern int bargraph[];

void alarm(int action);
void alarm_slice();
#define ALARM_FAST  2
#define ALARM_SLOW  1
#define ALARM_OFF   0
;
