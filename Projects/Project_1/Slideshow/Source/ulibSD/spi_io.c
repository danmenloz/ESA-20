/*
 *  File: spi_io.c.example
 *  Author: Nelson Lombardo
 *  Year: 2015
 *  e-mail: nelson.lombardo@gmail.com
 *  License at the end of file.
 */
 // Modified 2017 by Alex Dean (agdean@ncsu.edu) for teaching FSMs and general use


#include "spi_io.h"
#include <MKL25Z4.h>
#include "debug.h"

/******************************************************************************
 Module Public Functions - Low level SPI control functions
******************************************************************************/

BYTE msgReceive;

void SPI_Init(void) {

	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	/*
	 *SPI1 Clock gate control. 1 clock enabled
	 */
	SIM_SCGC4 |= SIM_SCGC4_SPI1_MASK;
	/*
	 * Multiplexing pins
	 */
	PORTE_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK & (~PORT_PCR_SRE_MASK);	//CS
	GPIOE_PDDR |= 1 << 4;					// Pin is configured as general-purpose output, for the GPIO function.

	PORTE_PCR2 = PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK & (~PORT_PCR_SRE_MASK);	// SCK
	PORTE_PCR1 = PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK & (~PORT_PCR_SRE_MASK);	// MOSI
	PORTE_PCR3 = PORT_PCR_MUX(2) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	// MISO

	/*
	 * Bit 7 SPIE   = 0 Disables receive and mode fault interrupts
	 * Bit 6 SPE    = 1 Enables the SPI system
	 * Bit 5 SPTIE  = 0 Disables SPI transmit interrupts
	 * Bit 4 MSTR   = 1 Sets the SPI module as a master SPI device
	 * Bit 3 CPOL   = 0 Configures SPI clock as active-high
	 * Bit 2 CPHA   = 0 First edge on SPSCK at start of first data transfer cycle
	 * Bit 1 SSOE   = 1 Determines SS pin function when mode fault enabled
	 * Bit 0 LSBFE  = 0 SPI serial data transfers start with most significant bit
	 */
	SPI1_C1 = 0xD0;
	/*
	 * Bit 7 PMIE       = 0 SPI hardware match interrupt disabled
	 * Bit 6            = 0 Unimplemented
	 * Bit 5 TXDMAE     = 0 DMA request disabled
	 * Bit 4 MODFEN     = 1 In master mode, ~SS pin function is automatic ~SS output
	 * Bit 3 BIDIROE    = 0 SPI data I/O pin acts as input
	 * Bit 2 RXDMAE     = 0 DMA request disabled
	 * Bit 1 SPISWAI    = 0 SPI clocks operate in wait mode
	 * Bit 0 SPC0       = 0 uses separate pins for data input and output
	 */
	SPI1_C2 = 0x00;

	/*
	 * Bit 7    SPRF    = 0 Flag is set when receive data buffer is full
	 * Bit 6    SPMF    = 0 Flag is set when SPIx_M = receive data buffer
	 * Bit 5    SPTEF   = 0 Flag is set when transmit data buffer is empty
	 * Bit 4    MODF    = 0 Mode fault flag for master mode
	 * Bit 3:0          = 0 Reserved
	 */
	SPI1_S = 0x00;	
}

static void DMA_CloseChannels()
{
  // Disable SPI DMA Requests
	SPI1->C2 &= ~SPI_C2_TXDMAE(1) & ~SPI_C2_RXDMAE(1); 

	// Disable clock to DMA
//	SIM->SCGC7 &= ~SIM_SCGC7_DMA_MASK;
  //SIM->SCGC6 &= ~SIM_SCGC6_DMAMUX_MASK;
}

void DMA_OpenChannelsForSpi(volatile void* destPtr, uint16_t numBytes)
{ 
	  // Dummy TX Buffer for transmits
    uint8_t txSrcBuf = 0xff;
	
	  // Enable clock
	  SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
 
	  // Reset MUX
	  DMAMUX0->CHCFG[RX_DMA] = 0;
	  DMAMUX0->CHCFG[TX_DMA] = 0;
	
	  // Reset done flags
    DMA0->DMA[RX_DMA].DSR_BCR = DMA_DSR_BCR_DONE_MASK;
    DMA0->DMA[TX_DMA].DSR_BCR = DMA_DSR_BCR_DONE_MASK;
	
	  // Set up source and destination registers (RX)
	  DMA0->DMA[RX_DMA].SAR = (uint32_t)&(SPI1_D); 
	  DMA0->DMA[RX_DMA].DAR = (uint32_t)destPtr;
	
		// Set up source and destination registers (TX)
	  DMA0->DMA[TX_DMA].SAR = (uint32_t)&txSrcBuf;
	  DMA0->DMA[TX_DMA].DAR = (uint32_t)&(SPI1_D); 
	
	  // Set up number of bytes to transfer
	  DMA0->DMA[RX_DMA].DSR_BCR |= DMA_DSR_BCR_BCR(numBytes);
		DMA0->DMA[TX_DMA].DSR_BCR |= DMA_DSR_BCR_BCR(numBytes);

	  // RX DCR
	  DMA0->DMA[RX_DMA].DCR = (
	    DMA_DCR_EINT(1)  |  // enable xfer complete interrupt
	    DMA_DCR_ERQ(1)   |  // enable peripheral request
	    DMA_DCR_CS(1)    |  // Cycle steal
	    DMA_DCR_AA(0)    |  // auto align disabled
	    DMA_DCR_SINC(0)  |  // do not increment source address
	    DMA_DCR_SSIZE(1) |  // 8 bit source data size
	    DMA_DCR_DINC(1)  |  // Increment destination address
	    DMA_DCR_DSIZE(1) |  // 8 bit destination data size
			DMA_DCR_D_REQ(0)) ; // ERQ bit is not cleared when the BCR is exhausted

	    // TX DCR
	  DMA0->DMA[TX_DMA].DCR = (
	    DMA_DCR_EINT(1)  |  // enable xfer complete interrupt
	    DMA_DCR_ERQ(1)   |  // enable peripheral request
	    DMA_DCR_CS(1)    |  // Cycle steal
	    DMA_DCR_AA(0)    |  // auto align disabled
	    DMA_DCR_SINC(0)  |  // do not increment source address
	    DMA_DCR_SSIZE(1) |  // 8 bit source data size
	    DMA_DCR_DINC(0)  |  // do not increment destination address
		  DMA_DCR_DSIZE(1) |  // 8 bit destination data size
			DMA_DCR_D_REQ(0));// | // ERQ bit is not cleared when the BCR is exhausted

			// Set up RX DMAMUX cfg
		DMAMUX0->CHCFG[RX_DMA] = 
			DMAMUX_CHCFG_ENBL(1) | // enable
			DMAMUX_CHCFG_TRIG(0) | // no periodic trigger
			DMAMUX_CHCFG_SOURCE(DMA_Source_SPI1_Rx); // Source
		
		// Set up TX DMAMUX cfg
		DMAMUX0->CHCFG[TX_DMA] = 
			DMAMUX_CHCFG_ENBL(1) | // enable
			DMAMUX_CHCFG_TRIG(0) | // no periodic trigger
			DMAMUX_CHCFG_SOURCE(DMA_Source_SPI1_Tx); // Source

		// Enable SPI/TX and receive by DMA
	  SPI1->C2 |= SPI_C2_TXDMAE(1) | SPI_C2_RXDMAE(1); 

	 // Wait for transfer to complete. We only care about RX.
		DEBUG_START(DBG_6);
	  while(!((DMA0->DMA[RX_DMA].DSR_BCR & DMA_DSR_BCR_DONE_MASK)));
		DEBUG_STOP(DBG_6);
		// Shut down DMA
		DMA_CloseChannels();	
}

inline BYTE SPI_RW(BYTE d) { 
	DEBUG_START(DBG_1);

	// Wait for previous transmission to complete
	while (!(SPI1_S & SPI_S_SPTEF_MASK)) {
		DEBUG_TOGGLE(DBG_1);
	}
	DEBUG_START(DBG_1);
	// Transmit the data
	SPI1_D = d;
	
	// block until TX complete
	while (!(SPI1_S & SPI_S_SPRF_MASK)) {
		DEBUG_TOGGLE(DBG_1);
	}
	DEBUG_STOP(DBG_1);
	
	// return data rx'ed
	return ((BYTE) (SPI1_D));
}

void SPI_Release(void) {
	WORD idx;
	for (idx = 512; idx && (SPI_RW(0xFF) != 0xFF); idx--);
}

inline void SPI_CS_Low(void) {
	GPIOE_PDOR &= ~(1 << 4);			//CS LOW
}

inline void SPI_CS_High(void) {
	GPIOE_PDOR |= (1 << 4);				//CS HIGH
}

inline void SPI_Freq_High(void) {
	//SPI1_BR = 0x05;
   SPI1_BR = 0x10; // 0x10;
}

inline void SPI_Freq_Low(void) {
	SPI1_BR = 0x44;			
}

void SPI_Timer_On(WORD ms) {
	SIM_SCGC5 |= SIM_SCGC5_LPTMR_MASK;	// Make sure clock is enabled
	LPTMR0_CSR = 0;								// Reset LPTMR settings
	LPTMR0_CMR = ms;							// Set compare value (in ms)
	// Use 1kHz LPO with no prescaler
	LPTMR0_PSR = LPTMR_PSR_PCS(1) | LPTMR_PSR_PBYP_MASK;
	// Start the timer and wait for it to reach the compare value
	LPTMR0_CSR = LPTMR_CSR_TEN_MASK;
}

inline BOOL SPI_Timer_Status(void) {
	return (!(LPTMR0_CSR & LPTMR_CSR_TCF_MASK) ? TRUE : FALSE);
}

inline void SPI_Timer_Off(void) {
	LPTMR0_CSR = 0;								// Turn off timer
}

#ifdef SPI_DEBUG_OSC
inline void SPI_Debug_Init(void) {
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;	// Port A enable
	PORTA_PCR12 = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	GPIOA_PDDR |= (1 << 12);			// Pin is configured as general-purpose output, for the GPIO function.
	GPIOA_PDOR &= ~(1 << 12);			// Off
}
inline void SPI_Debug_Mark(void) {
	GPIOA_PDOR |= (1 << 12);			// On
	GPIOA_PDOR &= ~(1 << 12);			// Off
}
#endif

/*
The MIT License (MIT)

Copyright (c) 2015 Nelson Lombardo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
