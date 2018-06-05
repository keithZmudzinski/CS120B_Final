#include "led.h"


unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
	//   Set bit to 1           Set bit to 0
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

void __attribute__((noinline)) led_strip_write(struct color* colors, unsigned int count)
{
	// Set the pin to be an output driving low.
	LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
	LED_STRIP_DDR |= (1<<LED_STRIP_PIN);

	cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
	while(count--)
	{
		// Send a color to the LED strip.
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		asm volatile(
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0\n"
		"rcall send_led_strip_byte%=\n"  // Send red component.
		"ld __tmp_reg__, -%a0\n"
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"rcall send_led_strip_byte%=\n"  // Send blue component.
		"rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

		// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
		"send_led_strip_byte%=:\n"
		"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
		"ret\n"

		// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
		// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
		// but this function always takes the same time (2 us).
		"send_led_strip_bit%=:\n"
		#if F_CPU == 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif
		"sbi %2, %3\n"                           // Drive the line high.

		#if F_CPU != 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif

		#if F_CPU == 16000000
		"nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU != 8000000
		#error "Unsupported F_CPU"
		#endif

		"brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

		#if F_CPU == 8000000
		"nop\n" "nop\n"
		#elif F_CPU == 16000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n"
		#endif

		"brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

		"ret\n"
		"led_strip_asm_end%=: "
		: "=b" (colors)
		: "0" (colors),         // %a0 points to the next color to display
		"I" (_SFR_IO_ADDR(LED_STRIP_PORT)),   // %2 is the port register (e.g. PORTC)
		"I" (LED_STRIP_PIN)     // %3 is the pin number (0-8)
		);

		// Uncomment the line below to temporarily enable interrupts between each color.
		//sei(); asm volatile("nop\n"); cli();
	}
	sei();          // Re-enable interrupts now that we are done.
	_delay_us(80);  // Send the reset signal.
}


void setGreen(struct color* led, unsigned char val){
	led->green  = val;
}
unsigned char getGreen(struct color led){
	return led.green;
}

void setRed(struct color* led,unsigned char val){
	led->red = val;
}
unsigned char getRed(struct color led){
	return led.red;
}

void setBlue(struct color* led,unsigned char val){
	led->blue = val;
}
unsigned char getBlue(struct color led){
	return led.blue;
}



void stepColor(struct color* led, signed short val){
	unsigned char red = getRed(*led);
	unsigned char green = getGreen(*led);
	unsigned char blue = getBlue(*led);
	
	if(red == 255 && green == 0 && blue == 0){//R
		green = ((green + val) <= 255)? (green + val) : 255;
		setGreen(led, green);
	}
	else if(red == 255 && green != 255 && blue == 0){//R->RG
		green = ((green + val) <= 255)? (green + val) : 255;
		setGreen(led, green);
	}
	else if(red == 255 && green == 255 && blue == 0){//RG
		red = ((red - val) >= 0)? (red - val) : 0;
		setRed(led,red);
	}
	else if(red != 0 && green == 255 && blue == 0){//RG->G
		red = ((red - val) >= 0)? (red - val) : 0;
		setRed(led,red);
	}
	else if(red == 0 && green == 255 && blue == 0){//G
		blue = ((blue + val) <= 255)? (blue + val) : 255;
		setBlue(led,blue);
	}
	else if(red == 0 && green == 255 && blue != 255){//G->GB
		blue = ((blue + val) <= 255)? (blue + val) : 255;	
		setBlue(led,blue);	
	}
	else if(red == 0 && green == 255 && blue == 255){//GB
		green = ((green - val) >= 0)? (green - val) : 0;
		setGreen(led, green);
	}
	else if(red == 0 && green != 0 && blue == 255){//GB->B
		green = ((green - val) >= 0)? (green - val) : 0;
		setGreen(led, green);
	}
	else if(red == 0 && green == 0 && blue == 255){//B
		red = ((red + val) <= 255)? (red + val) : 255;
		setRed(led,red);
	}
	else if(red != 255 && green == 0 && blue == 255){//B->RB
		red = ((red + val) <= 255)? (red + val) : 255;
		setRed(led,red);
	}
	else if(red == 255 && green == 0 && blue == 255){//RB
		blue = ((blue - val) >= 0)? blue - val : 0;
		setBlue(led,blue);
	}
	else if(red == 255 && green == 0 && blue != 0){//RB->R
		blue = ((blue - val) >= 0)? blue - val : 0;
		setBlue(led,blue);
	}
}

void pot2color(unsigned char ani, struct color* led){
	unsigned char i = 0;
	for(i = 0; i < ani; ++i){
		stepColor(led, 6);
	}
}

void brightenColor(struct color* reference, struct color* arr, unsigned char num){
	for(unsigned char i = 0x00; i < num; ++i){
		if(reference[i].red >= 245){
			if(arr[i].red < 255){
				setRed(arr + i, arr[i].red+1);
			}
		}
		else{
			setRed(arr + i, arr[i].red + (double)(255-reference[i].red)/10);
		}
		if(reference[i].green >= 245){
			if(arr[i].green < 255){
				setGreen(arr + i, arr[i].green+1);
			}
		}
		else{
			setGreen(arr + i, arr[i].green + (double)(255-reference[i].green)/10);
		}
		if(reference[i].blue >= 245){
			if(arr[i].blue < 255){
				setBlue(arr + i, arr[i].blue+1);
			}
		}
		else{
			setBlue(arr + i, arr[i].blue + (double)(255-reference[i].blue)/10);
		}
	}
}
void darkenColor(struct color* reference,struct color* arr, unsigned char num){
	for(unsigned char i = 0x00; i < num; ++i){
		if(reference[i].red <= 10){
			if(arr[i].red > 0){
				setRed(arr + i, arr[i].red-1);
			}
		}
		else{
			setRed(arr + i, arr[i].red - reference[i].red/10);
		}
		if(reference[i].green <= 10){
			if(arr[i].green > 0){
				setGreen(arr + i, arr[i].green-1);
			}
		}
		else{
			setGreen(arr + i, arr[i].green - reference[i].green/10);
		}
		if(reference[i].blue <= 10){
			if(arr[i].blue > 0){
				setBlue(arr + i, arr[i].blue-1);
			}
		}
		else{
			setBlue(arr + i, arr[i].green - reference[i].blue/10);
		}
	}
}

void solidLEDS(struct color* arr, struct color c, unsigned char size){
	for(unsigned char i = 0; i < size; ++i){
		setRed(arr + i, c.red);
		setGreen(arr + i, c.green);
		setBlue(arr + i, c.blue);
	}
}

void blockLEDS(struct color* arr, unsigned char size, struct color c1, struct color c2, struct color c3){
	for(unsigned char j = 0x00; j <size; ++j){
		if(j < size/3){
			setRed(arr + j, c1.red);
			setGreen(arr + j, c1.green);
			setBlue(arr + j, c1.blue);
		}
		else if(j < 2*(size/3)){
			setRed(arr + j, c2.red);
			setGreen(arr + j, c2.green);
			setBlue(arr + j, c2.blue);
		}
		else if(j < size){
			setRed(arr + j, c3.red);
			setGreen(arr + j, c3.green);
			setBlue(arr + j, c3.blue);
		}
	}
}


void rotate(struct color* arr, unsigned char size){
	for(unsigned char i = 0; i < size; ++i){
		stepColor(arr + i, 4);
	}
}

void slide(struct color* arr, unsigned char size){
	struct color end = arr[size-1];
	for(unsigned char i = size-1; i > 0; --i){
		arr[i].red = arr[i-1].red;
		arr[i].green = arr[i-1].green;
		arr[i].blue = arr[i-1].blue;
	}
	arr[0].red = end.red;
	arr[0].blue = end.blue;
	arr[0].green = end.green;
}

void pulse(struct color* arr, unsigned char size){
	//JUST TEMPORARY, MUST CHANGE TO ACTUAL FUNCTION
	for(unsigned char i = 0; i < size; ++i){
		setRed(arr + i, 255);
		setGreen(arr + i, 0);
		setBlue(arr + i, 0);
	}
}



