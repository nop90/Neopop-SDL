/* $NiH: system_hq2x.c,v 1.2 2004/07/21 09:34:02 dillo Exp $ */
/* Adapted for NeoPop-SDL by Dieter Baron and Thomas Klausner */

/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Header: /cvsroot/scummvm/scummvm/common/scaler/hq2x.cpp,v 1.12 2004/05/21 17:30:51 aquadran Exp 
 *
 */

#include "config.h"
#include "NeoPop-SDL.h"
#include "system_hq_intern.h"

#ifdef USE_NASM
/* Assembly version of HQ2x */

#ifndef _WIN32
#define hq2x_16 _hq2x_16
#endif

void hq2x_16(const byte *, byte *, uint32, uint32, uint32, uint32);

void HQ2x(const Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height) {
	hq2x_16(srcPtr, dstPtr, width, height, srcPitch, dstPitch);
}

#else

#ifdef HAS_ALTIVEC
#include <sys/sysctl.h> 

static int isAltiVecAvailable()  {
	int selectors[2] = { CTL_HW, HW_VECTORUNIT }; 
	int hasVectorUnit = 0; 
	size_t length = sizeof(hasVectorUnit); 
	int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0); 
	if( 0 == error )
		return hasVectorUnit != 0; 
	return false; 
}
#endif

#define PIXEL00_0	*(q) = w5;
#define PIXEL00_10	*(q) = interpolate16_2(3, 1, w5, w1);
#define PIXEL00_11	*(q) = interpolate16_2(3, 1, w5, w4);
#define PIXEL00_12	*(q) = interpolate16_2(3, 1, w5, w2);
#define PIXEL00_20	*(q) = interpolate16_3(2, 1, 1, w5, w4, w2);
#define PIXEL00_21	*(q) = interpolate16_3(2, 1, 1, w5, w1, w2);
#define PIXEL00_22	*(q) = interpolate16_3(2, 1, 1, w5, w1, w4);
#define PIXEL00_60	*(q) = interpolate16_3(5, 2, 1, w5, w2, w4);
#define PIXEL00_61	*(q) = interpolate16_3(5, 2, 1, w5, w4, w2);
#define PIXEL00_70	*(q) = interpolate16_3(6, 1, 1, w5, w4, w2);
#define PIXEL00_90	*(q) = interpolate16_3(2, 3, 3, w5, w4, w2);
#define PIXEL00_100	*(q) = interpolate16_3(14, 1, 1, w5, w4, w2);

#define PIXEL01_0	*(q+1) = w5;
#define PIXEL01_10	*(q+1) = interpolate16_2(3, 1, w5, w3);
#define PIXEL01_11	*(q+1) = interpolate16_2(3, 1, w5, w2);
#define PIXEL01_12	*(q+1) = interpolate16_2(3, 1, w5, w6);
#define PIXEL01_20	*(q+1) = interpolate16_3(2, 1, 1, w5, w2, w6);
#define PIXEL01_21	*(q+1) = interpolate16_3(2, 1, 1, w5, w3, w6);
#define PIXEL01_22	*(q+1) = interpolate16_3(2, 1, 1, w5, w3, w2);
#define PIXEL01_60	*(q+1) = interpolate16_3(5, 2, 1, w5, w6, w2);
#define PIXEL01_61	*(q+1) = interpolate16_3(5, 2, 1, w5, w2, w6);
#define PIXEL01_70	*(q+1) = interpolate16_3(6, 1, 1, w5, w2, w6);
#define PIXEL01_90	*(q+1) = interpolate16_3(2, 3, 3, w5, w2, w6);
#define PIXEL01_100	*(q+1) = interpolate16_3(14, 1, 1, w5, w2, w6);

#define PIXEL10_0	*(q+nextlineDst) = w5;
#define PIXEL10_10	*(q+nextlineDst) = interpolate16_2(3, 1, w5, w7);
#define PIXEL10_11	*(q+nextlineDst) = interpolate16_2(3, 1, w5, w8);
#define PIXEL10_12	*(q+nextlineDst) = interpolate16_2(3, 1, w5, w4);
#define PIXEL10_20	*(q+nextlineDst) = interpolate16_3(2, 1, 1, w5, w8, w4);
#define PIXEL10_21	*(q+nextlineDst) = interpolate16_3(2, 1, 1, w5, w7, w4);
#define PIXEL10_22	*(q+nextlineDst) = interpolate16_3(2, 1, 1, w5, w7, w8);
#define PIXEL10_60	*(q+nextlineDst) = interpolate16_3(5, 2, 1, w5, w4, w8);
#define PIXEL10_61	*(q+nextlineDst) = interpolate16_3(5, 2, 1, w5, w8, w4);
#define PIXEL10_70	*(q+nextlineDst) = interpolate16_3(6, 1, 1, w5, w8, w4);
#define PIXEL10_90	*(q+nextlineDst) = interpolate16_3(2, 3, 3, w5, w8, w4);
#define PIXEL10_100	*(q+nextlineDst) = interpolate16_3(14, 1, 1, w5, w8, w4);

#define PIXEL11_0	*(q+1+nextlineDst) = w5;
#define PIXEL11_10	*(q+1+nextlineDst) = interpolate16_2(3, 1, w5, w9);
#define PIXEL11_11	*(q+1+nextlineDst) = interpolate16_2(3, 1, w5, w6);
#define PIXEL11_12	*(q+1+nextlineDst) = interpolate16_2(3, 1, w5, w8);
#define PIXEL11_20	*(q+1+nextlineDst) = interpolate16_3(2, 1, 1, w5, w6, w8);
#define PIXEL11_21	*(q+1+nextlineDst) = interpolate16_3(2, 1, 1, w5, w9, w8);
#define PIXEL11_22	*(q+1+nextlineDst) = interpolate16_3(2, 1, 1, w5, w9, w6);
#define PIXEL11_60	*(q+1+nextlineDst) = interpolate16_3(5, 2, 1, w5, w8, w6);
#define PIXEL11_61	*(q+1+nextlineDst) = interpolate16_3(5, 2, 1, w5, w6, w8);
#define PIXEL11_70	*(q+1+nextlineDst) = interpolate16_3(6, 1, 1, w5, w6, w8);
#define PIXEL11_90	*(q+1+nextlineDst) = interpolate16_3(2, 3, 3, w5, w6, w8);
#define PIXEL11_100	*(q+1+nextlineDst) = interpolate16_3(14, 1, 1, w5, w6, w8);

#define YUV(x)	hqx_lookup[w ## x]

#ifdef HAS_ALTIVEC
#define HQ2x	HQ2x_c
#endif

void HQ2x(const Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height) {
#include "system_hq2x.h"
}

#ifdef HAS_ALTIVEC

#undef HQ2x

#define USE_ALTIVEC	1
void HQ2x_Altivec(const Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height) {
#include "system_hq2x.h"
}
#undef USE_ALTIVEC

void HQ2x(const Uint8 *srcPtr, Uint32 srcPitch, Uint8 *dstPtr, Uint32 dstPitch, int width, int height) {
	if (isAltiVecAvailable())
		HQ2x_565_Altivec(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		HQ2x_c(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}
#endif /* HAS_ALTIVEC */

#endif /* USE_NASM */
