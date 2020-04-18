#include <Arduino.h>
#include "vent.h"
#define ALARM_PIN 6

int alarm_status = ALARM_OFF;
long alarm_on_until = 0;
long alarm_off_until = 0;
static long now;
int is_ringing = 0;

void alarm_slice(){

  now = millis();
  if (!alarm_status)
    return;

  //Serial.print("off");Serial.print(alarm_off_until);
  //Serial.print(", on");Serial.println(alarm_on_until);

  if (alarm_off_until < now){

    if (alarm_status == ALARM_FAST){
      alarm_on_until = now + 100;
      alarm_off_until = now + 200;
    }
    else {
      alarm_on_until = now + 300;
      alarm_off_until = now + 600;
    }
    if (is_ringing == 0){
      tone(ALARM_PIN, 1000);
      is_ringing = 1;
    }
    return;
  }    

  if (alarm_on_until < now){
    if (is_ringing)
      noTone(ALARM_PIN);
    is_ringing = 0;
  }  
  //Serial.print("alarm off until: ");Serial.println(alarm_off_until);
  //Serial.println(__LINE__);

}

void alarm(int action){
  if (action == ALARM_OFF){
    noTone(ALARM_PIN);
    alarm_status = ALARM_OFF;
    alarm_on_until = 0;
    alarm_off_until = 0;
    //Serial.println("alamr off");
  }
  else if (alarm_status == ALARM_OFF){
    alarm_status = action;
    alarm_on_until = 0;
    alarm_off_until = 0;
    //Serial.print("alarm on");
    //Serial.println(__LINE__);
  }
  alarm_slice();
}
