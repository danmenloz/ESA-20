#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#define MASK(x) (1UL << (x))

// Debug outputs on Port B
#define DEBUG_RTCS_POS 	0		// J10 pin 2
#define DEBUG_RED_POS 	1 	// J10 pin 4
#define DEBUG_GREEN_POS 2 	// J10 pin 6
#define DEBUG_BLUE_POS 	3		// J10 pin 8

// Debug Signals on port B
#define DBG_1 1
#define DBG_2 2
#define DBG_3 3
#define DBG_4 8
#define DBG_5 9
#define DBG_6 10
#define DBG_7 11

// Switch Inputs
#define SW1_POS (7)		// on port D
#define SW2_POS (6)		// on port D

#define SWITCH_PRESSED(x) (!(PTD->PDIR & (MASK(x))))

#define PRIO_RS 3
#define PRIO_TF 1
#define PRIO_TRGB 2

#define ON_TIME (4000)
#define FLASH_TIME (600)

#if 1 // Software delays
#define W_DELAY_SLOW 400
#define W_DELAY_FAST 200
#define RGB_DELAY_SLOW 6000
#define RGB_DELAY_FAST 1000
#else
#define W_DELAY_SLOW 200
#define W_DELAY_FAST 100
#define RGB_DELAY_SLOW 3600
#define RGB_DELAY_FAST 800
#endif

#endif
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
