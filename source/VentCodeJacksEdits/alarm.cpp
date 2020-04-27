// NO PERMISSION TO USE THIS CODE FOR ANY PURPOSE WITHOUT 
// WRITTEN CONSENT FROM GORDON GIBBY
#ifndef BEENHERE
#include "vent.h"
#endif

/*****
  Purpose: Returns the alarm status

  Argument list:
    void

  Return value:
    int             1 = alarm ON, 0 alarm OFF
    
  CAUTION:  Doesn't make sense since it's a global
*****/
int alarm_get_status()
{
  return alarm_status;
}

/*****
  Purpose: Turns off alarm for X seconds

  Argument list:
    int silence_in_seconds    Duration of silence in seconds

  Return value:
    void
    
  CAUTION:
*****/
void alarm_silence(int silence_in_seconds)
{
  if (alarm_status != ALARM_OFF && silence_in_seconds > 0){
    alarm_status          = ALARM_OFF;                                       // reset the alarm!
    alarm_on_until        = 0L;
    alarm_off_until       = 0L;
    alarm_suppress_until  = now + ((long)silence_in_seconds * 1000L);
  }
  noTone(ALARM_PIN);
}


/*****
  Purpose: 
  
  Argument list:
    void

  Return value:
    void
    
  CAUTION:
*****/
void alarm_slice()
{

  now = millis();
  if (!alarm_status)
    return;

  //Serial.print("off");Serial.print(alarm_off_until);
  //Serial.print(", on");Serial.println(alarm_on_until);

  if (alarm_off_until < now && alarm_suppress_until <= now){
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


/*****
  Purpose: Sets the alarm status

  Argument list:
    int action        what to do
    
  Return value:
    void
    
  CAUTION:
*****/
void alarm(int action)
{
  if (action == ALARM_OFF){
    noTone(ALARM_PIN);
    alarm_status    = ALARM_OFF;
    alarm_on_until  = 0;
    alarm_off_until = 0;
    //Serial.println("alarm off");
  } else if (alarm_status == ALARM_OFF && alarm_suppress_until < now){
    alarm_status    = action;
    alarm_on_until  = 0;
    alarm_off_until = 0;
    //Serial.print("alarm on");
    //Serial.println(__LINE__);
  }
  alarm_slice();
}
