/******************************************************************************
 Stephanie Lampotang - March 19, 2020
 Open Source Contribution for Free Distribution
 University of Southern California
 Computer Science
 
 Equipment: Arduino Uno, LCD display, wires, buttons, breadboard
 Link to purchase:
 YSP2020
*******************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"

volatile int bpm_chosen; // current breaths per minute that system is operating at
volatile int inhale_time; // amount of time in 1/100 secs that the inhale valve should remain open
volatile int exhale_time; // amount of time in 1/100 secs that the exhale valve should remain open
volatile int inhale_complete; // flag signifying the end of an inhale
volatile int exhale_complete; // flag signifying the end of an exhale
volatile int secs; // hundredths of seconds that have passed in this inhale or exhale
volatile int inhale; // true if system is in the inhale state of the breath, false if in the exhale state of the breath
volatile int exhale_to_inhale; // ration of the duration of the exhale to the duration of the inhale

int main(void)
{
    // Initialize the LCD
    lcd_init();
    // Write splash screen
    lcd_writecommand(1);
    lcd_stringout("fight me covid19 :(");
    // breaths per minute from 10 - 30 in increments of 5
    int bpms[5] = {10, 15, 20, 25, 30};
    // tidal volumes in milliliters from 450 - 800 in increments of 50
    int tidal_volumes[8] = {450, 500, 550, 600, 650, 700, 750, 800};
    _delay_ms(2000);
    lcd_writecommand(1);
    // set up the default parameters
    int bpm_index = 0;
    int vt_index = 5;
    int vt_chosen = tidal_volumes[vt_index];
    exhale_to_inhale = 2;
    // SET parameters
    bpm_chosen = bpms[bpm_index];
    // times are in tenths of a second, so 600 represents 60 secs or 1 min
    inhale_time = 600/(bpm_chosen)/(exhale_to_inhale+1);
    exhale_time = 600/(bpm_chosen)*(exhale_to_inhale)/(exhale_to_inhale+1);
    inhale_complete = 0;
    exhale_complete = 0;
    secs = 0;
    inhale = 1;
    char bpm_string[17];
    snprintf(bpm_string, 17, "bpm: %d, etoi: %d", bpm_chosen, exhale_to_inhale);
    lcd_stringout(bpm_string);

    // Initializing pull-up resistors on PC5, PC4, and PC3 of the arduino
    PORTC |= ((1 << PC5) | (1 << PC4) | (1 << PC3));
    // Set up interrupts for every 1/100th of a second
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 6250;
    TCCR1B |= (1 << CS12);
    // turn on interrupts
    sei();
    
    while (1)
    { // Loop forever
        if ((PINC & (1 << PC5)) == 0) { // button pressed -> recalculate
            // debouncing code
            _delay_ms(5);
            while ((PINC & (1 << PC5)) == 0) {}
            bpm_index++;
            if (bpm_index >= 5) { // goes out of bounds -> loop around
                bpm_index = 0;
            }
            // RESET parameters
            bpm_chosen = bpms[bpm_index];
            // times are in tenths of a second, so 600 represents 60 secs or 1 min
            inhale_time = 600/(bpm_chosen)/(exhale_to_inhale+1);
            exhale_time = 600/(bpm_chosen)*(exhale_to_inhale)/(exhale_to_inhale+1);
            inhale_complete = 0;
            exhale_complete = 0;
            secs = 0;
            inhale = 1;
            snprintf(bpm_string, 17, "bpm: %d, etoi: %d", bpm_chosen, exhale_to_inhale);
            lcd_stringout(bpm_string);
        }
        if ((PINC & (1 << PC4)) == 0) { // button pressed -> switch exhale_to_inhale
            _delay_ms(5);
            while ((PINC & (1 << PC4)) == 0) {}
            if (exhale_to_inhale == 2) {
                exhale_to_inhale = 1;
            } else if (exhale_to_inhale == 1) {
                exhale_to_inhale = 2;
            }
            // RESET parameters
            bpm_chosen = bpms[bpm_index];
            // times are in tenths of a second, so 600 represents 60 secs or 1 min
            inhale_time = 600/(bpm_chosen)/(exhale_to_inhale+1);
            exhale_time = 600/(bpm_chosen)*(exhale_to_inhale)/(exhale_to_inhale+1);
            inhale_complete = 0;
            exhale_complete = 0;
            secs = 0;
            inhale = 1;
            snprintf(bpm_string, 17, "bpm: %d, etoi: %d", bpm_chosen, exhale_to_inhale);
            lcd_stringout(bpm_string);
        }
        if ((PINC & (1 << PC3)) == 0) { // button pressed -> increase tidal volume by 50 mL
            _delay_ms(5);
            while ((PINC & (1 << PC3)) == 0) {}
            vt_index++;
            if (vt_index >= 8) { // goes out of bounds -> loop around
                vt_index = 0;
            }
            // print the change in tidal volume
            char vt_string[8];
            snprintf(vt_string, 8, "vt: %d", vt_chosen);
            lcd_moveto(1, 7);
            lcd_stringout(vt_string);
        }
        if (inhale_complete) { // inhale is complete -> switch to exhale state
            lcd_moveto(1, 0);
            lcd_stringout("EXHALE");
            inhale_complete = 0;
            secs = 0;
            inhale = 0;
        } else if (exhale_complete) { // exhale is complete -> switch to inhale state
            lcd_moveto(1, 0);
            lcd_stringout("INHALE");
            exhale_complete = 0;
            secs = 0;
            inhale = 1;
        }
    }
    return 0;   /* never reached */
}

ISR(TIMER1_COMPA_vect) {
    secs++;

    if (inhale) { // inhaling
        if (secs >= inhale_time) {
            inhale_complete = 1;
        }
    } else { // must be exhaling
        if (secs >= exhale_time) {
            exhale_complete = 1;
        }
    }
}

