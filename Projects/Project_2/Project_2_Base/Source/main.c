/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "math.h"

#include "gpio_defs.h"

#include <cmsis_os2.h>
#include "threads.h"

#include "LCD.h"
#include "LCD_driver.h"
#include "font.h"

#include "LEDs.h"
#include "timers.h"
#include "sound.h"
#include "DMA.h"
#include "delay.h"
#include "profile.h"
#include "control.h"
#include "fault.h"
#include "shield.h"

volatile CTL_MODE_E control_mode=DEF_CONTROL_MODE;

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
 Init_Debug_Signals();
 Init_RGB_LEDs();
 Control_RGB_LEDs(0,0,1);   
 Sound_Disable_Amp();
 LCD_Init();
 LCD_Text_Init(1);
 LCD_Erase();
 Init_Buck_HBLED();
 
 osKernelInitialize();
 Fault_Init();
 Shield_Init();
 Create_OS_Objects();
 osKernelStart();
}
