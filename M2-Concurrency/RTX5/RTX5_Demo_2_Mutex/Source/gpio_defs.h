#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#define SW1_POS (7)		// on port D
#define SW2_POS (6)		// on port D

#define MASK(x) (1UL << (x))

// Debug Signals on port B
#define DBG_1 1
#define DBG_2 2
#define DBG_3 3
#define DBG_4 8
#define DBG_5 9
#define DBG_6 10
#define DBG_7 11


#define SWITCH_PRESSED(x) (!(PTD->PDIR & (MASK(x))))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)		// on port B
#define BLUE_LED_POS (1)		// on port D

#define ON_TIME (750)
#define FLASH_TIME (100)

#endif
// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
