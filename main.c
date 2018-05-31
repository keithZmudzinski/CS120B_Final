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

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodPC = 50;
const unsigned long periodPtt = 50;
const unsigned long periodDsp = 50;

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

enum Ptt_states {Ptt_start, Ptt_wait, Ptt_rotate, Ptt_next1, Ptt_slide, Ptt_next2, Ptt_pulse, Ptt_next3};
int Tick_Ptt(int state);

enum Dsp_states {Dsp_start, Dsp_mux};
int Tick_Dsp(int state);

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
unsigned char pickFlag = 0x01;
//Defines
//#define A7 (PINA & 0x80) >> 7
#define A6 !((PINA & 0x40) >> 6)

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Set A to input
	DDRB = 0xFF; PORTB = 0x00; // Set B to output
	DDRC = 0x00; PORTC = 0xFF; // Set C to input
	DDRD = 0xFF; PORTD = 0x00; // Set D to output
	
	initADC();
	
	unsigned char i = 0;
	tasks[i].state = PC_start;
	tasks[i].period = periodPC;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_PC;
	
	++i;
	tasks[i].state = Ptt_start;
	tasks[i].period = periodPtt;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Ptt;
		
	++i; 
	tasks[i].state = Dsp_start;
	tasks[i].period = periodDsp;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Dsp;
	
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while(1)
    {
    }
}
unsigned char tmpC0 = 0x00;
int Tick_PC(int state){
	static unsigned char i = 0x00;
	tmpC0 = !(PINC & 0x01);
	switch(state){//Transitions
		case PC_start://Initialize all leds in pick to red
			for(unsigned char j = 0; j < NUM_LEDS; ++j){
				setRed(pick + j, 255);
				setGreen(pick + j, 0);
				setBlue(pick + j, 0);
			}
			custColorStack.size = 0;
			state = PC_wait;
			i = 0;
			break;
		case PC_wait:
			//A7 = 1;//////////////////////////////////////
			if(!tmpC0){
				state = PC_wait;
			}
			else if(tmpC0 && pickFlag){//if A7 pushed and in pick mode
				state = PC_update;
				struct color custom = {255,0,0};
				pot2color(adc_get(0), &custom);//Get pot val, convert to color
				push(&custColorStack, custom);//Push to stack
				++i;
			}
			else{state = PC_wait;}
			break;
		case PC_update:
			for(unsigned char j = 0; j < NUM_LEDS; ++j){
				setRed(pick + j, 255);
				setGreen(pick + j, 0);
				setBlue(pick + j, 0);
			}
			if(tmpC0 && pickFlag){//holding button
				state = PC_update;
			}
			
			else if(!tmpC0 && i >= 3){//released button, and chosen 3 colors
				pickFlag = 0;
				i = 0;
				state = PC_wait;
			}
			else if(!tmpC0 && i < 3){ state = PC_wait;}//released button, not chosen 3 colors
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
			for(unsigned char k = 0; k < NUM_LEDS; ++k){
				setRed(pick+k,tmp.red);
				setGreen(pick+k, tmp.green);
				setBlue(pick+k, tmp.blue);
			}

			break;
		case PC_update:
			break;
		default:
			break;
	}
	//led_strip_write(pick, NUM_LEDS);
	return state;
}

int Tick_Ptt(int state){
	static unsigned char i = 0x00;
	//unsigned char A7 = (PINC & 0x01);
	static struct color c1,c2,c3;
	switch(state){
		case Ptt_start:
			i = 0;
			state = Ptt_wait;
			for(unsigned char j = 0; j < NUM_LEDS; ++j){
				setRed(pattern + j, 255);
				setGreen(pattern + j, 0);
				setBlue(pattern + j, 0);
			}
			
			break;
			
		case Ptt_wait:
			if(custColorStack.size < 3){
				state = Ptt_wait;
			}
			else if(custColorStack.size == 3){
				c1 = pop(&custColorStack);
				c2 = pop(&custColorStack);
				c3 = pop(&custColorStack);
				//initialize pattern to 3 blocks of colors
				for(unsigned char j = 0; j < NUM_LEDS; ++j){
					setRed(pattern + j, c1.red);
					setGreen(pattern + j, c1.green);
					setBlue(pattern + j, c1.blue);
				}
				state = Ptt_rotate;
			}
			break;
		case Ptt_rotate:
			if(!tmpC0){
				state = Ptt_rotate;
			}
			else if(tmpC0 && !pickFlag){
				state = Ptt_next1;
			}
			break;
		case Ptt_next1:
			if(tmpC0 && !pickFlag){state = Ptt_next1;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!tmpC0 && !pickFlag){
				i = 0;
				blockLEDS(pattern, NUM_LEDS, c1,c2,c3);
				state = Ptt_slide;
			}
			break;
		case Ptt_slide:
			if(!tmpC0){
				state = Ptt_slide;
			}
			else if(tmpC0 && !pickFlag){
				state = Ptt_next2;
			}
			break;
		case Ptt_next2:
			if(tmpC0 && !pickFlag){state = Ptt_next2;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!tmpC0 && !pickFlag){
				i = 0;
				state = Ptt_pulse;
			}
			break;
		case Ptt_pulse:
			if(!tmpC0){
				state = Ptt_pulse;
			}
			else if(tmpC0 && !pickFlag){
				state = Ptt_next3;
			}
			break;
		case Ptt_next3:
			if(tmpC0 && !pickFlag){state = Ptt_next3;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!tmpC0 && !pickFlag){
				i = 0;
				for(unsigned char j = 0; j < NUM_LEDS; ++j){
					setRed(pattern + j, c1.red);
					setGreen(pattern + j, c1.green);
					setBlue(pattern + j, c1.blue);
				}
				state = Ptt_rotate;
			}
			break;
		default:
			state = Ptt_start;
			break;
	}
	switch(state){
		case Ptt_start:
			break;
		case Ptt_wait:
			break;
		case Ptt_rotate:
			rotate(pattern, NUM_LEDS);
			break;
		case Ptt_next1:
			++i;
			break;
		case Ptt_slide:
			slide(pattern, NUM_LEDS);
			break;
		case Ptt_next2:
			++i;
			break;
		case Ptt_pulse:
			pulse(pattern, NUM_LEDS);
			break;
		case Ptt_next3:
			++i;
			break;
		default:
			break;
	}
	return state;
}

int Tick_Dsp(int state){//JUST DONE SO I CAN TEST 
	struct color arr[NUM_LEDS];
	for(unsigned char j = 0; j < NUM_LEDS; ++j){
		setRed(arr + j, 255);
		setGreen(arr + j, 0);
		setBlue(arr + j, 0);
	}
	if(pickFlag){led_strip_write(pick, NUM_LEDS);}
	else{led_strip_write(pattern,NUM_LEDS);}
	return state;
}