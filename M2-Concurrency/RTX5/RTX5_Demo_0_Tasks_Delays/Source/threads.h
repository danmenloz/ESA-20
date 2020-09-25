#ifndef THREADS_H

extern osThreadId_t tid_RGB;
extern osThreadId_t tid_Flash;
extern osThreadId_t tid_Read_Switches;


void Thread_RGB(void * arg);
void Thread_Flash(void * arg);
void Thread_Read_Switches(void  * arg);


#define THREADS_H
#endif // THREADS_H
