#include <avr/io.h>
#include "led.h"
#include "timer.h"
#include "adc.h"
#include "stack.h"

//TO DO: ADD OFFSET AND ON OFF STATE, GET BUZZER INPUT


typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[5];

const unsigned char tasksNum = 5;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodPC = 50;
const unsigned long periodPtt = 50;
const unsigned long periodDsp = 50;
const unsigned long periodShade = 50;
const unsigned long periodTmp = 1000;

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

enum Dsp_states {Dsp_start, Dsp_pattern1, Dsp_temp1, Dsp_temp2, Dsp_pattern2};
int Tick_Dsp(int state);

enum Tmp_states {Tmp_start, Tmp_sample, Tmp_update};
int Tick_Tmp(int state);

enum Shd_states{Shd_start, Shd_wait1, Shd_wait2,Shd_off1, Shd_off2};
int Tick_Shd(int state);

//Global arrays
struct color pick[NUM_LEDS];
struct color pattern[NUM_LEDS];
struct color temp[NUM_LEDS];
struct color offset[NUM_LEDS];
struct color* LEDS;
struct color* fSet;
//Global Stacks
struct color custColorArr[10];
struct stack custColorStack = {custColorArr, 0};
struct color tempArr[10];
struct stack tempStack = {tempArr, 0};
//Global Variables
unsigned char dspFlag = 0x01; //1 means displaying potentiometer, 2 displaying temp, 3 displaying mic, 0 for transition purposes
unsigned char pickFlag = 0x01;
unsigned char C0 = 0x00;
unsigned char C1 = 0x00;
//Defines



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
	
	++i;
	tasks[i].state = Tmp_start;
	tasks[i].period = periodDsp;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Tmp;
	
	++i;
	tasks[i].state = Shd_start;
	tasks[i].period = periodShade;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_Shd;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	while(1)
	{
	}
}

int Tick_PC(int state){
	static unsigned char i = 0x00;
	C0 = !(PINC & 0x01);
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
			if(!C0){
				state = PC_wait;
			}
			else if(C0 && pickFlag && dspFlag == 0x01){//if A7 pushed and in pick mode
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
			if(C0 && pickFlag && dspFlag == 0x01){//holding button
				state = PC_update;
			}
		
			else if(!C0 && i >= 3 && dspFlag == 0x01){//released button, and chosen 3 colors
				pickFlag = 0;
				i = 0;
				state = PC_wait;
			}
			else if(!C0 && i < 3 && dspFlag == 0x01){ state = PC_wait;}//released button, not chosen 3 colors
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
	return state;
}

int Tick_Ptt(int state){
	static unsigned char i = 0x00;
	C0 = !(PINC & 0x01);
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
			if(!C0){
				state = Ptt_rotate;
			}
			else if(C0 && !pickFlag && dspFlag == 0x01){
				state = Ptt_next1;
			}
			break;
		case Ptt_next1:
			if(C0 && !pickFlag && dspFlag == 0x01){state = Ptt_next1;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!C0 && !pickFlag && dspFlag == 0x01){
				i = 0;
				blockLEDS(pattern, NUM_LEDS, c1,c2,c3);
				state = Ptt_slide;
			}
			break;
		case Ptt_slide:
			if(!C0){
				state = Ptt_slide;
			}
			else if(C0 && !pickFlag && dspFlag == 0x01){
				state = Ptt_next2;
			}
			break;
		case Ptt_next2:
			if(C0 && !pickFlag && dspFlag == 0x01){state = Ptt_next2;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!C0 && !pickFlag && dspFlag == 0x01){
				i = 0;
				state = Ptt_pulse;
			}
			break;
		case Ptt_pulse:
			if(!C0 && !pickFlag && dspFlag == 0x01){
				state = Ptt_pulse;
			}
			else if(C0 && !pickFlag && dspFlag == 0x01){
				state = Ptt_next3;
			}
			break;
		case Ptt_next3:
			if(C0 && !pickFlag && dspFlag == 0x01){state = Ptt_next3;}
			else if(i >= 20){
				i = 0;
				pickFlag = 1;
				state = Ptt_wait;
			}
			else if(!C0 && !pickFlag && dspFlag == 0x01){
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

int Tick_Tmp(int state){
	static unsigned char sample = 0x00;
	switch(state){
		case Tmp_start:;
			struct color c = {0,0,255};
			solidLEDS(temp, c, NUM_LEDS);
			state = Tmp_sample;
			break;
		case Tmp_sample:
			state = Tmp_update;
			break;
		case Tmp_update:
			state = Tmp_sample;
			break;
		default:
			state = Tmp_start;
			break;
	}
	switch(state){
		case Tmp_start:
			break;
		case Tmp_sample:
			sample = adc_get(1)/2;
			break;
		case Tmp_update:;
			//reset leds to blue before incrementing them
			struct color c = {0,0,255};
			solidLEDS(temp, c, NUM_LEDS);
			//convert adc val to out of 33(my range of degrees) X
			//Solid red = 37C(100F), Solid blue = 4C(40F)
			for(unsigned char j = 0; j < ((33*sample)/100); ++j){
				//step each led by 31, X times
				for(unsigned char k = 0; k < NUM_LEDS; ++k){
					stepColor(temp + k,15);
				}
			}
			break;
		default:
			break;
	}
	return state;
}

int Tick_Dsp(int state){
	C1 = !(PINC & 0x02);
	static unsigned char tmpD = 0x00;
	switch(state){//transitions
		case Dsp_start:
			dspFlag = 0x01;
			state = Dsp_pattern1;
			break;
		case Dsp_pattern1:
			if(!C1 || (C1 && C0)){
				state = Dsp_pattern1;
			}
			else if(C1 && !C0){
				state = Dsp_temp1;
			}
			break;
		case Dsp_temp1:
			if(C1){state = Dsp_temp1;}
			else if(!C1){state = Dsp_temp2;}
			break;
		case Dsp_temp2:
			if(!C1){state = Dsp_temp2;}
			else if(C1){state = Dsp_pattern2;}
			break;
		case Dsp_pattern2:
			if(C1){state = Dsp_pattern2;}
			else if(!C1){state = Dsp_pattern1;}
			break;
		default:
			state = Dsp_start;
	}
	switch(state){//state actions
		case Dsp_start:
			break;
		case Dsp_pattern1:
			dspFlag = 0x01;
			if(pickFlag){LEDS = pick;}
			else{LEDS = pattern;}
			tmpD = 0x01;
			break;
		case Dsp_temp1:
			dspFlag = 0x02;
			tmpD = 0x40;
			LEDS = temp;
			break;
		case Dsp_temp2:
			dspFlag = 0x02;
			tmpD = 0x02;
			LEDS = temp;
			break;
		case Dsp_pattern2:
			dspFlag = 0x00;//So pressing C0 does nothing transitioning into pattern1,
			tmpD = 0x01; //Same way pressing C1 while already holding C0 does nothing !!!CONSISTENCY!!!
			break;		
		default:
			break;
	}
	//led_strip_write(LEDS, NUM_LEDS); //JUST UNTIL I MAKE OFFSET TASK
	PORTD = tmpD;//This can stay I believe
	return state;
}

int Tick_Shd(int state){
	unsigned char joyStick = adc_get(2);
	static struct color off[NUM_LEDS];
	struct color c = {0,0,0};
	solidLEDS(off, c, NUM_LEDS);
	switch(state){
		case Shd_start://copy led values to offset array
			for(unsigned char i = 0x00; i < NUM_LEDS; ++i){
				setRed(offset + i, LEDS[i].red);
				setGreen(offset + i, LEDS[i].green);
				setBlue(offset + i, LEDS[i].blue);
			}
			state = Shd_wait1;
			fSet = offset;
			break;
		case Shd_wait1:
			if(joyStick >= UPVALUE){
				brightenColor(LEDS, offset, NUM_LEDS);
				state = Shd_wait2;
			}
			else if(joyStick >= DOWNVALUE){
				darkenColor(LEDS, offset, NUM_LEDS);
				state = Shd_wait2;
			}
			else if(joyStick >= LEFTVALUE){
				for(unsigned char i = 0x00;i < NUM_LEDS; ++i){
					setRed(offset + i, LEDS[i].red);
					setGreen(offset + i, LEDS[i].green);
					setBlue(offset + i, LEDS[i].blue);
				}
				state = Shd_wait2;
			}
			else if(joyStick >= RIGHTVALUE){
				fSet = off;
				state = Shd_off1;
			}
			else{
				state = Shd_wait1;
			}
			break;
		case Shd_wait2:
			if(joyStick){
				state = Shd_wait2;
			}
			else{
				state = Shd_wait1;
			}
			break;
		case Shd_off1:
			if(joyStick){
				state = Shd_off1;
			}
			else{
				state = Shd_off2;
			}
			break;
		case Shd_off2:
			if(joyStick){
				state = Shd_wait1;
				fSet = offset;
			}
			else{
				state = Shd_off2;
			}
			break;
		default:
			state = Shd_start;
			break;
		
	}
	led_strip_write(fSet,NUM_LEDS);
	return state;
}
