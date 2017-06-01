/* $NiH: system_hq_intern.h,v 1.3 2004/07/21 10:00:17 dillo Exp $ */
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
 * Header: /cvsroot/scummvm/scummvm/common/scaler/intern.h,v 1.12 2004/05/21 02:08:47 sev Exp 
 *
 */


#ifndef COMMON_SCALER_INTERN_H
#define COMMON_SCALER_INTERN_H

#define redblueMask	0x0f0f
#define greenMask	0x00f0


/**
 * Interpolate two 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
#define interpolate16_2(w1, w2, p1, p2)					   \
	((((((p1) & redblueMask) * (w1)					   \
	    + ((p2) & redblueMask) * (w2)) / ((w1) + (w2))) & redblueMask) \
	 | (((((p1) & greenMask) * (w1)					   \
	      + ((p2) & greenMask) * (w2)) / ((w1) + (w2))) & greenMask))

/**
 * Interpolate three 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
#define interpolate16_3(w1, w2, w3, p1, p2, p3)				\
	((((((p1) & redblueMask) * (w1) + ((p2) & redblueMask) * (w2)	\
	    + ((p3) & redblueMask) * (w3)) / ((w1) + (w2) + (w3)))	\
	  & redblueMask)						\
	 | (((((p1) & greenMask) * (w1) + ((p2) & greenMask) * (w2)	\
	      + ((p3) & greenMask) * (w3)) / ((w1) + (w2) + (w3)))	\
	    & greenMask))


/**
 * Compare two YUV values (encoded 8-8-8) and check if they differ by more than
 * a certain hard coded threshold. Used by the hq scaler family.
 */
static inline int diffYUV(int yuv1, int yuv2) {
        static const int Ymask = 0x00FF0000;
        static const int Umask = 0x0000FF00;
        static const int Vmask = 0x000000FF;
        static const int trY   = 0x00300000;
        static const int trU   = 0x00000700;
        static const int trV   = 0x00000006;
	
	int diff;
	int mask;
	
	diff = ((yuv1 & Ymask) - (yuv2 & Ymask));
	mask = diff >> 31; /* -1 if value < 0, 0 otherwise */
	diff = (diff ^ mask) - mask; /* -1: ~value + 1; 0: value */
	if (diff > trY) return 1;

	diff = ((yuv1 & Umask) - (yuv2 & Umask));
	mask = diff >> 31; /* -1 if value < 0, 0 otherwise */
	diff = (diff ^ mask) - mask; /* -1: ~value + 1; 0: value */
	if (diff > trU) return 1;

	diff = ((yuv1 & Vmask) - (yuv2 & Vmask));
	mask = diff >> 31; /* -1 if value < 0, 0 otherwise */
	diff = (diff ^ mask) - mask; /* -1: ~value + 1; 0: value */
	if (diff > trV) return 1;

	return 0;
/*
	return
	  ( ( ABS((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
	    ( ABS((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
	    ( ABS((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
*/
}

#endif
