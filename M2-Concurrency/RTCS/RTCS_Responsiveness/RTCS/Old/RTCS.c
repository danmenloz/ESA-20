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
		RTCS_Task_Table[priority].RunRequestsPending = 0;
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
	}
	
	Init_LPTMR(freq);
	Start_LPTMR();
}
	
void RTCS_Run_Scheduler(void){
	uint8_t i;
	while (1) {
		// start at top of table to find highest priority ready task
		for (i=0; i<RTCS_NUM_TASKS; i++) {
			if ((RTCS_Task_Table[i].Task != 0) && (RTCS_Task_Table[i].RunRequestsPending > 0)) {
				RTCS_Task_Table[i].RunRequestsPending--;
				RTCS_Task_Table[i].Task();
				break; // exit from for loop, to start at top of table again
			}
		}
	}
}
void RTCS_Timer_Tick(void){
	uint8_t i;
	
	PTB->PSOR = MASK(DEBUG_RTCS_POS);
	RTCS_Num_Ticks++;
	
	for (i=0; i<RTCS_NUM_TASKS; i++) {
		if ((RTCS_Task_Table[i].Task != 0) && (RTCS_Task_Table[i].TicksToNextRelease > 0)) {
			if (--RTCS_Task_Table[i].TicksToNextRelease == 0) { // decrement TicksToNextRelease, if 0 then time to run
				RTCS_Task_Table[i].RunRequestsPending++;
				RTCS_Task_Table[i].TicksToNextRelease = RTCS_Task_Table[i].Period;
			}
		}
	}
	PTB->PCOR = MASK(DEBUG_RTCS_POS);
}

int RTCS_Trigger_Task(uint8_t priority, uint16_t allow_pending_requests) { 
	if ((priority < RTCS_NUM_TASKS) && (RTCS_Task_Table[priority].Task != 0)) {
		if (allow_pending_requests)
			RTCS_Task_Table[priority].RunRequestsPending++;
		else
			RTCS_Task_Table[priority].RunRequestsPending = 1;
		return 1;
	} else {
		return 0;
	}
}

