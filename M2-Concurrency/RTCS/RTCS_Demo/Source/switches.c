#include <MKL25Z4.H>
#include "switches.h"
#include "gpio_defs.h"

void Init_5way_Switch(void) {
	
#if USE_DISCRETE_DIGITAL_INPUTS
  SIM->SCGC5 |=  SIM_SCGC5_PORTE_MASK; /* enable clock for port E */
	
  /* Select GPIO and enable pull-up resistors for pins connected to switches */
  PORTE->PCR[SW_UP_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;	
  PORTE->PCR[SW_DN_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;	
  PORTE->PCR[SW_LT_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;	
  PORTE->PCR[SW_RT_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;	
  PORTE->PCR[SW_CR_POS] |= PORT_PCR_MUX(1) | PORT_PCR_PS_MASK | PORT_PCR_PE_MASK;	
	
  /* Set port E bits 0-3, 7 to inputs */
  PTE->PDDR &= ~( MASK(SW_UP_POS) | 
				  MASK(SW_DN_POS) |
				  MASK(SW_LT_POS) | 
				  MASK(SW_RT_POS) |
				  MASK(SW_CR_POS) );
#else
	/* Enable clock for ADC */
	SIM->SCGC6 |= (1UL << SIM_SCGC6_ADC0_SHIFT); 
	ADC0->CFG1 = 0x9C; // 16 bit conversion
	ADC0->SC2 = 0; // single ended mode
	#endif
}

#if USE_DISCRETE_DIGITAL_INPUTS
#else

#if VERTICAL_5_WAY_SWITCH 
RANGE_LOOKUP_T switch_converter[NUM_SWITCHES] = {
																			{~(1 << SW_LT_POS), (SW_N_VALUE + SW_LT_VALUE)/2 - 1, (SW_LT_VALUE+SW_DN_VALUE)/2},
																			{~(1 << SW_DN_POS), (SW_LT_VALUE + SW_DN_VALUE)/2 - 1, (SW_DN_VALUE+SW_RT_VALUE)/2},
																			{~(1 << SW_RT_POS), (SW_DN_VALUE + SW_RT_VALUE)/2 - 1, (SW_RT_VALUE+SW_UP_VALUE)/2},
																			{~(1 << SW_UP_POS), (SW_RT_VALUE + SW_UP_VALUE)/2 - 1, (SW_UP_VALUE+SW_CR_VALUE)/2},
																			{~(1 << SW_CR_POS), (SW_UP_VALUE + SW_CR_VALUE)/2 - 1, 0}};
#else
RANGE_LOOKUP_T switch_converter[NUM_SWITCHES] = {
																			{~(1 << SW_DN_POS), (SW_N_VALUE + SW_LT_VALUE)/2 - 1, (SW_LT_VALUE+SW_DN_VALUE)/2},
																			{~(1 << SW_RT_POS), (SW_LT_VALUE + SW_DN_VALUE)/2 - 1, (SW_DN_VALUE+SW_RT_VALUE)/2},
																			{~(1 << SW_UP_POS), (SW_DN_VALUE + SW_RT_VALUE)/2 - 1, (SW_RT_VALUE+SW_UP_VALUE)/2},
																			{~(1 << SW_LT_POS), (SW_RT_VALUE + SW_UP_VALUE)/2 - 1, (SW_UP_VALUE+SW_CR_VALUE)/2},
																			{~(1 << SW_CR_POS), (SW_UP_VALUE + SW_CR_VALUE)/2 - 1, 0}};

#endif

unsigned ReadSwitchesViaAnalog(void) {
	unsigned res=0, i;
	
	// Read ADC
	ADC0->SC1[0] = SW_ADC_CHANNEL; //start conversion
	
	// block until conversion is done
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	
	res = ADC0->R[0];
	// Look it up in the conversion table
	for (i=0; i<NUM_SWITCHES; i++) {
		if ((switch_converter[i].High >= res) && (res >= switch_converter[i].Low))
			return switch_converter[i].Result;
	}
	return 0xffffffff;
}
#endif
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
