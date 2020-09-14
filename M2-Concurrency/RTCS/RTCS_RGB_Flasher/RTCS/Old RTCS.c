#include "RTCS.h"
#include <stdint.h>
#include "gpio_defs.h"

volatile uint32_t RTCS_Num_Ticks=0;
volatile RTCS_TASK_ENTRY RTCS_Task_Table[RTCS_NUM_TASKS];

int RTCS_Add_Task(void (*task)(void), uint8_t priority, uint16_t period) {
	if (priority < RTCS_NUM_TASKS) {
		RTCS_Task_Table[priority].Task = task;
		RTCS_Task_Table[priority].Period = period;
		RTCS_Task_Table[priority].TicksToNextRelease = period;
		RTCS_Task_Table[priority].RunRequestsPending = 1;
		RTCS_Task_Table[priority].Enabled = 1;
		return 1;
	} else {
		return 0;
	}
}

void RTCS_Init(uint32_t freq) {
	uint8_t i;

	for (i=0; i<RTCS_NUM_TASKS; i++) {
		RTCS_Task_Table[i].Task = (void *)0;
		RTCS_Task_Table[i].Period = 0;
		RTCS_Task_Table[i].TicksToNextRelease = 0;
		RTCS_Task_Table[i].RunRequestsPending = 0;
		RTCS_Task_Table[i].Enabled = 0;
	}
	
	Init_LPTMR(freq);
	Start_LPTMR();
}
	
void RTCS_Run_Scheduler(void){
	uint8_t i;
	while (1) {
		// start at top of table to find highest priority ready task
		for (i=0; i<RTCS_NUM_TASKS; i++) {
			if ((RTCS_Task_Table[i].Task != 0) && (RTCS_Task_Table[i].Enabled) &&
				(RTCS_Task_Table[i].RunRequestsPending > 0)) {
				RTCS_Task_Table[i].RunRequestsPending--;
				RTCS_Task_Table[i].Task();
				break; // exit from for loop, to start at top of table again
			}
		}
		PTB->PTOR = MASK(DEBUG_RTCS_IDLE_POS);

	}
}
void RTCS_Timer_Tick(void){
	uint8_t i;
	
	PTB->PSOR = MASK(DEBUG_RTCS_TICK_POS);
	RTCS_Num_Ticks++;
	
	for (i=0; i<RTCS_NUM_TASKS; i++) {
		if ((RTCS_Task_Table[i].Task != 0) && (RTCS_Task_Table[i].Enabled) && 
			(RTCS_Task_Table[i].TicksToNextRelease > 0)) {
			if (--RTCS_Task_Table[i].TicksToNextRelease == 0) { // decrement TicksToNextRelease, if 0 then time to run
				RTCS_Task_Table[i].RunRequestsPending++;
				RTCS_Task_Table[i].TicksToNextRelease = RTCS_Task_Table[i].Period;
			}
		}
	}

	PTB->PCOR = MASK(DEBUG_RTCS_TICK_POS);
}


void RTCS_Enable_Task(void (*task)(void), uint32_t enable) {
	uint8_t i;
	
	for (i=0; i<RTCS_NUM_TASKS; i++) {
		if (RTCS_Task_Table[i].Task == task) {
				RTCS_Task_Table[i].Enabled = enable;
				return;
		}
	}
}

void RTCS_Set_Task_Period(void (*task)(void), uint16_t period){
	uint8_t i;
	
	for (i=0; i<RTCS_NUM_TASKS; i++) {
		if (RTCS_Task_Table[i].Task == task) {
				RTCS_Task_Table[i].Period = period;
				return;
		}
	}
}

void RTCS_Request_Task_Run(void (*task)(void)) {
	uint8_t i;
	
	for (i=0; i<RTCS_NUM_TASKS; i++) {
		if (RTCS_Task_Table[i].Task == task) {
				RTCS_Task_Table[i].RunRequestsPending++;
				return;
		}
	}
}

	
