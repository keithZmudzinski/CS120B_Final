#ifndef __STACK_H__
#define __STACK_H__

#include "led.h"

struct stack {
	struct color* top;
	unsigned char size;
};

void push(struct stack s, struct color val){
	if(s.size == 0){
		*(s.top) = val;
		++s.size;
		return;
	}
	++s.size;
	++s.top;
	*(s.top) = val;
}
struct color pop(struct stack s){
	if(s.size >0){
		struct color val = *(s.top);
		if(s.size > 1){
			--s.top;
		}
		--s.size;
		return val;
	}
	struct color x = {1,0,0};
	return  x;//Can't actually get this due to stepping
}
unsigned char empty(struct stack s){
	if(s.size){return 0x01;}
	return 0x00;
}

#endif
	