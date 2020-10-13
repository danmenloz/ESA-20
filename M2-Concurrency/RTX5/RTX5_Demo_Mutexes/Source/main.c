/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
 /*
	Demonstrate mutex

	Wiring Needed:
			Switch 1: SW1_POS PTD7	(J2-19)
			Switch 2: SW2_POS PTD6	(J2-17)
*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
 
 #ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif
 
#include "LEDs.h"
#include "gpio_defs.h"
#include "threads.h"
 

int main (void) {
   // System Initialization
  SystemCoreClockUpdate();
#ifdef RTE_Compiler_EventRecorder
  // Initialize and start Event Recorder
  EventRecorderInitialize(EventRecordError, 1U);
#endif
  // ...
	Init_RGB_LEDs();
	Init_Switches();
	
  osKernelInitialize();                 	// Initialize CMSIS-RTOS

#if USE_MUTEX
	LED_mutex = osMutexNew(NULL);		// Create mutex
#endif 
	
  tid_Red = osThreadNew(Thread_Control_RedLED, NULL, NULL);    // Create thread
  tid_Green = osThreadNew(Thread_Control_GreenLED, NULL, NULL);    // Create thread
  tid_Blue = osThreadNew(Thread_Control_BlueLED, NULL, NULL);    // Create thread

  osKernelStart();                      // Start thread execution
  for (;;) {}
}
