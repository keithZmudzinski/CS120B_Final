#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__



unsigned char getLeft(signed short mid, signed short LR){
	return(LR < mid - 20);
}
unsigned char getRight(signed short mid, signed short LR){
	return (LR > mid + 20);
}
unsigned char getUp(signed short mid, signed short UD){
	return (UD > mid + 20);
}
unsigned char getDown(signed short mid, signed short UD){
	return (UD < mid -20);
}




#endif