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
	
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	Init_My_RTOS_Objects();
	Initialize_Interrupts();

  osKernelStart();                      // Start thread execution
}
