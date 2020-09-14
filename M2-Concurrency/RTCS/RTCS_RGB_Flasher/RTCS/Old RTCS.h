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
	char Enabled;
} RTCS_TASK_ENTRY;

extern volatile RTCS_TASK_ENTRY RTCS_Task_Table[RTCS_NUM_TASKS];

int RTCS_Add_Task(void (*task)(void), uint8_t priority, uint16_t period);
void RTCS_Run_Scheduler(void);
void RTCS_Init(uint32_t freq);
void RTCS_Timer_Tick(void);
void RTCS_Enable_Task(void (*task)(void), uint32_t enable);
void RTCS_Set_Task_Period(void (*task)(void), uint16_t period);
void RTCS_Request_Task_Run(void (*task)(void));

#endif // RTCS_H
