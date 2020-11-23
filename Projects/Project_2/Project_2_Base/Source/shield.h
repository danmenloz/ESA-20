#include <MKL25Z4.H>
#include "fault.h"
#include <cmsis_os2.h>
#include "control.h"
#include "LEDs.h"
#include "threads.h"

// Event flags
#define F_READ_TS  	0x00000001U
#define F_US  			0x00000002U
#define F_BUS				0x00000004U
#define F_DMA0			0x00000008U
#define F_ADC0			0x00000010U
#define F_TPM0			0x00000020U
#define F_PIT				0x00000040U

// Shared global variables
extern volatile int g_set_current; // Default starting LED current
extern osEventFlagsId_t evflags_id;

void Write_Set_Current(int new_value);
int Read_Set_Current(void);
SPidFX * Read_SPidFX(SPidFX * pid);
void Init_COP_WDT(void);
void Shield_Init(void);
void Service_COP_WDT(void);
void Thread_Service_WDT(void * arg);
void Thread_Check_CLK(void * arg);

