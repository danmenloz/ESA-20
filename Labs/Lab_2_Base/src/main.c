/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "gpio_defs.h"
#include "LEDs.h"
#include "UART.h"
#include "delay.h"

void Init_ADC(void) {
	ADC0->CFG1 = ADC_CFG1_ADLPC_MASK | ADC_CFG1_ADIV(0) | ADC_CFG1_ADICLK(0) | 
	ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE(3);
	ADC0->SC2 = ADC_SC2_REFSEL(0); // VREFHL selection, software trigger
}

float Measure_VRail(void) {
	volatile float vrail;
	unsigned res=0;
	
	ADC0->SC1[0] = ADC_SC1_ADCH(27); // start conversion on channel 27 (Bandgap reference)
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	res = ADC0->R[0];
	vrail = (1.0/res)*65536;
	return vrail;
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	float voltage = 0.0;
	
	Init_RGB_LEDs();
	Control_RGB_LEDs(1, 1, 0);

	Init_ADC(); 
	Control_RGB_LEDs(0, 0, 1);

	Init_UART0(115200); 
	Control_RGB_LEDs(0, 1, 0);
	
	printf("\n\r\n\r\n\rDebug me!\n\r");
	
	while (1) {
		voltage = Measure_VRail();
		printf("P3V3 Rail is at %6.4d V\n\r", voltage);
		Delay(1000);
	}
}

// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
