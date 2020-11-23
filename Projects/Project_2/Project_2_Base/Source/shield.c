#include "shield.h"

#define N_COPIES  (2)

static int set_current[N_COPIES+1]; // original value saved at index 0
static uint8_t initial_MCG_C6;

osEventFlagsId_t evflags_id; //event flags

osThreadId_t t_WDT, t_CLK;
const osThreadAttr_t WDT_attr = {
  .priority = osPriorityBelowNormal           
};
const osThreadAttr_t CLK_attr = {
  .priority = osPriorityHigh          
};


void Write_Set_Current(int new_value){
	g_set_current = new_value;
	for (int i=1; i<N_COPIES+1; i++)
		set_current[i] = new_value;
}

int Read_Set_Current(void){
	//Boyer–Moore majority vote algorithm
	set_current[0] = g_set_current; //store original value at index 0
	int m = 0; // majority value
	int c = 0; // vote counter
	
	for (int i=0; i<N_COPIES+1; i++){
		if(c==0){
			m = set_current[i];
			c+=1;
		}
		else{
			if(m==set_current[i])
				c+=1;
			else
				c-=1;
		}
	}
	// update variable and return majority value
	g_set_current = m;
	return m;
	
}

SPidFX * Read_SPidFX(SPidFX * pid){
	// Scrub fixed gains
	pid->pGain = P_GAIN_FX;
	pid->iGain = I_GAIN_FX;
	pid->dGain = D_GAIN_FX;
	return pid;
}

void Init_COP_WDT(void){
	// 1kHz and 1024 cycle time-out
	SIM->COPC = SIM_COPC_COPT(2) & ~SIM_COPC_COPCLKS_MASK & ~SIM_COPC_COPW_MASK;
}

void Service_COP_WDT(void){
	SIM->SRVCOP = 0x55;
	SIM->SRVCOP = 0xaa;
}

void Shield_Init(void){
	evflags_id = osEventFlagsNew(NULL);
	t_WDT = osThreadNew(Thread_Service_WDT, NULL, &WDT_attr);
	Init_COP_WDT();
	
	initial_MCG_C6 = ~MCG->C6; // store complement
	t_CLK = osThreadNew(Thread_Check_CLK, NULL, &CLK_attr);
}

void Thread_Service_WDT(void * arg) {
	while(1){
		uint32_t flags = osEventFlagsWait(evflags_id, 
							F_READ_TS | F_US | F_BUS,// | F_ADC0,
							osFlagsWaitAll, osWaitForever);
		Service_COP_WDT();
	}
}

void Thread_Check_CLK(void * arg) {
	while(1){
		if ((MCG->C6 ^ initial_MCG_C6) == 0xff){
			osDelay(10); // how often to check?
		}else{
			// scrub CLK freq since they don't match!
			MCG->C6 = ~initial_MCG_C6;
		}			
	}
}
