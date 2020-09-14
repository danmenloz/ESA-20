#ifndef THREADS_H
#define THREADS_H
#include "cmsis_os2.h"

extern osMutexId_t LED_mutex;

#define USE_MUTEX		1

extern osThreadId_t tid_Red, tid_Green, tid_Blue;
extern osMutexId_t LED_mutex;

extern void Thread_Control_RedLED (void * arg);
extern void Thread_Control_GreenLED (void * arg);
extern void Thread_Control_BlueLED (void * arg);

#endif // THREADS_H
