#include "ADC.h"

void Init_ADC(void) {
	
	SIM->SCGC6 |= (1UL << SIM_SCGC6_ADC0_SHIFT); 
	ADC0->CFG1 = 0x9C; // 16 bit
	ADC0->SC2 = 0;
	// Enable Bandgap buffer
	PMC->REGSC |= PMC_REGSC_BGBE_MASK;
}

float Measure_VIn(uint8_t channel) {
	float vin; 
	uint16_t vbg1, vbg2;
	uint16_t res=0;
	
	if (channel > MAX_ADC_CHANNEL_NUM)
		return (-1.0);
		
	// Read bandgap reference
	ADC0->SC1[0] = BANDGAP_CHANNEL ; // start conversion on BG channel 
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	vbg1 = ADC0->R[0];
	
	// Read supply rail 
	ADC0->SC1[0] = 0x00; // start conversion on channel 0
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	res = ADC0->R[0];
	
	// Read bandgap again
	ADC0->SC1[0] = BANDGAP_CHANNEL ; // start conversion on BG channel
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	vbg2 = ADC0->R[0];
	
	vbg1 = (vbg1+vbg2)/2;
	vin = ((float) res)/vbg1;
	return vin; 
}

float Measure_VRef(void) {
	volatile float vref; 
	uint16_t vbg;
	
	// Read bandgap reference
	ADC0->SC1[0] = BANDGAP_CHANNEL ; // start conversion on BG channel 
	while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
		;
	vbg = ADC0->R[0];
	
	// compute reference voltage
	vref = 65535.0/vbg;
	return vref; 
}
