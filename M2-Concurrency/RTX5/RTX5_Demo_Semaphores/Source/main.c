/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
 /*
	Demonstrate counting and binary semaphores

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



void Initialize_Interrupts(void) {
	/* Configure PORT peripheral. Select GPIO and enable pull-up 
	resistors and interrupts on all edges for pins connected to switches */
	PORTD->PCR[SW1_POS] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | 
		PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);
	PORTD->PCR[SW2_POS] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | 
		PORT_PCR_PE_MASK | PORT_PCR_IRQC(11);

	/* Configure NVIC */
	NVIC_SetPriority(PORTD_IRQn, 128);
	NVIC_ClearPendingIRQ(PORTD_IRQn); 
	NVIC_EnableIRQ(PORTD_IRQn);
	
	/* Configure PRIMASK */
	__enable_irq();
}
 
int main (void) {
   // System Initialization
  SystemCoreClockUpdate();
	Init_RGB_LEDs();
	Init_Switches();
	
  osKernelInitialize();                 	// Initialize CMSIS-RTOS
  tid_RGB = osThreadNew(Thread_RGB, NULL, NULL);    // Create thread

	#if USE_COUNTING_SEM
	RGB_sem = osSemaphoreNew(5, 0, NULL);		// Create counting semaphore
#else 
	RGB_sem = osSemaphoreNew(1, 0, NULL);		// Create binary semaphore
#endif 

#if USE_ERROR_HANDLING
	error_sem = osSemaphoreNew(100, 0, NULL);
  tid_error = osThreadNew(Thread_Error, NULL, NULL);    // Create thread
#endif
	
	Initialize_Interrupts();

  osKernelStart();                      // Start thread execution
  for (;;) {}
}
