#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_RGB;
osThreadId_t tid_Flash;
osThreadId_t tid_Read_Switches;

osMessageQueueId_t switch_msgq_id;
osEventFlagsId_t evflags_id;    // Use bit 0 (value of 1) for flash request                               

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence
uint32_t g_w_delay=FLASH_TIME; 	// delay for flashing
uint8_t g_flash_LED=0;



void Init_My_RTOS_Objects(void) {
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread

	switch_msgq_id = osMessageQueueNew(5, sizeof(MY_MSG_T), NULL);
	evflags_id = osEventFlagsNew(NULL); 
}

void Thread_RGB(void * arg) {
	osStatus_t result;
	MY_MSG_T dest_msg;
	while (1) {
		result = osMessageQueueGet(switch_msgq_id, 
			&dest_msg, NULL, osWaitForever);
		if (result==osOK) {
			while (dest_msg.value-- > 0) { // Do RGB 
				Control_RGB_LEDs(1, 0, 0);
				osDelay(g_RGB_delay);
				Control_RGB_LEDs(0, 1, 0);
				osDelay(g_RGB_delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(g_RGB_delay);
			}
			Control_RGB_LEDs(0, 0, 0);
		}
	}
}

#if 1	// Version described in slides
void Thread_Flash(void * arg) {
	int n;
	while (1) {
		osEventFlagsWait(evflags_id, 1, osFlagsWaitAny, osWaitForever);
		for (n=0; n<5; n++) {
			Control_RGB_LEDs(1, 0, 1);
			osDelay(g_w_delay);
			Control_RGB_LEDs(0, 0, 1);
			osDelay(g_w_delay);
		}
		Control_RGB_LEDs(0, 0, 0);
	}
}
#else // Alternate version: times out if no event within 3000 ticks (3000 ms)
void Thread_Flash(void * arg) {
	uint32_t result;
	int n;
	while (1) {
		result = osEventFlagsWait(evflags_id, 1, osFlagsWaitAny, 3000);
		if (result == osFlagsErrorTimeout) {
			Control_RGB_LEDs(1, 0, 0); // Error: timed out!
			osDelay(500);
		} else {
			for (n=0; n<5; n++) {
				Control_RGB_LEDs(1, 0, 1);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(g_w_delay);
			}
		}
		Control_RGB_LEDs(0, 0, 0);
	}
}
#endif


void Thread_Read_Switches(void  * arg) {
	int count = 0;
	MY_MSG_T msg;

	msg.value = 0;
	msg.letter = ' ';
	while (1) {
		osDelay(100);
		if (SWITCH_PRESSED(SW1_POS)) {
			osEventFlagsSet(evflags_id, 1);
		}	
		
		if (SWITCH_PRESSED(SW2_POS)) { 
			count++;
			Control_RGB_LEDs(0, 1, 0);
			osDelay(g_RGB_delay/30);
			Control_RGB_LEDs(0, 0, 0);
		} else { // send message on release
			if (count > 0) {
				msg.value = count;
				if (count > 10)
					msg.letter = '!';
				else 
					msg.letter = '^';
				osMessageQueuePut(switch_msgq_id, 
					&msg, NULL, osWaitForever);
				count = 0;
			}
		}
	}
}
