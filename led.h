#ifndef __COLOR_H__
#define __COLOR_H__

#define F_CPU 8000000

#define LED_STRIP_PORT PORTB
#define LED_STRIP_DDR  DDRB
#define LED_STRIP_PIN  0
#define NUM_LEDS 60

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>



struct color{
	unsigned char red, green, blue;
}color;


void __attribute__((noinline)) led_strip_write(struct color* colors, unsigned int count);


//sets/gets green byte of passed in color
void setGreen(struct color* led,unsigned char val); //USES COLOR POINTER
unsigned char getGreen(struct color led);

//sets/gets red byte of passed in color
void setRed(struct color* led,unsigned char val);//USES COLOR POINTER
unsigned char getRed(struct color led);

//sets/gets blue byte of passed in color
void setBlue(struct color* led,unsigned char val);//USES COLOR POINTER
unsigned char getBlue(struct color led);

//Will change color by specified step value, won't go past [0,0,0] or [255,255,255]
//ORDER OF COLORS: R->RG->G->GB->B->RB->R
void stepColor(struct color* led, signed short val);//USES COLOR POINTER

//converts char to rgb
void pot2color(unsigned char ani, struct color* led);

//Moves color closer/farther to [0,0,0] or [255,255,255]
void brightenColor(struct color* reference,struct color* arr, unsigned char num);
void darkenColor(struct color* reference,struct color* arr, unsigned char num);

//Sets whole arr to single color c
void solidLEDS(struct color* arr, struct color c, unsigned char size);

//Offset LEDS
void offsetUpdate(struct color* offsetArr, struct color* reference, struct color* fSet, signed char count);
//makes strip into blocks of chosen 3 colors
void blockLEDS(struct color* arr, unsigned char size, struct color c1, struct color c2, struct color c3);
//The three pre-set patterns that can be displayed
void rotate(struct color* arr, unsigned char size);
void slide(struct color* arr, unsigned char size);
void pulse(struct color* arr,struct color c1, struct color c2,struct color c3,unsigned char size);
#endif
