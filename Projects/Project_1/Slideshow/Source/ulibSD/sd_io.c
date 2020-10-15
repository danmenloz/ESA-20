/*
 *  File: sd_io.c
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */

// Modified 2017 by Alex Dean (agdean@ncsu.edu) for teaching FSMs
// - Removed support for PC development (_M_IX86)
// - Split single-line loops & conditionals in source code for readability
// - Fused loops in SD_Read 
// _ Inlined __SD_Write_Block into SD_Write

#include "sd_io.h"
#include <MKL25Z4.h>
#include "debug.h"
#include "delay.h"
#include "cmsis_os2.h"
#include <string.h>
#include "timers.h"
#include "stats.h"

uint32_t timeout = 0;
uint32_t UseDMAForSDRead = 0;

/* Results of SD functions */
char SD_Errors[7][8] = {
	"OK",
	"NOINIT",											/* 1: SD not initialized    */
	"ERROR",											/* 2: Disk error            */
	"PARERR",											/* 3: Invalid parameter     */
	"BUSY",												/* 4: Programming busy      */
	"REJECT",											/* 5: Reject data           */
	"NORESP"											/* 6: No response           */
};

/******************************************************************************
 Private Methods Prototypes - Direct work with SD card
******************************************************************************/

/**
    \brief Simple function to calculate power of two.
    \param e Exponent.
    \return Math function result.
*/
DWORD __SD_Power_Of_Two(BYTE e);

/**
     \brief Assert the SD card (SPI CS low).
 */
inline void __SD_Assert(void);

/**
    \brief Deassert the SD (SPI CS high).
 */
inline void __SD_Deassert(void);

/**
    \brief Change to max the speed transfer.
    \param throttle
 */
void __SD_Speed_Transfer(BYTE throttle);

/**
    \brief Send SPI commands.
    \param cmd Command to send.
    \param arg Argument to send.
    \return R1 response.
 */
BYTE __SD_Send_Cmd(BYTE cmd, DWORD arg);

/**
    \brief Get the total numbers of sectors in SD card.
    \param dev Device descriptor.
    \return Quantity of sectors. Zero if fail.
 */
DWORD __SD_Sectors(SD_DEV * dev);

/*****************************************************************************
 Private Methods - Direct work with SD card
******************************************************************************/

DWORD __SD_Power_Of_Two(BYTE e) {
	DWORD partial = 1;
	BYTE idx;
	for (idx = 0; idx != e; idx++)
		partial *= 2;
	return (partial);
}

inline void __SD_Assert(void) {
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	__nop();
	SPI_CS_Low();
}

inline void __SD_Deassert(void) {
	SPI_CS_High();
}

void __SD_Speed_Transfer(BYTE throttle) {
	if (throttle == HIGH)
		SPI_Freq_High();
	else
		SPI_Freq_Low();
}

BYTE __SD_Send_Cmd(BYTE cmd, DWORD arg) {
	BYTE crc, res;

	DEBUG_START(DBG_4);

	// ACMD«n» is the command sequense of CMD55-CMD«n»
	if (cmd & 0x80) {
		cmd &= 0x7F;
		res = __SD_Send_Cmd(CMD55, 0);
		if (res > 1) {
			DEBUG_STOP(DBG_4);
			return (res);

		}
	}
	// Select the card
	__SD_Deassert();
	SPI_RW(0xFF);
	__SD_Assert();
	SPI_RW(0xFF);

	// Send complete command set
	SPI_RW(cmd);									// Start and command index
	SPI_RW((BYTE) (arg >> 24));		// Arg[31-24]
	SPI_RW((BYTE) (arg >> 16));		// Arg[23-16]
	SPI_RW((BYTE) (arg >> 8));		// Arg[15-08]
	SPI_RW((BYTE) (arg >> 0));		// Arg[07-00]

	// CRC?
	crc = 0x01;										// Dummy CRC and stop
	if (cmd == CMD0)
		crc = 0x95;									// Valid CRC for CMD0(0)
	if (cmd == CMD8)
		crc = 0x87;									// Valid CRC for CMD8(0x1AA)
	SPI_RW(crc);

	// Receive command response
	// Wait for a valid response in timeout of 5 milliseconds
	SPI_Timer_On(5);
	do {
		DEBUG_TOGGLE(DBG_4);
		res = SPI_RW(0xFF);
	} while ((res & 0x80) && (SPI_Timer_Status() == TRUE));
	DEBUG_START(DBG_4);
	while (SPI_Timer_Status() == FALSE);
	SPI_Timer_Off();

	// Return with the response value
	DEBUG_STOP(DBG_4)
			return (res);
}

DWORD __SD_Sectors(SD_DEV * dev) {
	BYTE csd[16];
	BYTE idx;
	DWORD ss = 0;
	WORD C_SIZE = 0;
	BYTE C_SIZE_MULT = 0;
	BYTE READ_BL_LEN = 0;
	if (__SD_Send_Cmd(CMD9, 0) == 0) {
		// Wait for response
		while (SPI_RW(0xFF) == 0xFF);
		for (idx = 0; idx != 16; idx++)
			csd[idx] = SPI_RW(0xFF);
		// Dummy CRC
		SPI_RW(0xFF);
		SPI_RW(0xFF);
		SPI_Release();
		if (dev->cardtype & SDCT_SD1) {
			ss = csd[0];
			// READ_BL_LEN[83:80]: max. read data block length
			READ_BL_LEN = (csd[5] & 0x0F);
			// C_SIZE [73:62]
			C_SIZE = (csd[6] & 0x03);
			C_SIZE <<= 8;
			C_SIZE |= (csd[7]);
			C_SIZE <<= 2;
			C_SIZE |= ((csd[8] >> 6) & 0x03);
			// C_SIZE_MULT [49:47]
			C_SIZE_MULT = (csd[9] & 0x03);
			C_SIZE_MULT <<= 1;
			C_SIZE_MULT |= ((csd[10] >> 7) & 0x01);
		} else if (dev->cardtype & SDCT_SD2) {
			// READ_BL_LEN = 9;
			// C_SIZE [69:48]
			C_SIZE = (csd[7] & 0x3F);
			C_SIZE <<= 8;
			C_SIZE |= (csd[8] & 0xFF);
			C_SIZE <<= 8;
			C_SIZE |= (csd[9] & 0xFF);
			C_SIZE_MULT = 8;					// AD changed
		}
		ss = (C_SIZE + 1);
		ss *= __SD_Power_Of_Two(C_SIZE_MULT + 2);
		ss *= __SD_Power_Of_Two(READ_BL_LEN);
		// ss /= SD_BLK_SIZE; ?? Bug in original code?

		return (ss);
	} else
		return (0);									// Error
}

/******************************************************************************
 Public Methods - Direct work with SD card
******************************************************************************/

SDRESULTS SD_Init(SD_DEV * dev) {
	BYTE n, cmd, ct, ocr[4];
	BYTE idx;
	BYTE init_trys;

	DEBUG_START(DBG_5);
	ct = 0;
	for (init_trys = 0; ((init_trys != SD_INIT_TRYS) && (!ct));
			 init_trys++) {
		// Initialize SPI for use with the memory card
		SPI_Init();

		SPI_CS_High();
		SPI_Freq_Low();

		// 80 dummy clocks
		for (idx = 0; idx != 10; idx++)
			SPI_RW(0xFF);

		SPI_Timer_On(500);
		while (SPI_Timer_Status() == TRUE) {
			DEBUG_TOGGLE(DBG_5)
		}
		DEBUG_START(DBG_5);
		SPI_Timer_Off();

		dev->mount = FALSE;
		SPI_Timer_On(500);
		while ((__SD_Send_Cmd(CMD0, 0) != 1)
					 && (SPI_Timer_Status() == TRUE)) {
			DEBUG_TOGGLE(DBG_5);
		}
		DEBUG_START(DBG_5);
		SPI_Timer_Off();
		// Idle state
		if (__SD_Send_Cmd(CMD0, 0) == 1) {
			// SD version 2?
			if (__SD_Send_Cmd(CMD8, 0x1AA) == 1) {
				// Get trailing return value of R7 resp
				for (n = 0; n < 4; n++)
					ocr[n] = SPI_RW(0xFF);
				// VDD range of 2.7-3.6V is OK?  
				if ((ocr[2] == 0x01) && (ocr[3] == 0xAA)) {
					// Wait for leaving idle state (ACMD41 with HCS bit)...
					SPI_Timer_On(1000);
					while ((SPI_Timer_Status() == TRUE)
								 && (__SD_Send_Cmd(ACMD41, 1UL << 30))) {
						DEBUG_TOGGLE(DBG_5);
					}
					DEBUG_START(DBG_5);
					SPI_Timer_Off();
					// CCS in the OCR? 
					if ((SPI_Timer_Status() == TRUE)
							&& (__SD_Send_Cmd(CMD58, 0) == 0)) {
						for (n = 0; n < 4; n++)
							ocr[n] = SPI_RW(0xFF);
						// SD version 2?
						ct = (ocr[0] & 0x40) ? SDCT_SD2 | SDCT_BLOCK : SDCT_SD2;
					}
				}
			} else {
				// SD version 1 or MMC?
				if (__SD_Send_Cmd(ACMD41, 0) <= 1) {
					// SD version 1
					ct = SDCT_SD1;
					cmd = ACMD41;
				} else {
					// MMC version 3
					ct = SDCT_MMC;
					cmd = CMD1;
				}
				// Wait for leaving idle state
				SPI_Timer_On(250);
				while ((SPI_Timer_Status() == TRUE)
							 && (__SD_Send_Cmd(cmd, 0))) {
					DEBUG_TOGGLE(DBG_5);
				}
				DEBUG_START(DBG_5);
				SPI_Timer_Off();
				if (SPI_Timer_Status() == FALSE)
					ct = 0;
				if (__SD_Send_Cmd(CMD59, 0))
					ct = 0;								// Deactivate CRC check (default)
				if (__SD_Send_Cmd(CMD16, 512))
					ct = 0;								// Set R/W block length to 512 bytes
			}
		}
	}
	if (ct) {
		dev->cardtype = ct;
		dev->mount = TRUE;
		dev->last_sector = __SD_Sectors(dev) - 1;
		dev->debug.read = 0;
		dev->debug.write = 0;
		__SD_Speed_Transfer(HIGH);	// High speed transfer
	}
	SPI_Release();
	DEBUG_STOP(DBG_5);
	return (ct ? SD_OK : SD_NOINIT);
}

#define SD_BUF_SIZE (SD_BLK_SIZE + 2)  // +2 for CRC
// #define USE_DMA_FOR_SD_READ
SDRESULTS SD_Read(SD_DEV * dev, void *dat, DWORD sector, WORD ofs,
									WORD cnt) {
	
	SDRESULTS res;
	BYTE tkn = 0;
	DEBUG_START(DBG_2);
	static uint16_t lastSectorRead = UINT16_MAX;
	static uint8_t sdBuf[SD_BUF_SIZE] = {0};
				
	res = SD_ERROR;
	if ((sector > dev->last_sector) || (cnt == 0)) {
		DEBUG_STOP(DBG_2);
		return (SD_PARERR);
	}

	if (sector != lastSectorRead) // If we haven't already read this sector, read it
	{
		blocks_read++;
		if (__SD_Send_Cmd(CMD17, sector) == 0) {	// Only for SDHC or SDXC 
			DEBUG_START(DBG_6);
			osDelay(2);
			SPI_Timer_On(100);
			do {
				DEBUG_TOGGLE(DBG_2);
				tkn = SPI_RW(0xFF);
			} while ((tkn == 0xFF) && SPI_Timer_Status() == TRUE);
			DEBUG_STOP(DBG_6);
			DEBUG_START(DBG_2);
			while (SPI_Timer_Status() == FALSE)
				;  // Trap here on timeout error
			SPI_Timer_Off();
			// Token of single block?
			if (tkn == 0xFE) {
				// Copy the entire sector into buffer
				if (UseDMAForSDRead)
					DMA_OpenChannelsForSpi(sdBuf, SD_BUF_SIZE);
				else {
					for (int byte_num = 0; byte_num < SD_BLK_SIZE + 2; byte_num++)
					{
						DEBUG_TOGGLE(DBG_2); 
						sdBuf[byte_num] = SPI_RW(0xff);
					}		
				}
				DEBUG_START(DBG_2);
				lastSectorRead = sector;
				res = SD_OK;
			} else { // Token is not 0xFE
				while (1)
					; // Trap here on error
			}
		} else { // Didn't get 0 back from __SD_Send_Cmd (17)
			while (1)
				; // Trap here on error
		}
		SPI_Release();
		dev->debug.read++;
		DEBUG_STOP(DBG_2);
   } else { // We just read this sector last time, so don't redo it
	 	 res = SD_OK;
	 }
	
	 // Copy result out of the buffer. There's room for 
	 // a bit more optimization here, but this is still better
	 // than before
	 uint8_t* datCpy = dat;
	 if (ofs+cnt >= SD_BUF_SIZE)
	 {
		 cnt = SD_BUF_SIZE - ofs; 
	 }
	 memcpy(datCpy, &sdBuf[ofs], cnt);


	 return (res);
}

SDRESULTS SD_Write(SD_DEV * dev, void *dat, DWORD sector) {
	WORD idx;
	BYTE line;

	DEBUG_START(DBG_3);

	// Query invalid?
	if (sector > dev->last_sector) {
		DEBUG_STOP(DBG_3);
		return (SD_PARERR);
	}
	// Convert sector number to bytes address (sector * SD_BLK_SIZE)
	//    if(__SD_Send_Cmd(CMD24, sector * SD_BLK_SIZE)==0) { // Only for SDSC
	if (__SD_Send_Cmd(CMD24, sector) == 0) {	// Only for SDHC or SDXC   
		// Send token (single block write)
		SPI_RW(0xFE);
		// Send block data
		for (idx = 0; idx != SD_BLK_SIZE; idx++)
			SPI_RW(*((BYTE *) dat + idx));
		/* Dummy CRC */
		SPI_RW(0xFF);
		SPI_RW(0xFF);
		// If not accepted, returns the reject error
		if ((SPI_RW(0xFF) & 0x1F) != 0x05) {
			DEBUG_STOP(DBG_3);
			return (SD_REJECT);
		}
		// Waits until finish of data programming with a timeout
		SPI_Timer_On(SD_IO_WRITE_TIMEOUT_WAIT);
		do {
			DEBUG_TOGGLE(DBG_3);
			line = SPI_RW(0xFF);
		} while ((line == 0) && (SPI_Timer_Status() == TRUE));
		DEBUG_START(DBG_3);
		SPI_Timer_Off();
		dev->debug.write++;

		if (line == 0) {
			DEBUG_STOP(DBG_3);
			return (SD_BUSY);
		} else {
			DEBUG_STOP(DBG_3);
			return (SD_OK);
		}
	} else {
		DEBUG_STOP(DBG_3);
		return (SD_ERROR);
	}
}

SDRESULTS SD_Status(SD_DEV * dev) {
	return (__SD_Send_Cmd(CMD0, 0) ? SD_OK : SD_NORESPONSE);
}

// «sd_io.c» is part of:
/*----------------------------------------------------------------------------/
/  ulibSD - Library for SD cards semantics            (C)Nelson Lombardo, 2015
/-----------------------------------------------------------------------------/
/ ulibSD library is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/

// Derived from Mister Chan works on FatFs code (http://elm-chan.org/fsw/ff/00index_e.html):
/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.11                 (C)ChaN, 2015
/-----------------------------------------------------------------------------/
/ FatFs module is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/----------------------------------------------------------------------------*/
