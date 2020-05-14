#ifndef BEENHERE
#include "vent.h"
#endif



//////////////////////PID FILTER CONSTANTS////////////////////////////////
#define P     1     // Proportional coefficient
#define I      2.5   // Integral coefficient (10 steps into the past)
#define D     .02       // First Derivative   // was .02
#define D2    0        // Second Derivative
//////////////////////////////////////////////////////////////////////////

float   phistory[SIZEOFFILTER +1 ];     // past history
int     peep_filter_pressure;  // use this only to drive the PEEP choices
int     weightprint;  // to print the adaptive weights
float   drop;  // value to use to calculate the drop

// variables for the predictive time ADAPTIVE2 algorithm
int           Tbottomlimit;  // earliest time we will even consider closing the expiration valve in expiration to try and set peep
int           exp_valve_last_closed;   // 1 = we closed the valve in expiration to maintain peep and will not reopen it
unsigned long exp_valve_last_closed_time ; // time that we closed the vale to try and hold PEEP
int           exp_valve_reopen; 
unsigned long exp_valve_first_open_time; 
int           exp_valve_first_open;
int           loopcounter;   
int           wastedloops;   // counts the loops between nearly no peep and Tbottomlimit
int           total_exp_valve_open_time;  // counts the total open time for the PID filter
int           loops_since_major_jump;   // track when we made a major jump in the exp open time 



int PIDfilter(int pressure) {          // filter that massages the current pressure received as argument
                                      // and passes back the analog altered value to send to pressure_off()
                                      // -- the PID is more predicitive of how fast the pressure will drop
                                      // or rise and thus gives the system a better chance of succeeding
                                      // and does not require any "learning time"
                                      // Since it works with instantaneously available pressures, it is 
                                      // better for the assistive ventilation solution
  int x, output2; 
  float derivative,integral,proportional,output;
  
    // NOTE that this filter is dependent on time steps of approx
    // 25 milliseconds.   If the code is changed to run more quickly,
    // the coefficients in the derivative and integral portions will
    // need to be adjusted
    //


  for (x=SIZEOFFILTER ;x>0;x--) {
    phistory[x] = phistory[x-1];
    }
    phistory[0] = pressure;    // stick in the actual pressure received in the call

  derivative = phistory[0]-phistory[1]/(.025)  ;    // assumed 25 mSec windows
  integral=0;
  for(int y=0;y<10;y++) integral=integral+ (phistory[y]);  
  integral= .025 * integral;    // scaling factor
  proportional = phistory[0];   

  output = (P * proportional) + (D*derivative) + (I*integral) ;
  output2 = (int) output;

  #ifdef PIDFILTERSPY
  Serial.print(F("PID output2: "));
  Serial.println(output2);
  #endif
  
  if(output2>desired_peep) total_exp_valve_open_time++;
  return(output2);

  }   // END of subroutine



////////////////////////////////////////////////////////////////////////////
int TimeCycledFilter(int pressure) {    // Filter that chooses a single stretch of time for the exp valve to 
                                        // be opened, and checks only once in phase 18, for the result (long
                                        // after the ringing perturbations have gone
                                        // This may take some time to converge and reach the correct opening
                                        // time
  int x, output2; 
  float output=0;
  float lung_compliance;
  float flow_resistance;  
  int instantFlowValueint; 
        
// millis() - exp_valve_first_open_time  would tell you the time since opening the valve.
// if we close early, there will be lots of ringing and lots of re-opening needed to find the moment of optimal closure
// if we close late, thepressure will dip below but come back up.
// so lets wait T milliseconds, close, and then delay until smoothed_run is stable and then measure
// and then adjust

    #ifdef FILTERSPY
    Serial.print(F("Time ms past valve open: "));
    Serial.println(( millis()-exp_valve_first_open_time));
    #endif
  output2 = desired_peep; 

  #ifdef LUNGMEASUREMENTSSPY
    //if(loopcounter>0 && loopcounter<15) {
    // calculate the resistance
    // flow_resistance = pressure/instantFlowValue0;

     instantFlowValueint = (int) instantFlowValue;
     Serial.print(F("time since open: "));
     Serial.print(millis()-exp_valve_first_open_time);  
     Serial.print(F(" "));
     Serial.print(F("LoopCount:  InstF:  "));
     Serial.print(loopcounter);
     Serial.print(F(" "));
     Serial.println(instantFlowValueint);
                   
     /*----------------------
     if(current_exp_volume!=0)
     {
      // instantFlowValue is in liters per minute.    Must convert to liters per second
      // divide the divisor by 60 to get liters per second.  
      flow_resistance =  60* (float)peakinspiratorypressure/instantFlowValue;
      Serial.println(flow_resistance);
      
     }
     else Serial.println(999.0);
     
     }
     ---------------------*/
     
   #endif

   
  
  if(loopcounter < Tbottomlimit) {
    // leave the valve open
    output2 = desired_peep+5;
    if(pressure< desired_peep/2) wastedloops++;   // counts the number of loops between reaching very little peep and Tbottomlimit
    }

  if(loopcounter>Tbottomlimit || loopcounter==Tbottomlimit) {
    output2 = desired_peep-5;  // close the valve
    }

    loopcounter++;   // increment the loopcounter


        
     //////////////////////////CHECK IN PHASE 18   ONCE  ///////////////////           
      // if we close it, now we need to study the outcome 
      // this will take some time to settle......
      
     if (current_phase==18 && tested_settled_peep==0) 
      {
        // now measure where we ended up at (given to use as pressure) and adjust T
        #ifdef FILTERSPY
        Serial.print(F("**CHECKING**pressure:  "));
        Serial.println(pressure);
        Serial.print(F("desired_peep: "));
        Serial.println(desired_peep);        
        #endif

        // Lets determine the compliance of the lung
        #ifdef LUNGMEASUREMENTSSPY
        if (peakinspiratorypressure != pressure) {
          lung_compliance = (exLitersPerCycle) /  (peakinspiratorypressure - pressure) ;   
        }
        else lung_compliance=999;  
        Serial.print(F("Estimated Lung Compliance: "));
        Serial.println(lung_compliance);
        #endif

        loops_since_major_jump++  ;   // keep a track of how many looops since a major jump. 


        // the changes that INCREASE PEEP will be limited so that the system doesn't oscillate from way below
        // to way above -- willing to force it to slowly grow to correct peep as the safety risks are less that way

        // We are way below desired_peep -- need to open the valve a third less
        // We allow ONE of these immmediately, but then you can't do another unless 7 small steps have been made. 
        if (pressure < desired_peep/2  && loops_since_major_jump>8) {
          Tbottomlimit = Tbottomlimit*2/3;
          loops_since_major_jump=0  ;   // track when we make major jumps
        }

        // We are sightly below desired_peep; need to open a litte less 
        if (pressure <desired_peep) {
          Tbottomlimit--  ;// 
          #ifdef FILTERSPY
          Serial.println(F("Decrement Tbottomlimit"));         
          #endif
          }

          // The changes that REDUCE PRESSURE won't be limited because they are patient protective in a very important way. 

          // provide some patient protection if the pressure is significantly higher than desired
          // this will also help the filter get to a converged condition sooner

          if(pressure>desired_peep+4){
            digitalWrite(MOTOR_B, LOW); // drop the pressure
            loops_since_major_jump=0  ;   // track when we make major jumps
          }

          // We are way high -- need to open a lot less
          if  (pressure>desired_peep*3/2){
            Tbottomlimit = Tbottomlimit*3/2;
            loops_since_major_jump=0  ;   // track when we make major jumps
          }

          // We are slightly high -- open just a little longer 
          if (pressure>desired_peep) Tbottomlimit++;
          tested_settled_peep = 1;  // set a flag so it doesn't get done again
      }
        

  #ifdef TOTALOPENINGSPY
  Serial.print(F("T-b-l: "));
  Serial.println(Tbottomlimit);
  #endif
  

  #ifdef FILTERSPY
  Serial.print(F("Filter output2= "));
  Serial.println(output2);
  Serial.print(F("T-b-l: "));
  Serial.println(Tbottomlimit);
  #endif
    
  return(output2);
     
}
