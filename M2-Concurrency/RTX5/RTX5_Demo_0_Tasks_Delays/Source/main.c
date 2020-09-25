/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
 /*
Demonstrate basic task concepts, OS delay

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
#include "debug.h"
 
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
	Init_Debug_Signals();
	
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread
  tid_Flash = osThreadNew(Thread_Flash, NULL, NULL);    // Create thread
  tid_Read_Switches = osThreadNew(Thread_Read_Switches, NULL, NULL);    // Create thread
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
