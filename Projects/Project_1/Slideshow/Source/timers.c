#include "timers.h"
#include "MKL25Z4.h"

void Init_PIT(unsigned period) {
	// Enable clock to PIT module
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	// Enable module, freeze timers in debug mode
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->MCR |= PIT_MCR_FRZ_MASK;
	
	// Initialize PIT0 to count down from argument 
	PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(period);

	// No chaining
	PIT->CHANNEL[0].TCTRL &= PIT_TCTRL_CHN_MASK;
	
	// Generate interrupts
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;

	/* Enable Interrupts */
	NVIC_SetPriority(PIT_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PIT_IRQn); 
	NVIC_EnableIRQ(PIT_IRQn);	
}


void Start_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void Stop_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}

void PIT_IRQHandler() {
	//clear pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	
	// check to see which channel triggered interrupt 
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 0
		PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
		// Do ISR work
		}
	} else if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 1
		PIT->CHANNEL[1].TFLG &= PIT_TFLG_TIF_MASK;
		// Do ISR work
	} 
}

void PWM_Init(TPM_Type * TPM, uint8_t channel_num, uint16_t period, uint16_t duty)
{
		//turn on clock to TPM 
		switch ((int) TPM) {
			case (int) TPM0:
				SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
				break;
			case (int) TPM1:
				SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
				break;
			case (int) TPM2:
				SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
				break;
			default:
				break;
		}
		//set clock source for tpm
		SIM->SOPT2 |= (SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK);

		//load the counter and mod
		TPM->MOD = period;
			
		//set channels to center-aligned high-true PWM
		TPM->CONTROLS[channel_num].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;

		//set TPM to up-down and divide by 8 prescaler and clock mode
		TPM->SC = (TPM_SC_CPWMS_MASK | TPM_SC_CMOD(1) | TPM_SC_PS(3));
		
		//set trigger mode, keep running when in debug 
		TPM->CONF |= TPM_CONF_TRGSEL(0xA) | TPM_CONF_DBGMODE(3);

		// Set initial duty cycle
		TPM->CONTROLS[channel_num].CnV = duty;
}

void PWM_Set_Value(TPM_Type * TPM, uint8_t channel_num, uint16_t value) {
	TPM->CONTROLS[channel_num].CnV = value;
}


void TPM0_Init(void) {
	//turn on clock to TPM 
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	//set clock source for tpm
	SIM->SOPT2 |= (SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK);
}

void Configure_TPM0_for_DMA(uint32_t period_us)
{

	// disable TPM
	TPM0->SC = 0;
	
	//load the counter and mod
	TPM0->MOD = TPM_MOD_MOD(period_us*48);

	//set TPM to count up and divide by 1 prescaler and clock mode
	TPM0->SC = (TPM_SC_DMA_MASK | TPM_SC_PS(0));
	
#if 0 // if using interrupt for debugging
	// Enable TPM interrupts for debugging
	TPM0->SC |= TPM_SC_TOIE_MASK;

	// Configure NVIC 
	NVIC_SetPriority(TPM0_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(TPM0_IRQn); 
	NVIC_EnableIRQ(TPM0_IRQn);	
#endif

}

void TPM0_Start(void) {
// Enable counter
	TPM0->SC |= TPM_SC_CMOD(1);
}



void TPM0_IRQHandler() {
	//clear pending IRQ flag
	TPM0->SC |= TPM_SC_TOF_MASK; 
}