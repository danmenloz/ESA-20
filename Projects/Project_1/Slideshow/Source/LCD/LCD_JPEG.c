//------------------------------------------------------------------------------
// jpg2tga.c
// JPEG to TGA file conversion example program.
// Public domain, Rich Geldreich <richgel99@gmail.com>
// Last updated Nov. 26, 2010
//------------------------------------------------------------------------------
#include "picojpeg.h"
#include "pff.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "cmsis_os2.h"

#include "LCD.h"
#include "LCD_driver.h"
#include "ST7789.h"

//------------------------------------------------------------------------------
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
//------------------------------------------------------------------------------
typedef unsigned char uint8;
typedef unsigned int uint;
//------------------------------------------------------------------------------
static uint g_nInFileOfs;
char *g_pInData;
//------------------------------------------------------------------------------
unsigned char pjpeg_need_bytes_callback(unsigned char *pBuf,
																				unsigned char buf_size,
																				unsigned char
																				*pBytes_actually_read,
																				void *pCallback_data) {
	FRESULT rc;
	UINT br;

// 	pCallback_data;
	rc = pf_read(pBuf, buf_size, &br);
	if (rc != FR_OK) {
		return PJPG_STREAM_READ_ERROR;
	}

	*pBytes_actually_read = (unsigned char) (br);
	g_nInFileOfs += br;

	return 0;
}

//------------------------------------------------------------------------------
// Decode JPEG image from memory and render on LCD. 
// Changed to return 0 on success, error code on failure.
// If reduce is non-zero, the image will be more quickly decoded at approximately
// 1/8 resolution (the actual returned resolution will depend on the JPEG 
// subsampling factor).
int pjpeg_load_from_file(const
														char
														*pFilename, int
														*x, int
														*y, int
														*comps,
														pjpeg_scan_type_t
														* pScan_type, int reduce) {
	pjpeg_image_info_t image_info;
	int mcu_x = 0;
	int mcu_y = 0;
	uint row_pitch;
	uint8 *pImage;
	uint8 status;
	uint decoded_width, decoded_height;
	uint row_blocks_per_mcu, col_blocks_per_mcu;
															
	*x = 0;
	*y = 0;
	*comps = 0;
	if (pScan_type)
		*pScan_type = PJPG_GRAYSCALE;

	if (FR_OK != pf_open(pFilename)) {
		return PJPG_FILE_OPEN_FAILED;
	}
	g_nInFileOfs = 0;
	// Should get the real file size with pf_readdir
	status =
			pjpeg_decode_init
			(&image_info,
			 pjpeg_need_bytes_callback, NULL, (unsigned char) reduce);
	if (status) {
		return status;
	}

	if (pScan_type)
		*pScan_type = image_info.m_scanType;
	// In reduce mode output 1 pixel per 8x8 block.
	decoded_width =
			reduce ? (image_info.m_MCUSPerRow *
								image_info.m_MCUWidth) / 8 : image_info.m_width;
	
	// Don't try to display larger images
	if (decoded_width > LCD_WIDTH)
		return PJPG_BAD_WIDTH;

	decoded_height =
			reduce ? (image_info.m_MCUSPerCol *
								image_info.m_MCUHeight) / 8 : image_info.m_height;
	if (decoded_height > LCD_HEIGHT)
		return PJPG_BAD_HEIGHT;
	
	row_pitch = decoded_width * image_info.m_comps;
	pImage = (uint8 *) malloc(row_pitch);	// Do one row at a time, reset at end of line
	if (!pImage) {
		return PJPG_NOTENOUGHMEM;
	}

	if (reduce) {
	row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
	col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
	}
	
	for (;;) {
		int y, x;
		uint8 *pDst_row;
		status = pjpeg_decode_mcu();
		if (status) {
			if (status != PJPG_NO_MORE_BLOCKS) {
				// printf("pjpeg_decode_mcu() failed with status %u\n", status);
				free(pImage);
				return status;
			}
			break;
		}

		if (mcu_y >= image_info.m_MCUSPerCol) {
			free(pImage);
			return PJPG_TOO_MANY_BLOCKS;
		}

		if (reduce) {
#if 0
			// In reduce mode, only the first pixel of each 8x8 block is valid.
			pDst_row =
					pImage +
					mcu_y *
					col_blocks_per_mcu *
					row_pitch + mcu_x * row_blocks_per_mcu * image_info.m_comps;
			if (image_info.m_scanType == PJPG_GRAYSCALE) {
				*pDst_row = image_info.m_pMCUBufR[0];
			} else {
				uint y, x;
				for (y = 0; y < col_blocks_per_mcu; y++) {
					uint src_ofs = (y * 128U);
					for (x = 0; x < row_blocks_per_mcu; x++) {
						pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
						pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
						pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
						pDst_row += 3;
						src_ofs += 64;
					}
					pDst_row += row_pitch - 3 * row_blocks_per_mcu;
				}
			}
#endif 
		} else {
			// Copy MCU's pixel blocks into the destination bitmap.
			PT_T pos;
			COLOR_T color;
			pDst_row =
					pImage +
					(mcu_x * image_info.m_MCUWidth * image_info.m_comps);
			for (y = 0; y < image_info.m_MCUHeight; y += 8) {
				const int by_limit = min(8,
																 image_info.m_height -
																 (mcu_y *
																	image_info.m_MCUHeight + y));
				for (x = 0; x < image_info.m_MCUWidth; x += 8) {
					uint8 *pDst_block = pDst_row + x * image_info.m_comps;
					// Compute source byte offset of the block in the decoder's MCU buffer.
					uint src_ofs = (x * 8U) + (y * 16U);
					const uint8 *pSrcR = image_info.m_pMCUBufR + src_ofs;
					const uint8 *pSrcG = image_info.m_pMCUBufG + src_ofs;
					const uint8 *pSrcB = image_info.m_pMCUBufB + src_ofs;
					const int bx_limit = min(8,
																	 image_info.m_width -
																	 (mcu_x *
																		image_info.m_MCUWidth + x));
					if (image_info.m_scanType == PJPG_GRAYSCALE) {
						// Grayscale image
						int bx, by;
						pos.Y = y + mcu_y * image_info.m_MCUHeight;
						for (by = 0; by < by_limit; by++) {
							uint8 *pDst = pDst_block;
							pos.X = x + mcu_x * image_info.m_MCUWidth;
							for (bx = 0; bx < bx_limit; bx++) {
								color.R = color.G = color.B = *pSrcR;
								LCD_Plot_Pixel(&pos, &color);
								pos.X++;
								*pDst++ = *pSrcR++;
							}
							pSrcR += (8 - bx_limit);
							pDst_block += row_pitch;
							pos.Y++;
						}
					} else {
						// Color image
						pos.Y = y + mcu_y * image_info.m_MCUHeight;
						pos.X = x + mcu_x * image_info.m_MCUWidth;
						PT_T ul, lr;
						ul.Y = pos.Y;
						ul.X = pos.X;				// x + mcu_x * image_info.m_MCUWidth;
						lr.Y = pos.Y + by_limit;
						lr.X = pos.X + bx_limit - 1;	//x + mcu_x * image_info.m_MCUWidth + bx_limit-1;
						LCD_Start_Rectangle(&ul, &lr);
						LCD_Write_Rectangle_N_Quad_Pixel_Components((uint32_t *) pSrcR,
																												(uint32_t *) pSrcG,
																												(uint32_t *)pSrcB,
																												(bx_limit * by_limit) >> 2);
					}
				}
			}
		}
		mcu_x++;
		if (mcu_x == image_info.m_MCUSPerRow) {
			mcu_x = 0;
			mcu_y++;
		}
	}
	*x = decoded_width;
	*y = decoded_height;
	*comps = image_info.m_comps;
	free(pImage);
	return 0;  // Success!
}

//------------------------------------------------------------------------------
int LCD_JPEG(void) {
	int width, height, comps;
	pjpeg_scan_type_t scan_type;
	int reduce = 0;
	DIR dir;
	FILINFO fno;
	FRESULT rc;
	int result;
	int image_count;

		pixel_data_hash = 0;
		rc = pf_opendir(&dir, "");
		image_count = 0;
		if (rc == FR_OK) {
			for (;;) {
				rc = pf_readdir(&dir, &fno);	/* Read a directory item */
				if (rc || !fno.fname[0])
					break;								/* Error or end of dir */
				if (!(fno.fattrib & AM_DIR)) {
					result= pjpeg_load_from_file
							(fno.fname, &width, &height, &comps, &scan_type,
							 reduce);
					if (result != 0) {
					LCD_Text_PrintStr_RC(4, 0, "pjpeg_load_from_file ");
					LCD_Text_PrintStr_RC(5, 0, "failed :(");
					osDelay(2000);
					return 1;
					} else {
						image_count++;
					}
				}
			}
			if (image_count == 0) {
			LCD_Text_PrintStr_RC(4, 0, "No JPEGs Found! :(");				
			osDelay(2000);
			return 1;
			}
		} else {
		LCD_Text_PrintStr_RC(4, 0, "Couldn't open");
		LCD_Text_PrintStr_RC(5, 0, "root directory :(");
		osDelay(2000);
		return 1;
		}
#if ENABLE_PIXEL_HASH
		if (pixel_data_hash != 0x7D215B3D) {
			LCD_Text_PrintStr_RC(0, 0, "Pixel hash failed");
			while (1)
				;
		}
#endif

	return 0;
}

//------------------------------------------------------------------------------
