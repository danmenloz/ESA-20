/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "cmsis_os2.h"
#include "gpio_defs.h"

#include "LCD.h"
#include "font.h"

#include "LEDs.h"
#include "timers.h"
#include "UART.h"
#include "debug.h"

#include "pff.h"

#include "I2C.h"
#include "mma8451.h"
#include "delay.h"
#include "profile.h"
#include "math.h"

extern int LCD_JPEG(void);

osThreadId_t tid_SlideShow;

const osThreadAttr_t SlideShow_attr = {
	.stack_size = 1024
};

void die(												/* Stop with dying message */
					FRESULT rc						/* FatFs return value */
		) {
//  printf("Failed with rc=%u.\n", rc);
	Control_RGB_LEDs(1, 0, 0);
	for (;;);
}


#if 0
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

void Thread_Slideshow(void * argument) {
	FATFS fatfs;
	FRESULT rc;
	int error;
	char buf[16];
	
	LCD_Text_PrintStr_RC(1, 0, "Looking for uSD card");
	rc = pf_mount(&fatfs);
	if (rc) {
		LCD_Text_PrintStr_RC(2, 0, "pf_mount failed");
		Control_RGB_LEDs(1, 0, 0);
		while (1);
	}
	LCD_Text_PrintStr_RC(2, 0, "Mounted uSD card");

	while (1) {
		LCD_Erase();
		Control_RGB_LEDs(1, 0, 1);	// Magenta: running
		error = LCD_JPEG();
		Control_RGB_LEDs(error, 1-error, 0);	// Green: ok. Red: bad.
		if (error != 0) {
			sprintf(buf, "LCD_JPEG failure %d", error);
			LCD_Text_PrintStr_RC(0, 0, buf); 
			while (1)
				;
		}
	}
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main(void) {
	
	Init_RGB_LEDs();
	Init_UART0(115200);
	printf("Hello world!\n\r");

	Init_Debug_Signals();

	LCD_Init();
	LCD_Text_Init(1);
	LCD_Erase();
	LCD_Text_PrintStr_RC(0, 0, "Initializing");

	osKernelInitialize();
	osThreadNew(Thread_Slideshow, NULL, &SlideShow_attr);
	osKernelStart();
}

