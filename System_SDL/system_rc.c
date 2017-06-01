/* $NiH: system_rc.c,v 1.14 2004/07/25 10:34:37 dillo Exp $ */
/*
  system_rc.c -- config file handling
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
#include <limits.h>

#include "NeoPop-SDL.h"

static const char *bool_name[] = {
    "no", "yes",
    "off", "on"
};

static const char *colour_name[] = {
    "greyscale",
    "colour",
    "auto"
};

static const char *lang_name[] = {
    "japanese",
    "english"
};

static const char *shift_name[] = {
    "", "C-", "M-"
};
static const char *axis_name[] = {
    "neg", "pos"
};
static const char *hat_name[] = {
    "up", "down", "left", "right"
};

#define NAME_SIZE(x)	(sizeof(x)/sizeof((x)[0]))



static const char *fname;
static int lno;
static int error_line;



static int find_name(const char *, char **, const char **, int);
static void rc_error(const char *, ...);
static int rc_parse_boolean(const char *, char **);
static int rc_parse_int(const char *, char **, int, int);



const char *
system_npev_name(enum neopop_event ev)
{
    static const char *unknown="[unknown]";

    if (ev < 0 || ev >= NPEV_LAST)
	return unknown;

    return npev_names[ev];
}



int
system_npev_parse(const char *name, char **end)
{
    return find_name(name, end, npev_names, NPEV_LAST);
}



const char *
system_npks_name(int k)
{
    static char b[8192];
    int n;

    if (k < NPKS_JOY_BASE) {
	n = k / NPKS_KEY_SIZE;
	k %= NPKS_KEY_SIZE;

	snprintf(b, sizeof(b), "%s%s", shift_name[n], SDL_GetKeyName(k));
    }
    else {
	k -= NPKS_JOY_BASE;
	n = k / NPKS_JOY_SIZE;
	k %= NPKS_JOY_SIZE;

	if (k < NPKS_JOY_HAT_OFFSET)
	    snprintf(b, sizeof(b), "joy %d axis %d %s",
		     n+1, (k/2)+1, axis_name[k%2]);
	else if (k < NPKS_JOY_BUTTON_OFFSET) {
	    k -= NPKS_JOY_HAT_OFFSET;
	    snprintf(b, sizeof(b), "joy %d hat %d %s",
		     n+1, (k/4)+1, hat_name[k%4]);
	}
	else {
	    k -= NPKS_JOY_BUTTON_OFFSET;
	    snprintf(b, sizeof(b), "joy %d button %d", n+1, k+1);
	}
    }

    return b;
}



int
system_npks_parse(const char *name, char **end)
{
    static const char *key_name[SDLK_LAST];
    static int key_name_inited = 0;

    const char *p;
    int n, k, i;

    p = name+strspn(name, " \t");
    if (strncasecmp(p, "joy", 3) == 0) {
	p += 4;
	n = strtoul(p, (char **)&p, 10)-1;
	if (n < 0 || n >= NPKS_NJOY)
	    return -1;

	p += strspn(p, " \t");
	if (strncasecmp(p, "axis", 4) == 0) {
	    p += 4;
	    k = strtoul(p, (char **)&p, 10)-1;
	    if (k < 0 || k >= NPKS_JOY_NAXIS)
		return -1;
	    
	    if ((i=find_name(p, (char **)&p,
			     axis_name, NAME_SIZE(axis_name))) == -1)
		return -1;

	    if (end)
		*end = (char *)p;
	    return NPKS_JOY_AXIS(n, k) + i;
	}
	else if (strncasecmp(p, "hat", 3) == 0) {
	    p += 3;

	    k = strtoul(p, (char **)&p, 10)-1;
	    if (k < 0 || k >= NPKS_JOY_NBUTTON)
		return -1;

	    if ((i=find_name(p, (char **)&p,
			     hat_name, NAME_SIZE(hat_name))) == -1)
		return -1;

	    if (end)
		*end = (char *)p;
	    return NPKS_JOY_HAT(n, k) + i;
	}
	else if (strncasecmp(p, "button", 6) == 0) {
	    p += 6;

	    k = strtoul(p, (char **)&p, 10)-1;
	    if (k < 0 || k >= NPKS_JOY_NBUTTON)
		return -1;

	    if (end)
		*end = (char *)p;
	    return NPKS_JOY_BUTTON(n, k);
	}
	else
	    return -1;
    }
    else {
	if (!key_name_inited) {
	    for (i=0; i<SDLK_LAST; i++) {
		key_name[i] = SDL_GetKeyName(i);
		if (strcmp(key_name[i], "unknown key") == 0)
		    key_name[i] = NULL;
	    }
	    key_name_inited = 1;
	}

	n = NPKS_SH_NONE;
	if (p[1] == '-') {
	    switch (p[0]) {
	    case 'C':
	    case 'c':
		n = NPKS_SH_CTRL;
		p += 2; 
		break;
	    case 'A':
	    case 'a':
	    case 'M':
	    case 'm':
		n = NPKS_SH_ALT;
		p += 2; 
		break;
	    }
	}

	if ((k=find_name(p, (char **)&p, key_name, SDLK_LAST)) != -1) {
	    if (end)
		*end = (char *)p;
	    return NPKS_KEY(n, k);
	}
	return -1;
    }    
}



int
system_rc_parse_comms_mode(const char *mode)
{
    return find_name(mode, NULL, comms_names, COMMS_LAST);
}



int
system_rc_parse_yuv(const char *mode)
{
    return find_name(mode, NULL, yuv_names, YUV_LAST);
}



void
system_rc_read(void)
{
    char b[8192], *home;

    /* XXX: read system wide rc file */

    if ((home=getenv("HOME")) == NULL)
	return;
    snprintf(b, sizeof(b), "%s/.neopop/neopoprc", home);
    system_rc_read_file(b);
}



void
system_rc_read_file(const char *filename)
{
    char b[8192], *p;
    FILE *f;
    int cmd;
    int i;

    if ((f=fopen(filename, "r")) == NULL)
	return;

    fname = filename;
    lno = 0;
    
    while (fgets(b, sizeof(b), f)) {
	lno++;
	error_line = 0;
	if (b[strlen(b)-1] == '\n')
	    b[strlen(b)-1] = '\0';
	if (b[0] == '#' || b[0] == '\0')
	    continue;

	cmd = find_name(b, &p, nprc_names, NPRC_LAST);
	
	switch (cmd) {
	case -1:
	    rc_error("unknown command [%s]", b);
	    break;
	    
	case NPRC_COLOUR:
	    if ((i=find_name(p, &p, colour_name,
			     NAME_SIZE(colour_name))) == -1) {
		rc_error("unknown colour setting [%s]", p);
		break;
	    }
	    system_colour = i;
	    break;

	case NPRC_COMMS_MODE:
	    if ((i=find_name(p, &p, comms_names, COMMS_LAST)) == -1) {
		rc_error("unknown comms mode [%s]", p);
		break;
	    }
	    comms_mode = i;
	    break;
	    
	case NPRC_COMMS_PORT:
	    if ((i=rc_parse_int(p, &p, 1, 0xffff)) == INT_MIN)
		break;
	    comms_port = i;
	    break;
	    
	case NPRC_COMMS_REMOTE:
	    p += strspn(p, " \t");
	    /* XXX: check for legal host name (syntax) */
	    comms_host = strdup(p);
	    p += strlen(p);
	    break;

	case NPRC_FLASH_DIR:
	    p += strspn(p, " \t");
	    free(flash_dir);
	    flash_dir = strdup(p);
	    p += strlen(p);
	    break;

	case NPRC_FRAMESKIP:
	    if ((i=rc_parse_int(p, &p, 1, 7)) == INT_MIN)
		break;
	    system_frameskip_key = i;
	    break;
	
	case NPRC_FULLSCREEN:
	    if ((i=rc_parse_boolean(p, &p)) == -1)
		break;
	    fs_mode = i;
	    break;
	    
	case NPRC_LANGUAGE:
	    if ((i=find_name(p, &p, lang_name,NAME_SIZE(lang_name))) == -1) {
		rc_error("unknown language [%s]", p);
		break;
	    }
	    language_english = i;
	    break;
	    
	case NPRC_MAGNIFY:
	    if ((i=rc_parse_int(p, &p, 1, 3)) == INT_MIN)
		break;
	    graphics_mag_req = i;
	    break;
	    
	case NPRC_MAP:
	    {
		int k, ev;
		
		if ((k=system_npks_parse(p, &p)) < 0) {
		    rc_error("cannot parse key [%s]", p);
		    break;
		}

		p += strspn(p, " \t");
		if (p[0] != '=') {
		    rc_error("missing = in map [%s]", p);
		    break;
		}
		p++;
		
		if ((ev=system_npev_parse(p, &p)) < 0) {
		    rc_error("cannot parse event [%s]", p);
		    break;
		}
		
		bindings[k] = ev;
	    }
	    break;

	case NPRC_OSD_COLOUR:
	    p += strspn(p, " \t");
	    switch (strspn(p, "0123456789abcdefABCDEF")) {
	    case 3:
		i = strtol(p, &p, 16);
		break;
	    case 6:
		i = strtol(p, &p, 16);
		i = ((i>>12)&0xf00) | ((i>>8)&0xf0) | ((i>>4)&0xf);
		break;
	    default:
		i = -1;
		rc_error("cannot parse OSD colour [%s]", p);
	    }
	    if (i != -1)
		osd_colour = ((i&0xf00)>>8) | (i&0x0f0) | ((i&0x00f)<<8);
	    break;

	case NPRC_SCREENSHOT_DIR:
	    p += strspn(p, " \t");
	    free(screenshot_dir);
	    screenshot_dir = strdup(p);
	    p += strlen(p);
	    break;

	case NPRC_SMOOTH:
	    if ((i=rc_parse_boolean(p, &p)) == -1)
		break;
	    graphics_mag_smooth = i;
	    break;

	case NPRC_SAMPLERATE:
	    if ((i=rc_parse_int(p, &p, 8000, 48000)) == INT_MIN)
		break;
	    samplerate = i;
	    break;

	case NPRC_SOUND:
	    if ((i=rc_parse_boolean(p, &p)) == -1)
		break;
	    mute = !i;
	    break;

	case NPRC_STATE_DIR:
	    p += strspn(p, " \t");
	    free(state_dir);
	    state_dir = strdup(p);
	    p += strlen(p);
	    break;

	case NPRC_USE_ROM_NAME:
	    if ((i=rc_parse_boolean(p, &p)) == -1)
		break;
	    use_rom_name = i;
	    break;

	case NPRC_YUV:
	    if ((i=find_name(p, &p, yuv_names, YUV_LAST)) == -1) {
		rc_error("unknown yuv setting [%s]", p);
		break;
	    }
	    use_yuv = i;
	    break;

	case NPRC_YUV_SOFTWARE:
	    if ((i=rc_parse_boolean(p, &p)) == -1)
		break;
	    use_software_yuv = i;
	    break;
	}

	if (!error_line) {
	    p += strspn(p, " \t");
	    if (p[0] != '\0') {
		printf("warning: trailing garbage ignored [%s]", p);
		break;
	    }
	}
    }

    fclose(f);
}



static int
find_name(const char *p, char **end, const char **names, int n)
{
    int i, l, k, l2;

    k = l2 = -1;
    p += strspn(p, " \t");
    
    for (i=0; i<n; i++) {
	if (names[i] == NULL)
	    continue;

	l = strlen(names[i]);
	if (l>l2 && strncasecmp(p, names[i], l) == 0
	    && (p[l] == '\0' || isspace(p[l]))) {
	    k = i;
	    l2 = l;
	}
    }

    if (k != -1) {
	if (end)
	    *end = (char *)(p+l2);
    }
    return k;
}



static void
rc_error(const char *fmt, ...)
{
    va_list va;

    error_line = 1;

    fprintf(stderr, "%s:%d: ", fname, lno);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    putc('\n', stderr);
}



static int
rc_parse_boolean(const char *p, char **end)
{
    char *q;
    int i;
    
    if ((i=find_name(p, end, bool_name, NAME_SIZE(bool_name))) != -1)
	return i%2;

    i = strtoul(p, &q, 10);
    if (p == q || (q[0] != '\0' && !isspace(q[0])) || ((i!=0) && (i!=1))) {
	rc_error("cannot parse boolean [%s]", p);
	return -1;
    }

    if (end)
	*end = q;
    return i;
}



static int
rc_parse_int(const char *p, char **end, int low, int hi)
{
    char *q;
    int i;

    i = strtol(p, &q, 10);

    if (p == q || (q[0] != '\0' && !isspace(q[0]))) {
	rc_error("cannot parse integer [%s]", p);
	return INT_MIN;
    }

    if (i < low || i > hi) {
	rc_error("integer out of range (%d..%d) [%d]", low, hi, i);
	return INT_MIN;
    }

    if (end)
	*end = q;
    return i;
}
