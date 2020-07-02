#ifndef OL_HBLED_H
#define OL_HBLED_H

// Open-Loop HBLED Driver code

// Switching parameters
#define TPM_HBLED (TPM0)
#define PWM_HBLED_CHANNEL (4)
#define PWM_PERIOD (400) 
/* 48 MHz input clock. 
	PWM frequency = 48 MHz/(PWM_PERIOD*2) 
	Timer is in count-up/down mode. */

#define LIM_DUTY_CYCLE (PWM_PERIOD)

void Init_OL_HBLED(void);
void Set_OL_HBLED_Pulse_Width(uint16_t pw);

#endif // OL_HBLED_H
