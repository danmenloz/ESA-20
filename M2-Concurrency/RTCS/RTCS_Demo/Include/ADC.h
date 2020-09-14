#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <MKL25Z4.H>

#define BANDGAP_CHANNEL (27)
#define MAX_ADC_CHANNEL_NUM (30)

void Init_ADC(void);
float Measure_VRail(void);
float Measure_VRef(void);

#endif
