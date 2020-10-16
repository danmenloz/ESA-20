#include "timers.h"
#include "MKL25Z4.h"

#define V_CHANNELS 4 // number of virtual channels
struct vch_info volatile vch_arr[V_CHANNELS]; // array of virtual channels' info
struct vch_info volatile * volatile current_vch; // pointer to the virtual channel currently using the timer

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



void TPM0_IRQHandler(void) {
	//clear pending IRQ flag
	TPM0->SC |= TPM_SC_TOF_MASK; 
}
void PIT_Init(uint32_t period ) {
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


void PIT_Start(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void PIT_Stop(void) {
// Disable counter
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}

// Clear virtual channel info
void clearInfo(struct vch_info *vch){
	vch->id = NULL;
	vch->flag = 0;
	vch->count = 0;
	vch->en = 0;
}

void virtual_PIT_Init(uint32_t virtual_channel, uint32_t delay, osThreadId_t tid, uint32_t flag){
	// if PIT not initialized, do it!
	if (~(SIM->SCGC6 & SIM_SCGC6_PIT_MASK)){
		PIT_Init(delay);
		// initialize array
		for(int i=0; i<V_CHANNELS; i++){
			//clearInfo(&vch_arr[i]);
			vch_arr[i].id = NULL;
			vch_arr[i].flag = 0;
			vch_arr[i].count = 0;
			vch_arr[i].en = 0;
		}
	}
	
	// Set channel info
	vch_arr[virtual_channel].id = tid;
	vch_arr[virtual_channel].flag = flag;
	vch_arr[virtual_channel].count = delay;
	
}

void virtual_PIT_Start(uint32_t virtual_channel){
	// PIT running?
	if(PIT->CHANNEL[0].TCTRL & PIT_TCTRL_TEN_MASK){
		if( vch_arr[virtual_channel].count > PIT->CHANNEL[0].CVAL ){
			vch_arr[virtual_channel].count -= PIT->CHANNEL[0].CVAL;
		}else{
			// run this channel first since it's shorter
		
			current_vch = &vch_arr[virtual_channel];
			PIT_Stop();
			PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(current_vch->count);
			PIT_Start();
			uint32_t gap = PIT->CHANNEL[0].CVAL - current_vch->count;
			// update channels info
			for(int i=0; i<V_CHANNELS; i++){
				if(vch_arr[i].en && &vch_arr[i]!=current_vch)
					vch_arr[virtual_channel].count += gap;
			}
			current_vch->count = 0; // clear count
			
		}
	}else{// PIT is disabled, this is the first virtual channel being used
		current_vch = &vch_arr[virtual_channel];
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(current_vch->count);
		PIT_Start();
		current_vch->count = 0; // clear count
	}
	
	vch_arr[virtual_channel].en = 1;
}
	
void virtual_PIT_Stop(uint32_t virtual_channel){
	vch_arr[virtual_channel].en = 0;
}

void PIT_IRQHandler(void) {
	//clear pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	
	// check to see which channel triggered interrupt 
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 0
		PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
		// Do ISR work
		
		// Determine which virtual channel caused the interrupt
		osThreadId_t th = current_vch->id; //(*current_vch).id;
		uint32_t flag = current_vch->flag;
		//clearInfo(current_vch); // clear channel info
		current_vch->id = NULL;
		current_vch->flag = 0;
		current_vch->count = 0;
		current_vch->en = 0;
		
		// Reconfigure the PIT for the next virtual channel event (if any)
		PIT_Stop();
		// Find the lowest-enabled channel counter and load it to the PIT value
		uint32_t min;
		for(int i=0; i<V_CHANNELS; i++){
			if (vch_arr[i].en){
				min = vch_arr[i].count;
				for(int i=0; i<V_CHANNELS; i++){
					if (vch_arr[i].count < min && vch_arr[i].en){
						min = vch_arr[i].count; 
						current_vch = &vch_arr[i];
					}
				}
			}
		}
		if(current_vch->en){ // virtual channel enabled? min found!
			//stop
			PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(current_vch->count); // set PIT val
			PIT_Start();
			//update count of all enabled channels
			for(int i=0; i<V_CHANNELS; i++){
				if(vch_arr[i].en)
					vch_arr[i].count -= current_vch->count;
			}
		}
		
		// Call osThreadsFlagsSet to let the thread resume
		uint32_t result = osThreadFlagsSet(th, flag);
		
		}
	if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 1
		PIT->CHANNEL[1].TFLG &= PIT_TFLG_TIF_MASK;
		// Do ISR work
	} 
}
