#ifndef __ADC_H__
#define __ADC_H__


#include <avr/io.h>

void initADC(){
	ADMUX |= (1 << REFS0) | (1 << ADLAR);
	//REFS0: Use Vcc as reference voltage
	//ADLAR: Left shift ADC value, get 8-bit precision instead of 10
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) |(1 << ADEN);
	//ADPS[2:1]: Use prescalar of 64 (8MHz / 64 = 125kHz)
	//ADEN: Enable ADC
}
//Reads specified ch analog value, return digital
unsigned char adc_get(unsigned char ch){
	ch &= 0x07;  // Make sure ch at max is 7
	ADMUX = (ADMUX & 0xF8) | ch; // Assign bottom 3 ADMUX bits to ch
	
	ADCSRA |= (1 << ADSC);// Start single conversion
	unsigned short i = 0;
	while(ADCSRA & (1<<ADSC));//TO DO: ADD WHILE LOOP TO CATCH ERROR
	//busy wait until conversion done
	//When conversion is done, ADSC = 0
	//AND-ing 1 with ADSC until ADSC = 0
	
	
	return (ADCH);//Returns left shifted 8 bits when conversion is done
}

#endif
