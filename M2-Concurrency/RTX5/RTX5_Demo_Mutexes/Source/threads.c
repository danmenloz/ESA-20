#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"
#include "gpio_defs.h"

#include "threads.h"

osThreadId_t tid_Red, tid_Green, tid_Blue;

osMutexId_t LED_mutex;

uint32_t g_RGB_delay=1000; 	// delay for RGB sequence

void Thread_Control_RedLED (void * arg) {
  for (;;) {
		osDelay(g_RGB_delay);
#if USE_MUTEX
		// Turning on an LED, so need to take mutex first
		osMutexAcquire(LED_mutex, osWaitForever);
#endif
    FPTB->PCOR = led_mask[LED_RED];
		osDelay(g_RGB_delay);
    FPTB->PSOR = led_mask[LED_RED];
#if USE_MUTEX
		// Just turned off an LED, so can release the mutex 
		osMutexRelease(LED_mutex);
#endif
  }
}

void Thread_Control_BlueLED (void * arg) {
	osDelay(g_RGB_delay/3);
  for (;;) {
		osDelay(g_RGB_delay);
#if USE_MUTEX
		// Turning on an LED, so need to take mutex first
		osMutexAcquire(LED_mutex, osWaitForever);
#endif
    FPTD->PCOR = led_mask[LED_BLUE];
		osDelay(g_RGB_delay);
    FPTD->PSOR = led_mask[LED_BLUE];
#if USE_MUTEX
		// Just turned off an LED, so can release the mutex 
		osMutexRelease(LED_mutex);
#endif
		}
}

void Thread_Control_GreenLED (void* arg) {
	osDelay(2*g_RGB_delay/3);
  for (;;) {
		osDelay(g_RGB_delay);
#if USE_MUTEX
		// Turning on an LED, so need to take mutex first
		osMutexAcquire(LED_mutex, osWaitForever);
#endif
    FPTB->PCOR = led_mask[LED_GREEN];
		osDelay(g_RGB_delay);
    FPTB->PSOR = led_mask[LED_GREEN];
#if USE_MUTEX
		// Just turned off an LED, so can release the mutex 
		osMutexRelease(LED_mutex);
#endif
  }
}
