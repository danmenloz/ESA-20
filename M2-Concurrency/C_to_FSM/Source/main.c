#include <MKL25Z4.H>
#include "gpio_defs.h"

/*
	Code to demonstrate flashing RGB LEDs in sequence based on switch presses.
	Wiring Needed:
			Switch 1: SW1_POS PTD7
			Switch 2: SW2_POS PTD6	
*/

typedef struct {
	enum {
		FSM_IDLE = 0, 	// FSM completed, ready for new data
		FSM_BUSY			 	// FSM busy
	} Status;
	uint16_t Start;
	uint32_t Delay;
	uint16_t Count;
} TCS_DATA_T;

volatile TCS_DATA_T TCS_Data={FSM_IDLE, 0, 0, 0};

void Delay(volatile unsigned int time_del) {
	volatile int n;
	while (time_del--) {
		n = 1000;
		while (n--)
			;
	}
}

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
	
	// Turn on LEDs
	PTB->PCOR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PCOR |= MASK(BLUE_LED_POS);

// Turn off LEDs
	PTB->PSOR |= MASK(RED_LED_POS) | MASK(GREEN_LED_POS);
	PTD->PSOR |= MASK(BLUE_LED_POS);
}

void Init_Debug_Signals(void) {
	// Enable clock to port B
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Make pins GPIO
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
	PTB->PDDR |= MASK(DBG_1) | MASK(DBG_2) | MASK(DBG_3) | MASK(DBG_4) | MASK(DBG_5) | MASK(DBG_6) | MASK(DBG_7);
	
	// Initial values are 0
	PTB->PCOR = MASK(DBG_1) | MASK(DBG_2) | MASK(DBG_3) | MASK(DBG_4) | MASK(DBG_5) | MASK(DBG_6) | MASK(DBG_7);
}	

void Control_RGB_LEDs(int r_on, int g_on, int b_on) {
	if (r_on)
		PTB->PCOR = MASK(RED_LED_POS);
	else
		PTB->PSOR = MASK(RED_LED_POS);
	if (g_on)
		PTB->PCOR = MASK(GREEN_LED_POS);
	else
		PTB->PSOR = MASK(GREEN_LED_POS);
	if (b_on)
		PTD->PCOR = MASK(BLUE_LED_POS);
	else
		PTD->PSOR = MASK(BLUE_LED_POS);
}


// Task code
// Non-FSM version

int Task_Color_Sequence(unsigned int delay_duration) {
	int flash_num;
	int response_delay=0;
	
	PTB->PSOR = MASK(DBG_1);
	Control_RGB_LEDs(1, 0, 0); // Red
	Delay(delay_duration);
	if (SWITCH_PRESSED(SW1_POS)) { 
		Control_RGB_LEDs(1, 0, 1); // Magenta
		Delay(delay_duration);
		response_delay = 0;
		do {
			Control_RGB_LEDs(0, 0, 1); // Blue
			Delay(delay_duration/2);
			Control_RGB_LEDs(1, 0, 0); // Red
			Delay(delay_duration/2);
			response_delay++;
		} while (!SWITCH_PRESSED(SW2_POS));
	} else {
		Control_RGB_LEDs(1, 1, 0); // Yellow
		Delay(delay_duration);
	}
	for (flash_num = 0; flash_num < 3; flash_num++) {
		Control_RGB_LEDs(0, 1, 0); // Green
		Delay(delay_duration);
		Control_RGB_LEDs(0, 0, 0); // Off
		Delay(delay_duration);
	}
	PTB->PCOR = MASK(DBG_1);
	return response_delay;
}


void Task_Color_Sequence_FSM(void) {
	int flash_num;
	static unsigned int delay_duration=2000; // or 100
	static enum {S1,S2,S3} next_state = S1;
	static int response_delay=0;

	switch (next_state) {
		case S1:
			if (TCS_Data.Start > 0) {
				// Clear start request
				TCS_Data.Start = 0;
				// Accept new input data and start processing
				delay_duration = TCS_Data.Delay; 			
				// Initialization
				response_delay = 0;
				TCS_Data.Count = 0;
				TCS_Data.Status = FSM_BUSY;
				Control_RGB_LEDs(1, 0, 0); // Red
				Delay(delay_duration);
				if (SWITCH_PRESSED(SW1_POS)) {
					Control_RGB_LEDs(1, 0, 1); // Magenta
					Delay(delay_duration);
					response_delay = 0;
					next_state = S2;
				} else {
					Control_RGB_LEDs(1, 1, 0); // Yellow
					Delay(delay_duration);
					next_state = S3;
				}
			} else {
				// stay in this state, awaiting a start command
			}
			break;
		case S2:
			Control_RGB_LEDs(0, 0, 1); // Blue
			Delay(delay_duration/2);
			Control_RGB_LEDs(1, 0, 0); // Red
			Delay(delay_duration/2);
			response_delay++;  
			// Could also update Count here for more frequent reporting
			if (SWITCH_PRESSED(SW2_POS))
				next_state = S3;
			else
				next_state = S2; 
			break;
		case S3:
			for (flash_num = 0; flash_num < 3; 
			flash_num++) {
				Control_RGB_LEDs(0, 1, 0); // Green
				Delay(delay_duration);
				Control_RGB_LEDs(0, 0, 0); // Off
				Delay(delay_duration);
			}
			next_state = S1;
			// Prepare return value 
			TCS_Data.Count = response_delay;
			TCS_Data.Status = FSM_IDLE;
			break;
		default: next_state = S1;
			TCS_Data.Status = FSM_IDLE;
			TCS_Data.Count = 0;
			break;
	}
}

void Task_Another_FSM(void) {
	static enum {S1,S2} next_state = S1;
	uint16_t n;

	switch (next_state) {
		case S1: // Wait for TCS to be idle and then start it
			if (TCS_Data.Status == FSM_IDLE) {
				TCS_Data.Delay = 1500;
				TCS_Data.Start = 1;
				next_state = S2;
			}
			break;
		case S2: // Wait for TCS to finish its work
			if ((TCS_Data.Status == FSM_IDLE) & (TCS_Data.Start == 0)) {
				// can now use TCS_Data.Count
				n = TCS_Data.Count;
				// use it for something
				next_state = S1;
			}
			break;
		default:
			next_state = S1;
			break;
		}
	}

void Basic_Scheduler(void) {
	while (1) {
#if 0
		Task_Color_Sequence(2000);
#else
		Task_Color_Sequence_FSM();
		Task_Another_FSM();
#endif
	}
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	Initialize_Ports();
	Init_Debug_Signals();
	Basic_Scheduler();
}
