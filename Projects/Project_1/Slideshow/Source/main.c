/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "gpio_defs.h"

#include "LCD.h"
#include "font.h"

#include "LEDs.h"
#include "timers.h"
#include "UART.h"
#include "debug.h"
#include "stats.h"

#include "pff.h"

#include "I2C.h"
#include "mma8451.h"
#include "delay.h"
#include "profile.h"
#include "math.h"

extern int LCD_JPEG(void);

osThreadId_t tid_SlideShow;

#define TEST_PREC_DELAY_ECE560 (1)

#if TEST_PREC_DELAY_ECE560
osThreadId_t tid_T1, tid_T2, tid_T3, tid_T4;

void T1(void * argument) {
	//virtual_PIT_Init(0, 239, osThreadGetId(), 1);
	//virtual_PIT_Start(0);
	while (1) {
		DEBUG_START(DBG_1);
		
		// Add code to perform a precision delay of 28 microseconds here
		//virtual_PIT_Init(0,  23999, osThreadGetId(), 1);
		PIT_Init( 671, osThreadGetId());
		PIT_Start();// Start called here
		uint32_t result = osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever); //wait for flag and clear it
		
		DEBUG_STOP(DBG_1);
		osDelay(10); // Wait 10 ms before testing timer channel again
	}
}

void T2(void * argument) {
	while (1) {
		DEBUG_START(DBG_2);
		// Add code to perform a precision delay of 10 microseconds here
		virtual_PIT_Init(1, 11999, osThreadGetId(), 1);
		virtual_PIT_Start(1);
		uint32_t result = osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
		DEBUG_STOP(DBG_2);
		osDelay(10); // Wait 10 ms before testing timer channel again
	}
}

void T3(void * argument) {
	while (1) {
		DEBUG_START(DBG_3);
		// Add code to perform a precision delay of 86 microseconds here
		virtual_PIT_Init(2, 7199999, osThreadGetId(), 1);
		virtual_PIT_Start(2);
		uint32_t result = osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		
		DEBUG_STOP(DBG_3);
		osDelay(10); // Wait 10 ms before testing timer channel again
	}
}

void T4(void * argument) {
	while (1) {
		DEBUG_START(DBG_4);
		// Add code to perform a precision delay of 179 microseconds here
		virtual_PIT_Init(3, 9599999, osThreadGetId(), 1);
		virtual_PIT_Start(3);
		uint32_t result = osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		DEBUG_STOP(DBG_4);
		osDelay(10); // Wait 10 ms before testing timer channel again
	}
}

#else

const osThreadAttr_t SlideShow_attr = {
	.stack_size = 1024
};

void Thread_Slideshow(void * argument) {
	FATFS fatfs;
	FRESULT rc;
	int error;
	char buf[16];
	
	LCD_Text_PrintStr_RC(1, 0, "Looking for uSD card");
	Control_RGB_LEDs(1, 1, 0); // Yellow: trying pf_mount
	rc = pf_mount(&fatfs);
	if (rc) {
		LCD_Text_PrintStr_RC(2, 0, "pf_mount failed :(");
		while (1){ // Flashing Yellow: pf_mount failed
			Control_RGB_LEDs(1, 1, 0); 
			osDelay(100);
			Control_RGB_LEDs(0, 0, 0); 
			osDelay(100);
		};
	}
	Control_RGB_LEDs(0, 0, 0); // Yellow: trying pf_mount
	
	LCD_Text_PrintStr_RC(2, 0, "Mounted uSD card");

	while (1) {
		//osDelay(5);
		Stats_Start();
		Control_RGB_LEDs(0, 0, 1);	// Blue: Read/Decode/Display 
		error = LCD_JPEG();
		Stats_Freeze();
		if (error != 0) {		
			Control_RGB_LEDs(1, 0, 0);	// Red: LCD_JPEG failure
			strcpy(buf, "LCD_JPEG_Failure :(");
			//			sprintf(buf, "LCD_JPEG failure %d", error);
			LCD_Text_PrintStr_RC(6, 0, buf); 
			while (1) {		// Stop here forever, but yield CPU
				osDelay(1000);
			}
		} else {
			Control_RGB_LEDs(0, 1, 0);	// Green: LCD_JPEG success
			Stats_Display();
			osDelay(4000);
		}
	}
}
#endif

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main(void) {
	Init_RGB_LEDs();
	Control_RGB_LEDs(1, 1, 1); // White: start-up and initialization
	Init_UART0(115200);
	// printf("Hello world!\n\r"); // Removed to reduce code size (printf is big)
	Init_Debug_Signals();

	LCD_Init();
	LCD_Text_Init(1);
	LCD_Erase();
	LCD_Text_PrintStr_RC(0, 0, "Initializing");

	osKernelInitialize();
#if TEST_PREC_DELAY_ECE560
	osThreadNew(T1, NULL, NULL);
	//osThreadNew(T2, NULL, NULL);
	//osThreadNew(T3, NULL, NULL);
	//osThreadNew(T4, NULL, NULL);
#else
	osThreadNew(Thread_Slideshow, NULL, &SlideShow_attr);
#endif
	osKernelStart();
}

#if 0  // File system test code, not used
void die(												/* Stop with dying message */
					FRESULT rc						/* FatFs return value */
		) {
//  printf("Failed with rc=%u.\n", rc);
	Control_RGB_LEDs(1, 0, 0);
	for (;;);
}

void PFF_Test(void) {
	FATFS fatfs;									/* File system object */
	DIR dir;											/* Directory object */
	FILINFO fno;									/* File information object */
	UINT br, i;
	BYTE buff[64];
	FRESULT rc;
	PT_T p;
	uint32_t row = 0;
	char tbuff[64];

	LCD_Text_PrintStr_RC(row++, 0, "Mounting file system");
	rc = pf_mount(&fatfs);
	if (rc)
		die(rc);

#if PF_USE_DIR
	LCD_Text_PrintStr_RC(row++, 0, "Opening root directory");

	rc = pf_opendir(&dir, "");
	if (rc)
		die(rc);

	for (;;) {
		rc = pf_readdir(&dir, &fno);	/* Read a directory item */
		if (rc || !fno.fname[0])
			break;										/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
			sprintf(tbuff, "   <dir>  %s", fno.fname);
		else
			sprintf(tbuff, "%8u  %s", fno.fsize, fno.fname);
		LCD_Text_PrintStr_RC(row++, 0, tbuff);
	}
	if (rc)
		die(rc);
#endif


	while (1);

#if 0
	rc = pf_open("PFF.c");
	if (rc)
		die(rc);

//  printf("\nType the file content.\n");
	for (;;) {
		rc = pf_read(buff, sizeof(buff), &br);	/* Read a chunk of file */
		if (rc || !br)
			break;										/* Error or end of file */
//    for (i = 0; i < br; i++)    /* Type the data */
//      putchar(buff[i]);
	}
	if (rc)
		die(rc);
#endif

#if PF_USE_WRITE
	printf("\nOpen a file to write (write.txt).\n");
	rc = pf_open("WRITE.TXT");
	if (rc)
		die(rc);

	printf("\nWrite a text data. (Hello world!)\n");
	for (;;) {
		rc = pf_write("Hello world!\r\n", 14, &bw);
		if (rc || !bw)
			break;
	}
	if (rc)
		die(rc);

	printf("\nTerminate the file write process.\n");
	rc = pf_write(0, 0, &bw);
	if (rc)
		die(rc);
#endif

	Control_RGB_LEDs(0, 1, 0);
//  printf("\nTest completed.\n");
	for (;;);
}
#endif
