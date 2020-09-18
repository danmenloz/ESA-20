#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

// Switch positions
#define SW1_POS (7)		// on port D
#define SW2_POS (6)		// on port D

#define MASK(x) (1UL << (x))

// Debug Signals on port B
#define DBG_0 0
#define DBG_1 1
#define DBG_2 2
#define DBG_3 3
#define DBG_4 8
#define DBG_5 9
#define DBG_6 10
#define DBG_7 11

#define DEBUG_RS_POS (DBG_0) 				// B1
#define DEBUG_FLASH_POS (DBG_1) 		// B1
#define DEBUG_RGB_POS (DBG_2)				// B2
#define DEBUG_ISR_POS (DBG_3)				// B3
#define DEBUG_RTCS_TICK_POS (DBG_6)	// B10
#define DEBUG_RTCS_IDLE_POS (DBG_7)	// B11

#define SWITCH_PRESSED(x) (!(PTD->PDIR & (MASK(x))))

#define W_DELAY_SLOW 10
#define RGB_DELAY_SLOW 60
#define W_DELAY_FAST 3
#define RGB_DELAY_FAST 30

#endif
