#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_RGB;
osThreadId_t tid_Read_Switches;

osMessageQueueId_t switch_msgq_id;

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence

void Init_My_RTOS_Objects(void) {
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread

	switch_msgq_id = osMessageQueueNew(5, sizeof(MY_MSG_T), NULL);
}

void Thread_RGB(void * arg) {
	osStatus_t result;
	MY_MSG_T dest_msg;
	uint32_t delay;
	
	while (1) {
		result = osMessageQueueGet(switch_msgq_id, 
			&dest_msg, NULL, osWaitForever);
		if (result==osOK) {
			if (dest_msg.letter == 'L')
				delay = g_RGB_delay/5;
			else
				delay = g_RGB_delay;
			while (dest_msg.value-- > 0) { // Do RGB 
				Control_RGB_LEDs(1, 0, 0);
				osDelay(delay);
				Control_RGB_LEDs(0, 1, 0);
				osDelay(delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(delay);
			}
			Control_RGB_LEDs(0, 0, 0);
		}
	}
}

void Thread_Read_Switches(void  * arg) {
	int count = 0;
	MY_MSG_T msg;

	msg.value = 0;
	msg.letter = ' ';
	while (1) {
		osDelay(100);
		if (SWITCH_PRESSED(SW2_POS)) { 
			count++;
			Control_RGB_LEDs(0, 1, 0);
			osDelay(g_RGB_delay/30);
			Control_RGB_LEDs(0, 0, 0);
		} else { // send message on release
			if (count > 0) {
				msg.value = count;
				if (count > 10)
					msg.letter = 'L';
				else 
					msg.letter = 'S';
				osMessageQueuePut(switch_msgq_id, 
					&msg, NULL, osWaitForever);
				count = 0;
			}
		}
	}
}
