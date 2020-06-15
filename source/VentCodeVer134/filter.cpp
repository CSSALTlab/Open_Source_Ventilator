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

// variables for the predictive time ADAPTIVE2 (ILC) algorithm
//int           Tbottomlimit;  // earliest "time" we will even consider closing the expiration valve in expiration to try and set peep // TJ 06.15.2020 "time" in previous comment should be "vent slice"
int			  exp_valve_closure_cycle; // TJ 06.15.2020 earliest cycle we will even consider closing the exhalation valve. New variable to replace old variable "Tbottomlimit".
int           exp_valve_last_closed;   // 1 = we closed the valve in expiration to maintain peep and will not reopen it
unsigned long exp_valve_last_closed_time ; // time that we closed the vale to try and hold PEEP

unsigned long exp_valve_first_open_time; 
int           exp_valve_first_open;
int           loopcounter;   //TJ 06.15.2020 How is loopcounter counted? when is it reset? when does it start counting?
int           wastedloops;   // counts the loops between nearly no peep and exp_valve_closure_cycle
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
										// TJ 06.15.2020 TimeCycledFilter is Iterative Learning Control (ILC) algorithm 
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
  //GLG0531 - removed some really old commented out code                  
    
     
   #endif

   
  
  if(loopcounter < exp_valve_closure_cycle) { 
    // leave the valve open
    output2 = desired_peep+5;
    if(pressure< desired_peep/2) wastedloops++;   // counts the number of loops between reaching very little peep and exp_valve_closure_cycle
    }

  if(loopcounter>exp_valve_closure_cycle || loopcounter==exp_valve_closure_cycle || current_phase==MAX_PHASES) {
   // GLG0531  GLG127 -- added the last clause to force an evaluation at least at the end of the entire respiratory cycle
    // in the case that we have incremented exp_valve_closure_cycle far too high....
     output2 = desired_peep-5;  // close the valve

// GLG0601 Oops!! turns out it was NOT done in pressure_off....so I went back there and added t back in.... 
 /* Not needed, already occurs in pressure_off() SDS120b    
    // GLG121a  -- if we have not previously closed it now we need to mark it
    if(exp_valve_last_closed==0)  // GLG121a
      {
        exp_valve_last_closed = 1;  // GLG121a this got set to 0 at cut_off
        exp_valve_last_closed_time = millis();   // GLG121a time of closure

        Serial.println(F("EValve closed for PEEP"));

    }
    */
    }

    loopcounter++;   // increment the loopcounter


        
     //////////////////////////CHECK PEEP   ONCE  ///////////////////           
      // if we close it, now we need to study the outcome 
      // this will take some time to settle......
		//TJ 06.15.2020 
       if ((( 
		   (exp_valve_last_closed==1) 
		   && (current_phase>cut_off) 
		   && (now > (exp_valve_last_closed_time + PEEPMEASUREMENTDELAY)) ) 
		   || (current_phase==MAX_PHASES) ) 
           && tested_settled_peep==0 ) 
           
              // GLG 121a  test the peep after ringing has stopped after clsoing exp valve.. 
              //GLG0602  -- added in several more parentheses in an atempt to FORCE checking if it has never been tested at all
              // GLG0531 added in my ver127 fix that forces AT LEAST ONE test
              // of the peep value in the entire respiratory cycle
              // so as to defeat a failure mode we discovered with
              // tony opening circut for a while and THEN connecting!
      {
        // now measure where PEEP ended up at (given to use as pressure) and adjust exp_valve_closure_cycle
        
        #ifdef FILTERSPY
        Serial.print(F("**CHECKING**pressure:  "));
        Serial.println(pressure);
        Serial.print(F("desired_peep: "));
        Serial.println(desired_peep);
        Serial.print(F("Loops since jump: "));
        Serial.println(loops_since_major_jump);        
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
		//TJ 06.12.2020 - THIS IS Iterative Learning Control (ILC)
        /*--------FIXING BELOW DESIRE PEEP (exp_valve_closure_cycle IS TOO LARGE)------------------------*/
        // CASE: WAY BELOW 
        // We are way below desired_peep -- need to open the valve a third less
        // We allow ONE of these immmediately, but then you can't do another unless 7 small steps have been made. 
		//TJ 06.12.2020 - loops_since_major_jump in lcd_ui is probably for tuning without having to change and recompile code
		if (pressure < (desired_peep/2)  && loops_since_major_jump>8) {
          exp_valve_closure_cycle = (exp_valve_closure_cycle*2)/3; 
          #ifdef FILTERSPY //Filterspy is a variable which toggles print statements if defined
          Serial.println(F("Jump down exp_valve_closure_cycle"));         
          #endif
          loops_since_major_jump=0  ;   // track when we make major jumps
        }

        // CASE: SLIGHTY BELOW
        // We are sightly below desired_peep; need to open a litte less 
		// TJ 06.15.2020 Changed this statment to be "else if" instead of just "if". This will prevent both "WAY BELOW" and "SLIGHTLY BELOW" conditions to excute if pressure is "WAY BELOW"
        else if (pressure <desired_peep)  
          {
          exp_valve_closure_cycle--  ;// 
          if(exp_valve_closure_cycle<1) exp_valve_closure_cycle=1;  
              // establish that it must be positive
              // due to valve latency, no reason to go below 1
          // GLG0531 -- added in this fix from ver127 that avoided the 
          // complete failure that Tony managed to provoke where 
          // Tbottom limit went way negative and then shot way positive.
          #ifdef FILTERSPY
          Serial.println(F("Decrement exp_valve_closure_cycle"));         
          #endif
          }


        /*--------FIXING ABOVE DESIRE PEEP (exp_valve_closure_cycle IS TOO SMALL)------------------------*/
          // The changes that REDUCE PRESSURE won't be limited because they are patient protective in a very important way. 
          // provide some patient protection if the pressure is significantly higher than desired
          // this will also help the filter get to a converged condition sooner

          // SAFETY:  IF MORE THAN 4 ABOVE, OPEN EXPIRATORY VAVE RELEASE TO ZERO
         
		//TJ 06.15.2020 - Removed "break glass" logic below. If the airway pressure was high enough to damage a lung, it would be during peak inspiration. 
		// There is no emergency situation during exhalation.
		/*
		if(pressure>desired_peep+4){
            digitalWrite(MOTOR_B, LOW); // drop the pressure //TJ 06.12.2020 this will cause exhalation valve to open more than once, even though ILC is designed to open exh. valve only once per exhalation
            //loops_since_major_jump=0  ;   // track when we make major jumps
          }
		  */

          // CASE:   WAY HIGH
          // We are way high -- need to open a lot less
          if  (pressure>desired_peep*3/2){
            exp_valve_closure_cycle = exp_valve_closure_cycle*3/2;
            loops_since_major_jump=0  ;   // track when we make major jumps
          }

          // CASE;  SLIGHTLY HIGH
          // We are slightly high -- open just a little longer 
		  // TJ 06.15.2020 Changed this statment to be "else if" instead of just "if". This will prevent both "WAY HIGH" and "SLIGHTLY HIGH" conditions to excute if pressure is "WAY HIGH"
		  else if (pressure>desired_peep) exp_valve_closure_cycle++; //TJ 06.12.2020 What are the units of exp_valve_closure_cycle? 
          tested_settled_peep = 1;  // set a flag so it doesn't get done again
          
      }   //End of checking pressue and adjusting exp_valve_closure_cycle
        

  #ifdef TOTALOPENINGSPY
  Serial.print(F("T-b-l: "));
  Serial.println(exp_valve_closure_cycle);
  #endif
  

  #ifdef FILTERSPY
  Serial.print(F("Filter output2= "));
  Serial.println(output2);
  Serial.print(F("T-b-l: "));
  Serial.println(exp_valve_closure_cycle);
  #endif
    
  return(output2);
     
}
