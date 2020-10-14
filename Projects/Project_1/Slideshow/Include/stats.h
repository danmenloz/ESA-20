#ifndef STATS_H
#define STATS_H
#include <stdint.h>

#define IDLE_LOOPS_PER_MS (6000) // Change to 10000000

extern volatile uint32_t	blocks_read;
extern volatile uint32_t idle_counter;

void Stats_Start(void);
void Stats_Freeze(void);
void Stats_Display(void);


	
#endif // STATS_H
