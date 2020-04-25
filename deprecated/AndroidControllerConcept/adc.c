#include <avr/io.h>

#include "adc.h"


void adc_init(unsigned char channel)
{
    // Initialize the ADC
    ADMUX &= 0;
    ADCSRA &= 0;
    channel &= 0x0f;
    ADMUX |= channel;
    ADMUX |= (1<<6);
    ADMUX |= (1<<5);
    ADCSRA |= 0x07;
    ADCSRA |= (1<<7);

}

unsigned char adc_sample()
{
    // Convert an analog input and return the 8-bit result
	ADCSRA |= (1<<6);
	while((ADCSRA & (1<<ADSC)) != 0)
	{}
    unsigned char result = ADCH;
	return result; //change that
}
