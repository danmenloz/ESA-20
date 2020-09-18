/*----------------------------------------------------------------------------
	Improved version of RGB Flasher on RTCS
	- Long tasks are FSM-based.
	- Delays are provided by scheduler based on task period.
	- Switches trigger interrupts. 
	- Switch ISR adjusts task periods, enable/disables/releases tasks.
	- Idle time available to scheduler.
 *----------------------------------------------------------------------------*/

#include <MKL25Z4.H>
#include <stdio.h>
#include "gpio_defs.h"
#include "LEDs.h"
#include "timers.h"
#include "RTCS.h"

volatile uint32_t g_w_delay=W_DELAY_SLOW; 		// delay for white flash
volatile uint32_t g_RGB_delay=RGB_DELAY_SLOW; 	// delay for RGB sequence

void Initialize_Ports(void) {
	// Enable clock to ports A, B and D
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;;
	
	// Make 3 pins GPIO
	PORTB->PCR[RED_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[RED_LED_POS] |= PORT_PCR_MUX(1);          
	PORTB->PCR[GREEN_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[GREEN_LED_POS] |= PORT_PCR_MUX(1);          
	PORTD->PCR[BLUE_LED_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTD->PCR[BLUE_LED_POS] |= PORT_PCR_MUX(1);          
	
	// Set LED port bits to outputs
	PTB->PDDR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PDDR |= MASK(BLUE_LED_POS);

	// Select port D on pin mux, enable pull-up resistors
	PORTD->PCR[SW1_POS] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;
	PORTD->PCR[SW2_POS] = PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;

	// Clear switch bits to input
	PTD->PDDR &= ~MASK(SW1_POS); 
	PTD->PDDR &= ~MASK(SW2_POS); 
	
	// Turn off LEDs
	PTB->PSOR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PSOR |= MASK(BLUE_LED_POS);
}

void Init_Debug_Signals(void) {
	// Enable clock to port B
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Make pins GPIO
	PORTB->PCR[DBG_0] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_0] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_1] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_1] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_2] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_2] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_3] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_3] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_4] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_4] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_5] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_5] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_6] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_6] |= PORT_PCR_MUX(1);          
	PORTB->PCR[DBG_7] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[DBG_7] |= PORT_PCR_MUX(1);          

	
	// Set ports to outputs
	PTB->PDDR |= MASK(DBG_0) | MASK(DBG_1) | MASK(DBG_2) | MASK(DBG_3) | MASK(DBG_4) | MASK(DBG_5) | MASK(DBG_6) | MASK(DBG_7);
	
	// Initial values are 0
	PTB->PCOR = MASK(DBG_0) | MASK(DBG_1) | MASK(DBG_2) | MASK(DBG_3) | MASK(DBG_4) | MASK(DBG_5) | MASK(DBG_6) | MASK(DBG_7);
}	
	
void Init_Interrupts(void) {
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
	
	/* Ensure PRIMASK allows interrupts */
	__enable_irq();
}

void Task_Flash_FSM(void) {
	static enum {ST_WHITE, ST_BLACK} next_state = ST_WHITE;

	PTB->PSOR = MASK(DEBUG_FLASH_POS);
	switch (next_state) {
		case ST_WHITE:
			Control_RGB_LEDs(1, 1, 1);
			next_state = ST_BLACK;
			break;
		case ST_BLACK:
			Control_RGB_LEDs(0, 0, 0);
			next_state = ST_WHITE;
			break;
		default:
			next_state = ST_WHITE;
			break;
	}
	PTB->PCOR = MASK(DEBUG_FLASH_POS);
}

void Task_RGB_FSM(void) {
	static enum {ST_RED, ST_GREEN, ST_BLUE, ST_OFF} next_state = ST_RED;

	PTB->PSOR = MASK(DEBUG_RGB_POS);
	switch (next_state) {
		case ST_RED:
			Control_RGB_LEDs(1, 0, 0);
			next_state = ST_GREEN;
			break;
		case ST_GREEN:
			Control_RGB_LEDs(0, 1, 0);
			next_state = ST_BLUE;
			break;
		case ST_BLUE:
			Control_RGB_LEDs(0, 0, 1);
			next_state = ST_RED;
			break;
		default:
			next_state = ST_RED;
			break;
	}
	PTB->PCOR = MASK(DEBUG_RGB_POS);
}

void PORTD_IRQHandler(void) {  
	PTB->PSOR = MASK(DEBUG_ISR_POS);
	// Read switches
	if ((PORTD->ISFR & MASK(SW1_POS))) {	
		if (SWITCH_PRESSED(SW1_POS)) { // flash white
			RTCS_Enable_Task(Task_Flash_FSM, 1);
			RTCS_Release_Task(Task_Flash_FSM);
			RTCS_Enable_Task(Task_RGB_FSM, 0); // disable RGB
		} else {
			RTCS_Enable_Task(Task_RGB_FSM, 1);
			RTCS_Release_Task(Task_RGB_FSM);
			RTCS_Enable_Task(Task_Flash_FSM, 0); // disable flash
		}
	}
	if ((PORTD->ISFR & MASK(SW2_POS))) {	
		if (SWITCH_PRESSED(SW2_POS)) {
			RTCS_Set_Task_Period(Task_Flash_FSM, W_DELAY_FAST, 0);
			RTCS_Set_Task_Period(Task_RGB_FSM, RGB_DELAY_FAST, 0);
		}	else {
			RTCS_Set_Task_Period(Task_Flash_FSM, W_DELAY_SLOW, 0);
			RTCS_Set_Task_Period(Task_RGB_FSM, RGB_DELAY_SLOW, 0);
		}
	}
	// clear status flags 
	PORTD->ISFR = 0xffffffff;
	PTB->PCOR = MASK(DEBUG_ISR_POS);
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	Initialize_Ports();
	Init_Debug_Signals();
	Init_RGB_LEDs();
	Init_Interrupts();
	
	RTCS_Init(100); // 100 Hz timer ticks

	RTCS_Add_Task(Task_Flash_FSM, 0, W_DELAY_SLOW);
	RTCS_Add_Task(Task_RGB_FSM, 1, RGB_DELAY_SLOW);

	RTCS_Enable_Task(Task_Flash_FSM, 0);
	
	RTCS_Run_Scheduler();  // This call never returns
}

