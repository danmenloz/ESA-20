/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
 
 #ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif
 
 #include "LEDs.h"
 #include "gpio_defs.h"
 
 #include "threads.h"
 
osThreadId_t tid_Blinky; // Thread id
 
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
#ifdef RTE_Compiler_EventRecorder
  // Initialize and start Event Recorder
  EventRecorderInitialize(EventRecordError, 1U);
#endif
  // ...
	Init_RGB_LEDs();
	
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  tid_Blinky = osThreadNew(Thread_Blinky, NULL, NULL);    // Create blinky thread
  osKernelStart();                      // Start thread execution
}
