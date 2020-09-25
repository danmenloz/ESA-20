#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#include "LEDs.h"

void Thread_Blinky(void * argument) {
	for (;;) { // CYMK!
		Control_RGB_LEDs(0,1,1);
		osDelay(500);
		Control_RGB_LEDs(1,0,1);
		osDelay(500);
		Control_RGB_LEDs(1,1,0);
		osDelay(500);
		Control_RGB_LEDs(0,0,0);
		osDelay(500);
	}
}
