#include <avr/io.h>
#include "led.h"
#include "timer.h"
#include "adc.h"
#include "stack.h"




typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[1];

const unsigned char tasksNum = 1;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodPC = 50;

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

enum PC_states {PC_start, PC_wait, PC_update};
int Tick_PC(int state);

//Global arrays
struct color pick[NUM_LEDS];
struct color pattern[NUM_LEDS];
struct color temp[NUM_LEDS];
struct color mic[NUM_LEDS];
struct color LEDS[NUM_LEDS];
//Global Stacks
struct color custColorArr[10];
struct stack custColorStack = {custColorArr, 0};
struct color tempArr[10];
struct stack tempStack = {tempArr, 0};
//Global Variables
unsigned char pickFlag = 0x00;
//Defines
#define A7 (PINA >> 7)
#define A6 ((PINA & 0x40) >> 6)

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Set A to input
	DDRB = 0xFF; PORTB = 0x00; // Set B to output
	DDRC = 0x00; PORTC = 0x00; // Set C to input
	DDRD = 0xFF; PORTD = 0x00; // Set D to output
	
	initADC();
	
	unsigned char i = 0;
	tasks[i].state = PC_start;
	tasks[i].period = periodPC;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_PC;
	
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while(1)
    {
    }
}

int Tick_PC(int state){
	static unsigned char i = 0x00;
	switch(state){//Transitions
		case PC_start://Initialize all leds in pick to red
			for(unsigned char i = 0; i < NUM_LEDS; ++i){
				setRed(pick + i, 255);
				setGreen(pick + i, 0);
				setBlue(pick + i, 0);
			}
			state = PC_wait;
			break;
		case PC_wait:
			if(!A7){state = PC_wait;}
			else if(A7 && pickFlag){//if A7 pushed and in pick mode
				state = PC_update;
				struct color custom = {0,0,0};
				pot2color(adc_get(0), &custom);//Get pot val, convert to color
				push(custColorStack, custom);//Push to stack
				++i;
			}
			else{state = PC_wait;}
			break;
		case PC_update:
			if(A7 && pickFlag){//holding button
				state = PC_update;
			}
			else if(!A7 && i >= 3){//released button, and chosen 3 colors
				pickFlag = 0;
				i = 0;
				state = PC_wait;
			}
			else{ state = PC_wait;}//released button, not chosen 3 colors
			break;
		default:
			state = PC_start;
	}
	switch(state){//State actions
		case PC_start:
			break;
		case PC_wait://update pick[] with current pot value
			;struct color tmp = {255,0,0};
			pot2color(adc_get(0),&tmp);
			for(unsigned char i = 0; i < NUM_LEDS; ++i){
				setRed(pick+i,tmp.red);
				setGreen(pick+i, tmp.green);
				setBlue(pick+i, tmp.blue);
			}
			break;
		case PC_update:
			break;
		default:
			break;
	}
	return state;
}