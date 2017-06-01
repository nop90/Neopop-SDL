/* $NiH: system_screenshot.c,v 1.10 2004/07/25 10:42:08 wiz Exp $ */
/*
  system_screenshot.c -- screenshot functions
  Copyright (C) 2002-2004 Thomas Klausner and Dieter Baron

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

#include "config.h"
#include "NeoPop-SDL.h"
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char *screenshot_dir;

static char *
get_screenshot_name(const char *ext)
{
    static int scount;
    char *name;
    char *countext;
    struct stat sb;

    /* 3 for the number, one for the \0 */
    if ((countext=malloc(strlen(ext)+4)) == NULL) {
	fprintf(stderr, "Saving screenshot failed: %s\n",
		strerror(errno));
	return NULL;
    }
	
    do {
	snprintf(countext, strlen(ext)+4, "%03d%s",
		 scount, ext);
	name = system_make_file_name(screenshot_dir, countext, 1);
	if (stat(name, &sb) == -1 && errno == ENOENT) {
	    free(countext);
	    return name;
	}
	free(name);
	scount++;
    } while (scount < 1000);

    free(countext);

    return NULL;
}

#if !defined(HAVE_LIBPNG)
int
system_screenshot(void)
{
    SDL_Surface *image;
    char *bmpname;
    int ret;

    ret = 0;

    if ((bmpname=get_screenshot_name(".bmp")) == NULL)
	return -1;

    image = SDL_CreateRGBSurfaceFrom(cfb, SCREEN_WIDTH,
				     SCREEN_HEIGHT, 16,
				     sizeof(_u16)*SCREEN_WIDTH,
				     0x000f, 0x00f0, 0x0f00, 0);
    if (image == NULL) {
	free(bmpname);
	return -1;
    }

    if (SDL_SaveBMP(image, bmpname) < 0) {
	fprintf(stderr, "Saving screenshot to `%s' failed: %s\n",
		bmpname, SDL_GetError());
	ret = -1;
    }

    SDL_FreeSurface(image);
    free(bmpname);

    return ret;
}
#else /* HAVE LIBPNG */
#include <png.h>

static void
converter(png_structp ptr, png_row_infop row_info, png_bytep row)
{
    int i;
    _u16 *inp;
    _u8 out[3*SCREEN_WIDTH];
    _u8 *outp;

    outp = out;
    inp = (_u16 *)row;
    for (i=0; i<SCREEN_WIDTH; i++) {
	*outp++ = (_u8)CONV4TO8(*inp & 0x000f);		/* red */
	*outp++ = (_u8)CONV4TO8((*inp & 0x00f0)>>4);	/* green */
	*outp++ = (_u8)CONV4TO8((*inp & 0x0f00)>>8);	/* blue */
	inp++;
    }

    memcpy(row, out, sizeof(out));
    return;
}
    
int
system_screenshot(void)
{
    char *pngname;
    int ret, i;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_color_8 sig_bit;
    png_bytep row_pointer;
    png_bytep row_pointers[SCREEN_HEIGHT];
    FILE *fp;

    ret = 0;
    /* get file name */
    if ((pngname=get_screenshot_name(".png")) == NULL)
	return -1;

    /* open file for writing */
    if ((fp=fopen(pngname, "wb")) == NULL) {
	ret = -1;
	goto end;
    }

    /* init png structures */
    if ((png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
					 NULL, NULL)) == NULL) {
	ret = -1;
	goto closeend;
    }
    
    if ((info_ptr=png_create_info_struct(png_ptr)) == NULL) {
	ret = -1;
	goto freepng;
    }

    png_init_io(png_ptr, fp);
    png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);
    png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT, 8,
		 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    sig_bit.red = sig_bit.green = sig_bit.blue = 4;
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_shift(png_ptr, &sig_bit);
    png_set_write_user_transform_fn(png_ptr, converter);
    /* write image to file */
    row_pointer = (png_bytep)cfb;
    for (i=0; i<SCREEN_HEIGHT; i++) {
	row_pointers[i] = row_pointer;
	row_pointer += 2*SCREEN_WIDTH;
    }
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);

 freepng:
    png_destroy_write_struct(&png_ptr, &info_ptr);

 closeend:
    /* close file and cleanup */
    if (ret == 0)
	ret = fclose(fp);
    else
	fclose(fp);

 end:
    free(pngname);
    return ret;
}
#endif /* HAVE_LIBPNG */
