#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"
#include "threads.h"

osThreadId_t tid_Flash;
osThreadId_t tid_Read_Switches;
osEventFlagsId_t evflags_id;    // Use bit 0 for press, bit 1 for release

// Private to file
uint32_t g_w_delay=FLASH_TIME; 	// delay for flashing
uint8_t g_flash_LED=0;

void Init_My_RTOS_Objects(void) {
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread
	evflags_id = osEventFlagsNew(NULL); 
}

void Thread_Flash(void * arg) {
	int n;
	uint32_t result;
	while (1) {
		result = osEventFlagsWait(evflags_id, 
		PRESSED | RELEASED, osFlagsWaitAny, osWaitForever);
		if (result & PRESSED){
			for (n=0; n<5; n++) {
				Control_RGB_LEDs(1, 0, 1);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(g_w_delay);
			}
		} 
		if (result & RELEASED) { 
			for (n=0; n<5; n++) {
				Control_RGB_LEDs(1, 1, 0);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 1, 0);
				osDelay(g_w_delay);
			}
		}
		Control_RGB_LEDs(0, 0, 0);
	}
}

void PORTD_IRQHandler(void) {  
	PTB->PSOR = MASK(DBG_1);
	// Read switches
	if ((PORTD->ISFR & MASK(SW1_POS))) {	
		if (SWITCH_PRESSED(SW1_POS)) { 
			osEventFlagsSet(evflags_id, PRESSED); 
		} else {
			osEventFlagsSet(evflags_id, RELEASED); 
		}
	}
	// clear status flags 
	PORTD->ISFR = 0xffffffff;
	PTB->PCOR = MASK(DBG_1);
}
