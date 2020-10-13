#ifndef THREADS_H
#include "cmsis_os2.h"

extern osThreadId_t tid_RGB;
extern osThreadId_t tid_error;

extern osSemaphoreId_t RGB_sem;
extern osSemaphoreId_t error_sem;

	
void Thread_RGB(void * arg);
void Thread_Error(void * arg);

#define USE_COUNTING_SEM (1)
#define USE_ERROR_HANDLING (1)

#define THREADS_H
#endif // THREADS_H
