


#include <avr/io.h>
#include "led.h"
#include "timer.h"
#include "adc.h"

struct color LEDS[NUM_LEDS];

typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodAI = 50;
const unsigned long periodDisplay = 50;

task tasks[2];

enum AI_states {AI_start, AI_convert};
int Tick_AI(int state);

enum Display_States{Display_start, Display_go};
int Tick_Display(int state);

void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Set A to input
	DDRB = 0xFF; PORTB = 0x00; // Set B to output
	DDRC = 0xFF; PORTC = 0x00; // Set C to output
	
	initADC();
	
	unsigned char i = 0;
	tasks[i].state = AI_start;
	tasks[i].period = periodAI;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_AI;
	
	++i;
	tasks[i].state = Display_start;
	tasks[i].period = periodDisplay;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Display;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while(1)
    {
    }
}

int Tick_AI(int state){
	switch(state)
	{
		case AI_start:
			for(int i = 0; i < NUM_LEDS; ++i){
				setRed(LEDS+i, 255);
				setGreen(LEDS+i, 0);
				setBlue(LEDS+i, 0);
			}
			state = AI_convert;
			break;
		case AI_convert:
			for(int i = 0; i < NUM_LEDS; ++i){
				setRed(LEDS+i,255);
				setGreen(LEDS+i, 0);
				setBlue(LEDS+i, 0);
			}
			for(int i = 0; i < NUM_LEDS; ++i){
				ani2Digi(adc_get(0),LEDS+i);
			}
			
			break;
	}
	return state;
}
int Tick_Display(int state){
	switch(state)
	{
		case Display_start:
			state = Display_go;
			break;
		case Display_go:
			led_strip_write(LEDS, NUM_LEDS);
			break;
	}
	return state;
}