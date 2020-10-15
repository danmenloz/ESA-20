#ifndef THREADS_H
#define THREADS_H

#include "cmsis_os2.h"

extern osThreadId_t tid_RGB;
extern osThreadId_t tid_Flash;
extern osThreadId_t tid_Read_Switches;


void Init_My_RTOS_Objects(void);

void Thread_RGB(void * arg);
void Thread_Flash(void * arg);
void Thread_Read_Switches(void  * arg);

typedef struct {
	char letter;
	int value;
} MY_MSG_T;

#endif // THREADS_H
