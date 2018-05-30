#ifndef __STACK_H__
#define __STACK_H__

struct stack {
	unsigned char* top;
	unsigned char size;
};

void push(struct stack s, unsigned char val){
	if(s.size == 0){
		*(s.top) = val;
		++s.size;
		return;
	}
	++s.size;
	++s.top;
	*(s.top) = val;
}
unsigned short pop(struct stack s){
	if(s.size >0){
		unsigned char val = *(s.top);
		if(s.size > 1){
			--s.top;
		}
		--s.size;
		return val;
	}
	return 404; //Value too large for char, if get this I know popped too many times
}
unsigned char empty(struct stack s){
	if(s.size){return 0x01;}
	return 0x00;
}

#endif
	