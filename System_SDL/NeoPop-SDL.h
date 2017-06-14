/* $NiH: NeoPop-SDL.h,v 1.23 2004/07/24 00:07:42 dillo Exp $ */
/*
  NeoPop-SDL.h -- common header file
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

#include "neopop.h"
#include <SDL.h>

/* one per line, please */
enum neopop_event {
    /* unbound key */
    NPEV_NONE,
    
    /* console input, keep these in order */
    NPEV_JOY_UP,
    NPEV_JOY_DOWN,
    NPEV_JOY_LEFT,
    NPEV_JOY_RIGHT,
    NPEV_JOY_BUTTON_A,
    NPEV_JOY_BUTTON_B,
    NPEV_JOY_OPTION,
    
    /* gui events */
    NPEV_GUI_FRAMESKIP_DECREMENT,
    NPEV_GUI_FRAMESKIP_INCREMENT,
    NPEV_GUI_FULLSCREEN_OFF,
    NPEV_GUI_FULLSCREEN_ON,
    NPEV_GUI_FULLSCREEN_TOGGLE,
    NPEV_GUI_MAGNIFY_1,
    NPEV_GUI_MAGNIFY_2,
    NPEV_GUI_MAGNIFY_3,
    NPEV_GUI_MENU,
    NPEV_GUI_MUTE_OFF,
    NPEV_GUI_MUTE_ON,
    NPEV_GUI_MUTE_TOGGLE,
    NPEV_GUI_PAUSE_OFF,
    NPEV_GUI_PAUSE_ON,
    NPEV_GUI_PAUSE_TOGGLE,
    NPEV_GUI_QUIT,
    NPEV_GUI_SCREENSHOT,
    NPEV_GUI_SMOOTH_OFF,
    NPEV_GUI_SMOOTH_ON,
    NPEV_GUI_SMOOTH_TOGGLE,
    NPEV_GUI_STATE_LOAD,
    NPEV_GUI_STATE_SAVE,
    NPEV_GUI_STATE_SLOT_DECREMENT,
    NPEV_GUI_STATE_SLOT_INCREMENT,

    NPEV_LAST
};

enum nprc {
    NPRC_COLOUR,
    NPRC_COMMS_MODE,
    NPRC_COMMS_PORT,
    NPRC_COMMS_REMOTE,
    NPRC_FLASH_DIR,
    NPRC_FRAMESKIP,
    NPRC_FULLSCREEN,
    NPRC_LANGUAGE,
    NPRC_MAGNIFY,
    NPRC_MAP,
    NPRC_OSD_COLOUR,
    NPRC_SAMPLERATE,
    NPRC_SCREENSHOT_DIR,
    NPRC_SMOOTH,
    NPRC_STATE_DIR,
    NPRC_SOUND,
    NPRC_USE_ROM_NAME,
    NPRC_YUV,
    NPRC_YUV_SOFTWARE,

    NPRC_LAST
};

/* defines for keys */

#define NPKS_UP			0	/* key being pressed */
#define NPKS_DOWN		1	/* key being released */

/* modifiers */

enum npks_shift {
    NPKS_SH_NONE,
    NPKS_SH_CTRL,
    NPKS_SH_ALT,
    NPKS_NKEY
};

/* size and layout of bindings array */

#define NPKS_KEY_BASE		0
#define NPKS_KEY_SIZE		SDLK_LAST
#define NPKS_JOY_BASE		(NPKS_KEY_BASE+NPKS_NKEY*NPKS_KEY_SIZE)
#define NPKS_NJOY		1
#define NPKS_JOY_NAXIS		7
#define NPKS_JOY_NHAT		1
#define NPKS_JOY_NBUTTON	20
#define NPKS_JOY_AXIS_OFFSET	0
#define NPKS_JOY_HAT_OFFSET	(2*NPKS_JOY_NAXIS)
#define NPKS_JOY_BUTTON_OFFSET	(NPKS_JOY_HAT_OFFSET+4*NPKS_JOY_NHAT)
#define NPKS_JOY_SIZE		(NPKS_JOY_BUTTON_OFFSET+NPKS_JOY_NBUTTON)
#define NPKS_SIZE		(NPKS_NKEY*NPKS_KEY_SIZE	\
				 +NPKS_NJOY*NPKS_JOY_SIZE)

#define NPKS_KEY(c, k)		(NPKS_KEY_BASE+(c)*NPKS_KEY_SIZE+(k))
#define NPKS_JOY(n)		(NPKS_JOY_BASE+(n)*NPKS_JOY_SIZE)
#define NPKS_JOY_AXIS(n, i)	(NPKS_JOY(n)+NPKS_JOY_AXIS_OFFSET+2*(i))
#define NPKS_JOY_HAT(n, i)	(NPKS_JOY(n)+NPKS_JOY_HAT_OFFSET+4*(i))
#define NPKS_JOY_BUTTON(n, i)	(NPKS_JOY(n)+NPKS_JOY_BUTTON_OFFSET+(i))

enum comms_mode {
    COMMS_NONE,
    COMMS_SERVER,
    COMMS_CLIENT,

    COMMS_LAST
};

enum {
    YUV_NEVER,
    YUV_FULLSCREEN,
    YUV_MAGNIFIED,
    YUV_ALWAYS,

    YUV_LAST
};

#define NGP_FPS 59.95			/* frames per second */

#define DEFAULT_SAMPLERATE	8000 //22050 //44100	/* default sample rate */
#define DEFAULT_YUV	YUV_MAGNIFIED

#define PAUSED_LOCAL	1
#define PAUSED_REMOTE	2

/* convert 4bit colour component to 8bit */
#define CONV4TO8(x)	(((x)<<4)|(x))

void system_bindings_init(void);

BOOL system_comms_connect(void);
void system_comms_pause(BOOL);

void system_graphics_fullscreen(int);
BOOL system_graphics_init(void);
void system_graphics_update(void);

void system_input_update(void);

char *system_make_file_name(const char *, const char *, int);

void system_osd(const char *, ...);
void system_osd_display(void);
int system_osd_init(void);
void system_osd_pause(int);

const char *system_npev_name(enum neopop_event);
int system_npev_parse(const char *, char **);
const char *system_npks_name(int);
int system_npks_parse(const char *, char **);

void system_rc_read(void);
void system_rc_read_file(const char *);
int system_rc_parse_comms_mode(const char *);
int system_rc_parse_yuv(const char *);

void system_rom_changed(void);
BOOL system_rom_load(char *);
void system_rom_unload(void);

int system_screenshot(void);

void system_sound_callback(void *, Uint8 *, int);
void system_sound_chipreset(void);
BOOL system_sound_init(void);
void system_sound_shutdown(void);
void system_sound_silence(void);
void system_sound_update(int);

void system_state_load(void);
void system_state_save(void);

void save_config(char *file);

void HQ2x(const Uint8 *, Uint32, Uint8 *, Uint32, int, int);
void HQ3x(const Uint8 *, Uint32, Uint8 *, Uint32, int, int);

extern int do_exit;
extern int paused;
extern int have_sound;
extern int graphics_mag_req;
extern int graphics_mag_smooth;
extern int comms_mode;
extern int comms_port;
extern char *comms_host;
extern int samplerate;
extern int use_yuv;
extern int use_software_yuv;
extern char *state_dir;
extern char *flash_dir;
extern char *screenshot_dir;
extern int use_rom_name;
extern int state_slot;
extern int fs_mode;
extern int need_redraw;
extern int osd_colour;

extern Uint32 hqx_lookup[16*16*16];

extern enum neopop_event bindings[];
extern const char *comms_names[];
extern const char *npev_names[];
extern const char *nprc_names[];
extern const char *yuv_names[];
