#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_RGB;
osThreadId_t tid_Read_Switches;

osSemaphoreId_t RGB_sem;

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence
uint8_t g_flash_LED=0;

void Thread_RGB(void * arg) {
	while (1) {
		osSemaphoreAcquire(RGB_sem, osWaitForever);
		Control_RGB_LEDs(1, 0, 0);
		osDelay(g_RGB_delay);
		Control_RGB_LEDs(0, 1, 0);
		osDelay(g_RGB_delay);
		Control_RGB_LEDs(0, 0, 1);
		osDelay(g_RGB_delay);
		Control_RGB_LEDs(0, 0, 0);
	}
}

void Thread_Read_Switches(void  * arg) {
	while (1) {
		osDelay(100);
		if (SWITCH_PRESSED(SW1_POS)) { // Do RGB Sequence
			osSemaphoreRelease(RGB_sem);
		}
		
		if (SWITCH_PRESSED(SW2_POS)) { // Fast
			g_RGB_delay = ON_TIME/5;
		}	else { // Slow
			g_RGB_delay = ON_TIME;
		}
	}
}
