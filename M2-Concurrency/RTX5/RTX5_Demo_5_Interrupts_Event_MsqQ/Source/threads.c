#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_RGB;
osThreadId_t tid_Flash;

osMessageQueueId_t switch_msgq_id;
osEventFlagsId_t evflags_id;    // Use bit 0 (value of 1) for flash request                               

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence
uint32_t g_w_delay=FLASH_TIME; 	// delay for flashing

void Init_My_RTOS_Objects(void) {
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread

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
			while (dest_msg.value-- > 0) { // Do sequence 
				if (dest_msg.letter == 'P') { // RGB
					Control_RGB_LEDs(1, 0, 0);
					osDelay(g_RGB_delay);
					Control_RGB_LEDs(0, 1, 0);
					osDelay(g_RGB_delay);
					Control_RGB_LEDs(0, 0, 1);
					osDelay(g_RGB_delay);
				} else {											// BGR
					Control_RGB_LEDs(0, 0, 1);
					osDelay(g_RGB_delay);
					Control_RGB_LEDs(0, 1, 0);
					osDelay(g_RGB_delay);
					Control_RGB_LEDs(1, 0, 0);
					osDelay(g_RGB_delay);
				}
				Control_RGB_LEDs(0, 0, 0);
			}
		}
	}
}

void Thread_Flash(void * arg) {
	int n;
	uint32_t result;
	while (1) {
		result = osEventFlagsWait(evflags_id, PRESSED | RELEASED, 
		osFlagsWaitAny, osWaitForever);
		
		if (result & PRESSED){
			for (n=0; n<3; n++) {
				Control_RGB_LEDs(1, 0, 1);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 0);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(g_w_delay);
			}
		} else if (result & RELEASED) { 
			for (n=0; n<3; n++) {
				Control_RGB_LEDs(1, 1, 0);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 0);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 1, 0);
				osDelay(g_w_delay);
			}
		}
		Control_RGB_LEDs(0, 0, 0);
	}
}

void PORTD_IRQHandler(void) {  
	static MY_MSG_T msg;
	osStatus_t result;
	
	PTB->PSOR = MASK(DBG_1);
	// Read switches
	if ((PORTD->ISFR & MASK(SW1_POS))) {	
		if (SWITCH_PRESSED(SW1_POS)) { 
			osEventFlagsSet(evflags_id, PRESSED); 
		} else {
			osEventFlagsSet(evflags_id, RELEASED); 
		}
	}
	if ((PORTD->ISFR & MASK(SW2_POS))) {	
		if (SWITCH_PRESSED(SW2_POS)) {
				msg.value = 1;
				msg.letter = 'P'; // pressed
				result = osMessageQueuePut(switch_msgq_id, 
					&msg, NULL, 0);
				if (result != osOK) {
					Control_RGB_LEDs(1,0,0);
				}
		} else { // released
				msg.value = 2;
				msg.letter = 'R'; // released
				result = osMessageQueuePut(switch_msgq_id, 
					&msg, NULL, 0);
				if (result != osOK) {
					Control_RGB_LEDs(1,0,0);
				}
		}
	}
	// clear status flags 
	PORTD->ISFR = 0xffffffff;
	PTB->PCOR = MASK(DBG_1);
}

