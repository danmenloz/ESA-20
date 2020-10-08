#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_RGB;
osThreadId_t tid_error;

osSemaphoreId_t RGB_sem;
osSemaphoreId_t error_sem;

// Private to file
uint32_t g_RGB_delay=ON_TIME; 	// delay for RGB sequence
uint8_t g_flash_LED=0;
uint32_t sem_error=0;

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

#if USE_ERROR_HANDLING
void Thread_Error(void * arg) {
	while (1) {
		osSemaphoreAcquire(error_sem, osWaitForever);
		while (1) { // Flash red LED forever now
			Control_RGB_LEDs(1, 0, 0);
			osDelay(50);
			Control_RGB_LEDs(0, 0, 0);
			osDelay(50);
		}
	}
}

void PORTD_IRQHandler(void) {  
	osStatus_t result;
	PTB->PSOR = MASK(DBG_1);
	// Read switches
	if ((PORTD->ISFR & MASK(SW1_POS))) {	
		if (SWITCH_PRESSED(SW1_POS)) { 
			result = osSemaphoreRelease(RGB_sem);	
			if (result != osOK) {
				result = osSemaphoreRelease(error_sem);
				// Shouldn't need to check this result too 
				// Exercise for students: Explain why.
			}
		}
	}
	// clear status flags 
	PORTD->ISFR = 0xffffffff;
	PTB->PCOR = MASK(DBG_1);
}

#else
void PORTD_IRQHandler(void) {  
	osStatus_t result;
	PTB->PSOR = MASK(DBG_1);
	// Read switches
	if ((PORTD->ISFR & MASK(SW1_POS))) {	
		if (SWITCH_PRESSED(SW1_POS)) { 
			result = osSemaphoreRelease(RGB_sem);	
		}
	}
	// clear status flags 
	PORTD->ISFR = 0xffffffff;
	PTB->PCOR = MASK(DBG_1);
}
#endif

