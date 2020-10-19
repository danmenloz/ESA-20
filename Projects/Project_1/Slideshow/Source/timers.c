#include "timers.h"
#include "MKL25Z4.h"
#include "debug.h"

#define V_CHANNELS 4 // number of virtual channels
struct vch_info volatile vch_arr[V_CHANNELS]; // array of virtual channels' info, volatile struct
struct vch_info volatile * volatile current_vch; // volatile pointer to volitile struct
//uint32_t PIT_en = 0;

void PWM_Init(TPM_Type * TPM, uint8_t channel_num, uint16_t period, uint16_t duty){
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

void Configure_TPM0_for_DMA(uint32_t period_us){

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
void precise_delay(uint32_t virtual_channel, uint32_t delay_usec){
	// remove offset caused by OS context switch
	if (delay_usec>=30)
		delay_usec -= 30;
	else 
		delay_usec = 1; // smalles possible delay. It will be saturated
	
	uint32_t ldval = (uint32_t) delay_usec*24-1; // PIT formula
	uint32_t flag = 0x00000001U;
	
	// Saturate load value. The smallest working value is LDVAL=500
	if (ldval<=500)
		ldval = 500;
	
	PIT_Init(virtual_channel, ldval , osThreadGetId(), flag);
	PIT_Start(virtual_channel);
	osThreadFlagsClear(flag); //clear flag before waiting
	uint32_t result = osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever); //wait until flag is set
}
void PIT_Init(uint32_t virtual_channel, uint32_t delay, osThreadId_t tid, uint32_t flag){
	// if PIT not initialized, do it only once!
	uint32_t clk_en = (SIM->SCGC6 & SIM_SCGC6_PIT_MASK) >> SIM_SCGC6_PIT_SHIFT;
	if (clk_en == 0){
		// Initialize PIT:
		// Enable clock to PIT module
		SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
		
		// Enable module, freeze timers in debug mode
		PIT->MCR &= ~PIT_MCR_MDIS_MASK;
		PIT->MCR |= PIT_MCR_FRZ_MASK;
		
		// Initialize PIT0 to count down from argument 
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(delay);

		// No chaining
		PIT->CHANNEL[0].TCTRL &= PIT_TCTRL_CHN_MASK;
		
		// Generate interrupts
		PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;

		/* Enable Interrupts */
		NVIC_SetPriority(PIT_IRQn, 128); // 0, 64, 128 or 192
		NVIC_ClearPendingIRQ(PIT_IRQn); 
		NVIC_EnableIRQ(PIT_IRQn);
		
		// initialize virtual channel array
		for(int i=0; i<V_CHANNELS; i++){
			vch_arr[i].id = NULL;
			vch_arr[i].flag = 0;
			vch_arr[i].count = 0;	
			vch_arr[i].preemptive = 0;
			vch_arr[i].preempted = 0;
			vch_arr[i].p_count = 0;
		}
	}
	// Set channel info
	vch_arr[virtual_channel].id = tid;
	vch_arr[virtual_channel].flag = flag;
	vch_arr[virtual_channel].count = delay;
}

void PIT_Start(uint32_t virtual_channel){
	// PIT enabled?
	//uint32_t PIT_en = (PIT->CHANNEL[0].TCTRL & PIT_TCTRL_TEN_MASK) >> PIT_TCTRL_TEN_SHIFT;
	if( PIT->CHANNEL[0].TCTRL & PIT_TCTRL_TEN_MASK ) {
		uint32_t cval = PIT->CHANNEL[0].CVAL; //safe to read CVAL 
		if( vch_arr[virtual_channel].count > cval ){
			// don't run channel, wait until the current timer expires
			vch_arr[virtual_channel].count -= cval;
		}else{
			// run this channel immediatly since it's the shortest
			vch_arr[virtual_channel].preemptive = 1; // this is a preemptive timer
			
			// run timer
			PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop();
			PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(vch_arr[virtual_channel].count);
			PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;// PIT_Start();
			
			vch_arr[virtual_channel].count = cval - vch_arr[virtual_channel].count; // gap
			current_vch->preempted = 1; // set preempted field
			current_vch->p_count = current_vch->count; // save channel count to be restored later
			current_vch->count = vch_arr[virtual_channel].count; // temporarily store gap in the current channel's counter
			current_vch = &vch_arr[virtual_channel]; //change current channel		
		}
	}else{// PIT is disabled, only one virtual channel is being used
		// run timer
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop();
		PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(vch_arr[virtual_channel].count);
		PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;// PIT_Start();
		vch_arr[virtual_channel].count = 0; // clear count since no pending channel requests
		current_vch = &vch_arr[virtual_channel]; // set current channel
	}
}

# if COMPILE_PIT_STOP
void PIT_Stop(uint32_t virtual_channel){
	// Big picture: change control to the next virtual channel in line
	// find next channel
	struct vch_info volatile *min = &vch_arr[virtual_channel]; // pointer to volatile struct
	min->count = 0; //clear count to disable channel
	uint32_t pending = 0;
	// first find an enabled channel
	for(int i=0; i<V_CHANNELS; i++){
		if(vch_arr[i].count > 0){ // 0 means virtual channel is disabled
			min = &vch_arr[i];
			pending++;
		}
	}
	if(pending){ // at least one pending channel request found!
		// find minimum
		for(int i=0; i<V_CHANNELS; i++){
			if(vch_arr[i].count < min->count  &&  vch_arr[i].count>0)
				min = &vch_arr[i];
		}
		// change control to next channel request
		if(PIT->CHANNEL[0].TCTRL & PIT_TCTRL_TEN_MASK){ // check if PIT is running, it should be
			// run next channel
			PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop();
			uint32_t cval = PIT->CHANNEL[0].CVAL; //safe to read CVAL 
			PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV( min->count + cval ); //load timer value
			PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; //PIT_Start();  	
			// update all pending requests with new baseline
			if (pending>1 && !min->preemptive){ 
				for(int i=0; i<V_CHANNELS; i++){
					if ( vch_arr[i].count>0 && min!=&vch_arr[i] )
						vch_arr[i].count -= min->count;
				}
			}else{ // no pending requests
				min->count = 0; 
			}
			current_vch = min; // update current channel		
		}
	}else{ //only this virtual channel was running
		PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop(); 
	}
	//clear fields
	vch_arr[virtual_channel].id = NULL;
	vch_arr[virtual_channel].flag = 0;
	vch_arr[virtual_channel].count = 0;	
	vch_arr[virtual_channel].preemptive = 0;
	vch_arr[virtual_channel].preempted = 0;
	vch_arr[virtual_channel].p_count = 0;
}
# endif

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
		
		// Reconfigure the PIT for the next virtual channel event (if any)
		// Find the lowest-enabled channel counter and load it to the PIT value
		struct vch_info volatile *min = current_vch; // pointer to volatile struct
		uint32_t pending = 0;
		// first find an enabled channel
		for(int i=0; i<V_CHANNELS; i++){
			if(vch_arr[i].count>0 && &vch_arr[i]!=current_vch){ // enabled and !current
				min = &vch_arr[i];
				pending++;
			}
		}
		if(pending){ // at least one pending request found
			// find minimum
			for(int i=0; i<V_CHANNELS; i++){
				if(vch_arr[i].count < min->count && vch_arr[i].count>0 && &vch_arr[i]!=current_vch)
					min = &vch_arr[i];
			}
			//min is the next request
			if (current_vch->preemptive){
				current_vch->preemptive = 0; // clear preemptive field
				// run channel
				PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop();
				PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV( min->count ); //load timer value
				PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; //PIT_Start();
				if(min->preempted){// channel request was preempted?
					min->preempted = 0; //clear preempted field
					min->count = min->p_count; //restore counter before preemption
				}
				current_vch = min; //update current channel
			}else{ //!preemptive
				min->count -= current_vch->count;
				
				//clear channel
				current_vch->id = NULL;
				current_vch->flag = 0;
				current_vch->count = 0;	
				current_vch->preemptive = 0;
				current_vch->preempted = 0;
				current_vch->p_count = 0;
				
				// run channel
				PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK; //PIT_Stop();
				PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV( min->count ); //load timer value
				PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; //PIT_Start();
				
				current_vch = min; //update current channel
				// search pending requests
				if (pending<=1)// no other pending requests?
					current_vch->count = 0; // count no longer needed since no pending requests
			}
		}
		else{ // no pending requests
			PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;  //PIT_Stop();
			//clear channel
			current_vch->id = NULL;
			current_vch->flag = 0;
			current_vch->count = 0;	
			current_vch->preemptive = 0;
			current_vch->preempted = 0;
			current_vch->p_count = 0;
		}
		
		// Call osThreadsFlagsSet to let the thread resume
			uint32_t result = osThreadFlagsSet(th, flag);
			if(result == flag)
				DEBUG_TOGGLE(DBG_5);
	}
		
	if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 1
		PIT->CHANNEL[1].TFLG &= PIT_TFLG_TIF_MASK;
		// Do ISR work
	} 
}
