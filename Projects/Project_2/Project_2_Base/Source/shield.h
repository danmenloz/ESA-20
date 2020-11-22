#include <MKL25Z4.H>
#include "fault.h"
#include <cmsis_os2.h>
#include "control.h"
#include "LEDs.h"
#include "threads.h"

// Shared global variables
extern volatile int g_set_current; // Default starting LED current

void Write_Set_Current(int new_value);
int Read_Set_Current(void);
