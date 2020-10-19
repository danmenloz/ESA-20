#ifndef TIMERS_H
#define TIMERS_H
#include "MKL25Z4.h"
#include "cmsis_os2.h"

#define USE_PIT (0)

#define LCD_UPDATE_PERIOD 10

#define COMPILE_PIT_STOP 0 // switch to 0 to save code size!

void TPM_Init(unsigned period_ms);

void TPM0_Init(void);
void TPM0_Start(void);
void Configure_TPM0_for_DMA(uint32_t period_us);

void PWM_Init(TPM_Type * TPM, uint8_t channel_num, uint16_t period, uint16_t duty);
void PWM_Set_Value(TPM_Type * TPM, uint8_t channel_num, uint16_t value);


void LPTMR_Init(void);
void LPTMR_Start(void);
void LPTMR_Stop(void);

void PIT_IRQHandler(void);
void PIT_Init(uint32_t virtual_channel, uint32_t delay, osThreadId_t tid, uint32_t flag);
void PIT_Start(uint32_t virtual_channel);
# if COMPILE_PIT_STOP
void PIT_Stop(uint32_t virtual_channel);
# endif
void precise_delay(uint32_t virtual_channel, uint32_t delay_usec);

struct vch_info {
	osThreadId_t id;
	uint32_t flag;
	uint32_t count; // counter
	uint32_t preemptive; // preemtive request
	uint32_t preempted; // channel preempted?
	uint32_t p_count; // preempted conter
};
	
#endif

