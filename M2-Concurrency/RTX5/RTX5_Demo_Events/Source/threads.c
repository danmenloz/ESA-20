#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

#define DONT_USE_EVENTS (0)

osThreadId_t tid_Flash;
osThreadId_t tid_Read_Switches;

// Private to file
uint32_t g_w_delay=FLASH_TIME; 	// delay for flashing
uint8_t g_flash_LED=0;

#if DONT_USE_EVENTS
osEventFlagsId_t evflags_id;    // Use bit 0 (value of 1) for flash request                               

void Init_My_RTOS_Objects(void) {
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread
}

void Thread_Flash(void * arg) {
	int n;
	while (1) {
		if (g_flash_LED > 0) {
			g_flash_LED = 0;
			for (n=0; n<5; n++) {
				Control_RGB_LEDs(1, 0, 1);
				osDelay(g_w_delay);
				Control_RGB_LEDs(0, 0, 1);
				osDelay(g_w_delay);
			}
			Control_RGB_LEDs(0, 0, 0);
		} else {
			osDelay(1);
		}
	}
}

void Thread_Read_Switches(void  * arg) {
	while (1) {
		osDelay(10);
		if (SWITCH_PRESSED(SW1_POS)) {
			g_flash_LED = 1;
		}		
	}
}
#else
osEventFlagsId_t evflags_id;    // Use bit 0 (value of 1) for flash request                               

void Init_My_RTOS_Objects(void) {
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread
	evflags_id = osEventFlagsNew(NULL); 
}

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

void Thread_Read_Switches(void  * arg) {
	while (1) {
		osDelay(200);
		if (SWITCH_PRESSED(SW1_POS)) {
			osEventFlagsSet(evflags_id, 1);
		}		
	}
}
#endif
