#ifndef SWITCHES_H
#define SWITCHES_H

#define USE_DISCRETE_DIGITAL_INPUTS 0

// If digital, all switches are on port E
// If analog, each of these values must be unique
#define SW_UP_POS (21)
#define SW_DN_POS (29)
#define SW_LT_POS (30)
#define SW_RT_POS (23)
#define SW_CR_POS (22)

#define NUM_SWITCHES (5)

#if USE_DISCRETE_DIGITAL_INPUTS

// Macro to read switches returns state switches, active low
#define READ_SWITCHES  (PTE->PDIR)  

#else
extern unsigned ReadSwitchesViaAnalog(void);
#define READ_SWITCHES ReadSwitchesViaAnalog()
#define SW_ADC_CHANNEL (8)

#define VERTICAL_5_WAY_SWITCH (0)

// vertical pins
#define SW_N_VALUE (65312)
#define SW_LT_VALUE (13799)
#define SW_DN_VALUE (9642)
#define SW_RT_VALUE (7317)
#define SW_UP_VALUE (1927)
#define SW_CR_VALUE (0)

typedef struct {
	unsigned Result;
	unsigned High;
	unsigned Low;
} RANGE_LOOKUP_T;
#endif

// Function prototypes
extern void Init_5way_Switch(void);

#endif
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
