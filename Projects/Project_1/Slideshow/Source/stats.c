#include "stats.h"
#include "LCD.h"
#include "font.h"
#include "cmsis_os2.h"
#include "itoa.h"

#define IDLE_LOOPS_PER_MS (6000) // Remove

volatile uint32_t blocks_read=0;
volatile uint32_t idle_counter=0; 
uint32_t idle_counter_s;
uint32_t ticks_1, ticks_2;

void Stats_Start(void) {
	blocks_read=0;
	idle_counter_s = idle_counter;
	ticks_1 = osKernelGetTickCount();
}


void Stats_Freeze(void){
	ticks_2 = osKernelGetTickCount() - ticks_1;
	idle_counter_s = idle_counter - idle_counter_s;
}

void Stats_Display(void) {
	uint32_t r = 0;
	char buffer[16];
	
	LCD_Erase();
	LCD_Text_PrintStr_RC(r++, 0, "Blocks read");
	LCD_Text_PrintStr_RC(r++, 0, "Total time");
	LCD_Text_PrintStr_RC(r++, 0, "Idle loops");
	LCD_Text_PrintStr_RC(r++, 0, "Idle time");

	r = 0;
	itoa(blocks_read, buffer);
	LCD_Text_PrintStr_RC(r++, 12, buffer);
	itoa(ticks_2, buffer);
	LCD_Text_PrintStr_RC(r++, 12, buffer);
	itoa(idle_counter_s, buffer);
	LCD_Text_PrintStr_RC(r++, 12, buffer);
	itoa(idle_counter_s/IDLE_LOOPS_PER_MS, buffer);
	LCD_Text_PrintStr_RC(r++, 12, buffer);
	
	// LCD_TS_Blocking_Read(NULL); // Uncomment to keep data on screen until pressed
}
