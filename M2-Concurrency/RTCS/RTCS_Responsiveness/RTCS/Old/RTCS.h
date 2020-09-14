#ifndef RTCS_H
#define RTCS_H

#include <stdint.h>
#include "timers.h"

#define RTCS_NUM_TASKS (10)

extern volatile uint32_t RTCS_Num_Ticks;

typedef struct {
	void (*Task)(void);
	uint16_t Period;
	uint16_t TicksToNextRelease;
	char RunRequestsPending;
} RTCS_TASK_ENTRY;

extern volatile RTCS_TASK_ENTRY RTCS_Task_Table[RTCS_NUM_TASKS];

int RTCS_Add_Task(void (*task)(void), uint8_t priority, uint16_t period);
void RTCS_Run_Scheduler(void);
void RTCS_Init(uint32_t freq);
void RTCS_Timer_Tick(void);
int RTCS_Trigger_Task(uint8_t priority, uint16_t accumulate);


#endif // RTCS_H
