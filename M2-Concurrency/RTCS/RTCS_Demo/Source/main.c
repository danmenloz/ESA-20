/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "gpio_defs.h"
#include "LEDs.h"
#include "timers.h"
#include "delay.h"
#include "ADC.h"
#include "config.h"
#include "RTCS.h"

void Init_Debug_Signals(void) {
	// Enable clock to port B
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Make 3 pins GPIO
	PORTB->PCR[DEBUG_RTCS_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DEBUG_RTCS_POS] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DEBUG_RED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DEBUG_RED_POS] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DEBUG_GREEN_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DEBUG_GREEN_POS] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DEBUG_BLUE_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DEBUG_BLUE_POS] |= PORT_PCR_MUX(1);          
	
	// Set ports to outputs
	PTB->PDDR |= MASK(DEBUG_RTCS_POS);
	PTB->PDDR |= MASK(DEBUG_RED_POS);
	PTB->PDDR |= MASK(DEBUG_GREEN_POS);
	PTB->PDDR |= MASK(DEBUG_BLUE_POS);
	
	// Initial values are 0
	PTB->PCOR = MASK(DEBUG_RTCS_POS);
	PTB->PCOR = MASK(DEBUG_RED_POS);
	PTB->PCOR = MASK(DEBUG_GREEN_POS);
	PTB->PCOR = MASK(DEBUG_BLUE_POS);
}	
	
void Task_R(void) {
	static uint8_t LED_On=0;
	
	PTB->PSOR = MASK(DEBUG_RED_POS);
	
	if (LED_On)
		PTB->PCOR = MASK(RED_LED_POS);
	else 
		PTB->PSOR = MASK(RED_LED_POS);
		
	LED_On = 1 - LED_On;
	
	PTB->PCOR = MASK(DEBUG_RED_POS);
}

void Task_G(void) {
	static uint8_t LED_On=0;

	PTB->PSOR = MASK(DEBUG_GREEN_POS);

	if (LED_On)
		PTB->PCOR = MASK(GREEN_LED_POS);
	else 
		PTB->PSOR = MASK(GREEN_LED_POS);
		
	LED_On = 1 - LED_On;

	PTB->PCOR = MASK(DEBUG_GREEN_POS);
}

void Task_B(void) {
	static uint8_t LED_On=0;
	
	PTB->PSOR = MASK(DEBUG_BLUE_POS);
	
	if (LED_On)
		PTD->PCOR = MASK(BLUE_LED_POS);
	else 
		PTD->PSOR = MASK(BLUE_LED_POS);
		
	LED_On = 1 - LED_On;

	PTB->PCOR = MASK(DEBUG_BLUE_POS);
}
/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	Init_Debug_Signals();
	Init_RGB_LEDs();
	
	RTCS_Init(100); // 100 Hz timer ticks
	RTCS_Add_Task(Task_R, 0, 50);
	RTCS_Add_Task(Task_G, 1, 49);
	RTCS_Add_Task(Task_B, 2, 48);

	RTCS_Run_Scheduler();  // This call never returns
}

// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
