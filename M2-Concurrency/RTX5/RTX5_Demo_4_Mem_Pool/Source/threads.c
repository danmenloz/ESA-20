#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

#define NUM_Q_ENTRIES 8

#define MAX_NUM_LED_STEPS 16
#define LED_PATTERN_POOL_COUNT 2

typedef struct {
	uint8_t R:1;
	uint8_t G:1;
	uint8_t B:1;
} RGB_t;

typedef struct {
	RGB_t Step[MAX_NUM_LED_STEPS];
	uint8_t NumSteps;
} LED_pattern_t;

osThreadId_t tid_RGB;
osThreadId_t tid_Read_Switches;
osMemoryPoolId_t pattern_pool_id;
osMessageQueueId_t switch_msgq_id;

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence

void Init_My_RTOS_Objects(void) {
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread

	switch_msgq_id = osMessageQueueNew(NUM_Q_ENTRIES, sizeof(LED_pattern_t), NULL);
	pattern_pool_id = osMemoryPoolNew(LED_PATTERN_POOL_COUNT, sizeof(LED_pattern_t), NULL);
}

void Thread_RGB(void * arg) {
	osStatus_t result;
	LED_pattern_t pattern;
	uint8_t cur_step;
	
	while (1) {
		result = osMessageQueueGet(switch_msgq_id, 
			&pattern, NULL, osWaitForever);
		if (result==osOK) {
			for (cur_step = 0; cur_step < pattern.NumSteps; cur_step++) {
				Control_RGB_LEDs(pattern.Step[cur_step].R, pattern.Step[cur_step].G, 
					pattern.Step[cur_step].B);
				osDelay(g_RGB_delay);
			}	
			// Turn off LEDs
			Control_RGB_LEDs(0, 0, 0);
		}
	}
}

void Thread_Read_Switches(void * arg) {
	int count = 0;
	LED_pattern_t * pattern;
	int cur_step = 0;
	
	while (1) {
		osDelay(250);
		if (SWITCH_PRESSED(SW1_POS)) { 
			if (count == 0) { 	// when switch is first pressed, allocate an object
				pattern = (LED_pattern_t *) osMemoryPoolAlloc(pattern_pool_id, osWaitForever);
				if (!pattern) {				// out of memory blocks!
					Control_RGB_LEDs(1,0,0);
					while (1) {
						;
					}
				}
				cur_step = 0;
				pattern->NumSteps = 0;
			}
			count++;
			
			if (cur_step < MAX_NUM_LED_STEPS) {
				// load current step of pattern based on count value
				pattern->Step[cur_step].R = (count & 1)? 1 : 0;
				pattern->Step[cur_step].G = (count & 2)? 1 : 0;
				pattern->Step[cur_step].B = (count & 4)? 1 : 0;
				cur_step++;
			}
			Control_RGB_LEDs(0, 1, 0);
			osDelay(g_RGB_delay/5);
			Control_RGB_LEDs(0, 0, 0);
		} else {
			if (count > 0) {
				// Complete the message
				pattern->NumSteps = cur_step;
				// Send the message
				osMessageQueuePut(switch_msgq_id, pattern, 0, osWaitForever);
				// Free up the memory for pattern
				osMemoryPoolFree(pattern_pool_id, pattern);
				// Reset button hold-time counter
				count = 0;
			}
		}	
	}
}


