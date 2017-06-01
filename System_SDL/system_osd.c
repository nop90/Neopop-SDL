/* $NiH: system_osd.c,v 1.6 2004/07/24 00:07:43 dillo Exp $

  system_osd.c -- on-screen display
  Copyright (C) 2004 Thomas Klausner and Dieter Baron

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <wiz@danbala.tuwien.ac.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "NeoPop-SDL.h"

int osd_colour;

#define OSD_DISPLAY_SECONDS	2	/* seconds to display OSD message */

#define FONT_WIDTH	9	/* char width in pixels */
#define FONT_HEIGHT	9	/* char height in pixels */
#define FONT_NCHAR	64	/* number of chars */
#define CHAR_SIZE	(FONT_WIDTH*FONT_HEIGHT)

#define OSD_BORDER	0x000

#define BLEND(c1, c2, c3, a, b, s)					\
	((( (((c1)>>s)&0xf) * (a)					\
	  + (((c2)>>s)&0xf) * (b)					\
	  + (((c3)>>s)&0xf) * (0x100-a-b) ) & 0xf00) >> (8-(s)))

#include "font.xpm"

static _u8 font_c[CHAR_SIZE*FONT_NCHAR];
static _u8 font_b[CHAR_SIZE*FONT_NCHAR];
static int font_w[FONT_NCHAR];
static int font_usable;

static char osd[SCREEN_WIDTH/FONT_WIDTH+1];
static int osd_timer;

static _u16 osd_line[SCREEN_WIDTH*FONT_HEIGHT];
static _u16 pause_line[SCREEN_WIDTH*FONT_HEIGHT];

#define PAUSE_LINE	(SCREEN_HEIGHT-FONT_HEIGHT)/2
#define OSD_LINE	(SCREEN_HEIGHT-FONT_HEIGHT)

#define DARKEN(x)	(((x)>>1) & 0x0777)

static void display_string(int, int, const char *);
static int string_width(const char *);



int
system_osd_init(void)
{
    _u8 pal_b[128], pal_c[128];
    int trans;
    int w, h;
    int i, n;
    int x, y, c;
    int off, pixel;
    char *p;

    font_usable = FALSE;

    w = strtol(font_xpm[0], &p, 10);
    h = strtol(p, &p, 10);
    n = strtol(p, &p, 10);

    if (w%FONT_WIDTH || h%FONT_HEIGHT || w*h != CHAR_SIZE*FONT_NCHAR) {
	printf("font has wrong dimensions\n");
	return FALSE;
    }

    if (strtol(p, &p, 10) != 1) {
	printf("font uses multi-char pixels\n");
	return FALSE;
    }

    trans = -1;
    for (i=0; i<n; i++) {
	c = font_xpm[i+1][0];

	if ((p=strrchr(font_xpm[i+1], '#')) == NULL)
	    trans = c;
	else {
	    pixel = strtol(p+1, NULL, 16);
	    pal_c[c] = pixel&0xff;
	    pal_b[c] = (pixel>>8)&0xff;
	}
	if (pal_b[c] + pal_c[c] > 0x100)
	    pal_b[c] = 0x100 - pal_c[c];
    }

    for (c=0; c<FONT_NCHAR; c++)
	font_w[c] = FONT_WIDTH;

    for (y=0; y<h; y++) {
	for (x=0; x<w; x++) {
	    c = (y/FONT_HEIGHT) * (w/FONT_WIDTH) + (x/FONT_WIDTH);
	    off = c*CHAR_SIZE + (y%FONT_HEIGHT)*FONT_HEIGHT + x%FONT_WIDTH;
	    pixel = font_xpm[y+n+1][x];
	    if (pixel == trans) {
		if ((x%FONT_WIDTH) < font_w[c])
		    font_w[c] = x%FONT_WIDTH;
	    }
	    else {
		font_c[off] = pal_c[pixel];
		font_b[off] = pal_b[pixel];
	    }
	}
    }

    font_usable = TRUE;

    return TRUE;
}



void
system_osd(const char *fmt, ...)
{
    int i;
    va_list va;

    if (!font_usable)
	return;

    va_start(va, fmt);
    vsnprintf(osd, sizeof(osd), fmt, va);
    va_end(va);
    osd_timer = OSD_DISPLAY_SECONDS*NGP_FPS;

    /* eliminate illegal chars */
    for (i=0; osd[i]; i++) {
	if (osd[i] >= 'a' && osd[i] <= 'z')
	    osd[i] = toupper(osd[i]);
	else if (osd[i] < ' ' || osd[i] > '_')
	    osd[i] = ' ';
    }
}



void
system_osd_display(void)
{
    if (!font_usable || osd_timer == 0)
	return;

    if (paused)
	    memcpy(cfb+SCREEN_WIDTH*OSD_LINE, osd_line,
		   SCREEN_WIDTH*sizeof(_u16)*FONT_HEIGHT);
    
    if (--osd_timer > 0)
	display_string(0, OSD_LINE, osd);

    if (paused)
	system_graphics_update();
}



void
system_osd_pause(int which)
{
    int i;
    char *s;

    if (which == paused)
	return;

    if (which == 0) {
	/* leaving pause: next frame update restores cfb */
	return;
    }
    if (paused == 0) {
	/* entering pause: darken display and save lines used by OSD */

	for (i=0; i<SCREEN_WIDTH*SCREEN_HEIGHT; i++)
	    cfb[i] = DARKEN(cfb[i]);
	memcpy(pause_line, cfb+SCREEN_WIDTH*PAUSE_LINE,
	       SCREEN_WIDTH*sizeof(_u16)*FONT_HEIGHT);
	memcpy(osd_line, cfb+SCREEN_WIDTH*OSD_LINE,
	       SCREEN_WIDTH*sizeof(_u16)*FONT_HEIGHT);
	if (osd_timer > 0)
	    display_string(0, OSD_LINE, osd);
    }

    /* display paused message */
    memcpy(cfb+SCREEN_WIDTH*PAUSE_LINE, pause_line,
	   SCREEN_WIDTH*sizeof(_u16)*FONT_HEIGHT);
    
    s = (which & PAUSED_LOCAL) ? "PAUSED" : "REMOTE PAUSED";
    display_string((SCREEN_WIDTH-string_width(s))/2, PAUSE_LINE, s);

    system_graphics_update();
}



static void
display_string(int x0, int y0, const char *s)
{
    int i, x, y;
    _u16 *base;
    int alpha_c, alpha_b, c;

    base = cfb + SCREEN_WIDTH*y0+x0;
    
    for (i=0; s[i]; i++) {
	for (y=0; y<FONT_HEIGHT; y++)
	    for (x=0; x<font_w[s[i]-32]; x++) {
		c = base[y*SCREEN_WIDTH+x];
		alpha_c = font_c[(s[i]-32)*CHAR_SIZE+y*FONT_WIDTH+x];
		if (alpha_c == 0xff)
		    alpha_c = 0x100;
		alpha_b = font_b[(s[i]-32)*CHAR_SIZE+y*FONT_WIDTH+x];
		if (alpha_b == 0xff)
		    alpha_b = 0x100;
		base[y*SCREEN_WIDTH+x] =
		    BLEND(osd_colour, OSD_BORDER, c, alpha_c, alpha_b, 8)
		    | BLEND(osd_colour, OSD_BORDER, c, alpha_c, alpha_b, 4)
		    | BLEND(osd_colour, OSD_BORDER, c, alpha_c, alpha_b, 0);
	    }
	base += font_w[s[i]-32]-1;
    }
}



static int
string_width(const char *s)
{
    int w;

    w = 0;
    for (; *s; s++)
	w += font_w[*s-32]-1;

    return w+1;
}
