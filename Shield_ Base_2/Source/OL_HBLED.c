#include <MKL25Z4.H>
#include "OL_HBLED.h"
#include "timers.h"

void Init_OL_HBLED(void) {
	// Configure driver for buck converter
	// Set up PTE31 to use for SMPS with TPM0 Ch 4
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	PORTE->PCR[31]  &= PORT_PCR_MUX(7);
	PORTE->PCR[31]  |= PORT_PCR_MUX(3);
	PWM_Init(TPM_HBLED, PWM_HBLED_CHANNEL, PWM_PERIOD, LIM_DUTY_CYCLE/128);
}

void Set_OL_HBLED_Pulse_Width(uint16_t pw) {
	if (pw > LIM_DUTY_CYCLE)
		pw = LIM_DUTY_CYCLE;
	PWM_Set_Value(TPM_HBLED, PWM_HBLED_CHANNEL, pw);
}
