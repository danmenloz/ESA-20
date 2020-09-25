#ifndef RTCS_H
#define RTCS_H

#include <stdint.h>
#include "timers.h"

#define RTCS_NUM_TASKS (10)

#define DEBUG_RTCS_POS 	0		// Port B bit 0, J10 pin 2

extern volatile uint32_t RTCS_Num_Ticks;

typedef struct {
	void (*Task)(void);
	uint32_t Period;
	uint32_t TicksToNextRelease;
	uint32_t ReleasesPending;
	char Enabled;
} RTCS_TASK_ENTRY;

extern volatile RTCS_TASK_ENTRY RTCS_Task_Table[RTCS_NUM_TASKS];

int RTCS_Add_Task(void (*task)(void), uint32_t priority, uint32_t period);
void RTCS_Run_Scheduler(void);
void RTCS_Init(uint32_t freq);
void RTCS_Timer_Tick(void);
int RTCS_Enable_Task(uint32_t i, uint32_t enable);
int RTCS_Find_Task_Priority(void (*task)(void));
int RTCS_Release_Task(uint32_t i);


#endif // RTCS_H
