#ifndef TIMERS_H
#define TIMERS_H
#include "MKL25Z4.h"
#include "cmsis_os2.h"

#define USE_PIT (0)

#define LCD_UPDATE_PERIOD 10

void TPM_Init(unsigned period_ms);

void TPM0_Init(void);
void TPM0_Start(void);
void Configure_TPM0_for_DMA(uint32_t period_us);

void PWM_Init(TPM_Type * TPM, uint8_t channel_num, uint16_t period, uint16_t duty);
void PWM_Set_Value(TPM_Type * TPM, uint8_t channel_num, uint16_t value);


void LPTMR_Init(void);
void LPTMR_Start(void);
void LPTMR_Stop(void);

void PIT_Init(uint32_t period);
void PIT_Start(void);
void PIT_Stop(void);
void PIT_IRQHandler(void);
void virtual_PIT_Init(uint32_t virtual_channel, uint32_t delay, osThreadId_t tid, uint32_t flag);
void virtual_PIT_Start(uint32_t virtual_channel);
void virtual_PIT_Stop(uint32_t virtual_channel);

struct vch_info {
	osThreadId_t id;
	uint32_t flag;
	uint32_t count; // counter
	uint32_t en; // enable
};

void clearInfo(struct vch_info *vch);
	
#endif

