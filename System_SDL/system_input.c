/* $NiH: system_input.c,v 1.26 2004/07/23 13:16:39 dillo Exp $ */
/*
  system_input.c -- input support functions
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

/* #define KEY_DEBUG */

#include "NeoPop-SDL.h"

static const int joy_mask[] = {
    0x01, /* up */
    0x02, /* down */
    0x04, /* left */
    0x08, /* right */
    0x10, /* button a */
    0x20, /* button b */
    0x40  /* option */
};
#define JOYPORT_ADDR	0x6F82

static int joy_axis[NPKS_NJOY*NPKS_JOY_NAXIS];
static int joy_hat[NPKS_NJOY*NPKS_JOY_NHAT];

#define JOY_AXIS(n, k)	(joy_axis[(n)*NPKS_JOY_NAXIS+(k)])
#define JOY_HAT(n, k)	(joy_hat[(n)*NPKS_JOY_NHAT+(k)])

static void handle_event(enum neopop_event, int);
static void emit_key(int, int);

static void set_fullscreen(int);
static void set_mute(int);
static void set_paused(int);



void
system_input_update(void)
{
    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
	switch(evt.type) {
	case SDL_QUIT:
	    do_exit = 1;
	    break;

	case SDL_VIDEOEXPOSE:
	    need_redraw = TRUE;
	    break;

	case SDL_KEYUP:
	case SDL_KEYDOWN:
	    {
		int c;

		switch(evt.key.keysym.sym) {
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		case SDLK_LALT:
		case SDLK_RALT:
		    /* shift keys are never considered shifted */
		    c = NPKS_SH_NONE;
		    break;
		default:
		    if (evt.key.keysym.mod & KMOD_CTRL)
			c = NPKS_SH_CTRL;
		    else if (evt.key.keysym.mod & KMOD_ALT)
			c = NPKS_SH_ALT;
		    else
			c = NPKS_SH_NONE;
		    break;
		}
		emit_key(NPKS_KEY(c, evt.key.keysym.sym), 
			 (evt.type == SDL_KEYDOWN ? NPKS_DOWN : NPKS_UP));
	    }
	    break;
	    
	case SDL_JOYAXISMOTION:
	    { 
		int k, pos, opos;

		if (evt.jaxis.which >= NPKS_NJOY
		    || evt.jaxis.axis >= NPKS_JOY_NAXIS)
		    break;

		if (evt.jaxis.value < -10922)
		    pos = -1;
		else if (evt.jaxis.value > 10922)
		    pos = 1;
		else
		    pos = 0;

		opos = JOY_AXIS(evt.jaxis.which, evt.jaxis.axis);
		if (pos == opos)
		    break;
		JOY_AXIS(evt.jaxis.which, evt.jaxis.axis) = pos;

		k = NPKS_JOY_AXIS(evt.jaxis.which, evt.jaxis.axis);

		switch (pos) {
		case -1:
		    if (opos == 1)
			emit_key(k+1, NPKS_UP);
		    emit_key(k, NPKS_DOWN);
		    break;

		case 0:
		    if (opos == -1)
			emit_key(k, NPKS_UP);
		    if (opos == 1)
			emit_key(k+1, NPKS_UP);
		    break;

		case 1:
		    if (opos == -1)
			emit_key(k, NPKS_UP);
		    emit_key(k+1, NPKS_DOWN);
		    break;
		}
	    }
	    break;
	    
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
	    if (evt.jbutton.which >= NPKS_NJOY
		|| evt.jbutton.button >= NPKS_JOY_NBUTTON)
		break;
	    
	    emit_key(NPKS_JOY_BUTTON(evt.jbutton.which,
				     evt.jbutton.button),
		     (evt.type == SDL_JOYBUTTONDOWN ? NPKS_DOWN : NPKS_UP));
	    break;

	case SDL_JOYHATMOTION:
	    {
		int pos, opos, k;
		
		if (evt.jhat.which >= NPKS_NJOY
		    || evt.jhat.hat >= NPKS_JOY_NHAT)
		break;
	    
		pos = evt.jhat.value;
		opos = JOY_HAT(evt.jhat.which, evt.jhat.hat);
		if (pos == opos)
		    break;
		JOY_HAT(evt.jhat.which, evt.jhat.hat) = pos;

		k = NPKS_JOY_HAT(evt.jhat.which, evt.jhat.hat);

		if ((pos^opos) & SDL_HAT_UP)
		    emit_key(k, ((pos & SDL_HAT_UP) ? NPKS_DOWN : NPKS_UP));
		if ((pos^opos) & SDL_HAT_DOWN)
		    emit_key(k+1,
			     ((pos & SDL_HAT_DOWN) ? NPKS_DOWN : NPKS_UP));
		if ((pos^opos) & SDL_HAT_LEFT)
		    emit_key(k+2,
			     ((pos & SDL_HAT_LEFT) ? NPKS_DOWN : NPKS_UP));
		if ((pos^opos) & SDL_HAT_RIGHT)
		    emit_key(k+3,
			     ((pos & SDL_HAT_RIGHT) ? NPKS_DOWN : NPKS_UP));
	    }
	    break;
	    
	default:
	    /* ignore */
	    break;
	}
    }
}



static void
emit_key(int k, int type)
{
#ifdef KEY_DEBUG
    printf("key %s (%d) %s == %s\n",
	   system_npks_name(k), k,
	   (type == NPKS_DOWN ? "down" : "up"),
	   system_npev_name(bindings[k]));
#endif
    handle_event(bindings[k], type);
}



static void
handle_event(enum neopop_event ev, int type)
{
    if (type == NPKS_DOWN) {
	switch (ev) {
	case NPEV_NONE:
	    break;

	case NPEV_JOY_UP:
	case NPEV_JOY_DOWN:
	case NPEV_JOY_LEFT:
	case NPEV_JOY_RIGHT:
	case NPEV_JOY_BUTTON_A:
	case NPEV_JOY_BUTTON_B:
	case NPEV_JOY_OPTION:
	    ram[JOYPORT_ADDR] |= joy_mask[ev-NPEV_JOY_UP];
	    break;

	case NPEV_GUI_FRAMESKIP_DECREMENT:
	    if (system_frameskip_key > 0)
		system_frameskip_key--;
	    system_osd("frameskip %d", system_frameskip_key);
	    break;
	case NPEV_GUI_FRAMESKIP_INCREMENT:
	    if (system_frameskip_key < 5)
		system_frameskip_key++;
	    system_osd("frameskip %d", system_frameskip_key);
	    break;
	    
	case NPEV_GUI_FULLSCREEN_OFF:
	    set_fullscreen(0);
	    break;
	case NPEV_GUI_FULLSCREEN_ON:
	    set_fullscreen(1);
	    break;
	case NPEV_GUI_FULLSCREEN_TOGGLE:
	    set_fullscreen(!fs_mode);
	    break;
	    
	case NPEV_GUI_MAGNIFY_1:
	case NPEV_GUI_MAGNIFY_2:
	case NPEV_GUI_MAGNIFY_3:
	    graphics_mag_req = ev-NPEV_GUI_MAGNIFY_1+1;
	    system_osd("magnification %d", graphics_mag_req);
	    break;
	    
	case NPEV_GUI_MENU:
	    /* not implemented yet */
	    break;
	    
	case NPEV_GUI_MUTE_OFF:
	    set_mute(FALSE);
	    break;
	case NPEV_GUI_MUTE_ON:
	    set_mute(TRUE);
	    break;
	case NPEV_GUI_MUTE_TOGGLE:
	    set_mute(!mute);
	    break;

	case NPEV_GUI_PAUSE_ON:
	    set_paused(paused|PAUSED_LOCAL);
	    break;
	case NPEV_GUI_PAUSE_OFF:
	    set_paused(paused&~PAUSED_LOCAL);
	    break;
	case NPEV_GUI_PAUSE_TOGGLE:
	    set_paused(paused^PAUSED_LOCAL);
	    break;

	case NPEV_GUI_QUIT:
	    do_exit = 1;
	    break;
	    
	case NPEV_GUI_SCREENSHOT:
	    system_screenshot();
	    system_osd("screenshot saved");
	    break;
	    
	case NPEV_GUI_SMOOTH_OFF:
	    graphics_mag_smooth = 0;
	    system_osd("smooth off");
	    break;
	case NPEV_GUI_SMOOTH_ON:
	    graphics_mag_smooth = 1;
	    system_osd("smooth on");
	    break;
	case NPEV_GUI_SMOOTH_TOGGLE:
	    graphics_mag_smooth = !graphics_mag_smooth;
	    system_osd("smooth %s", graphics_mag_smooth ? "on" : "off");
	    break;
	    
	case NPEV_GUI_STATE_LOAD:
	    system_state_load();
	    system_osd("state %d loaded", state_slot);
	    break;
	case NPEV_GUI_STATE_SAVE:
	    system_state_save();
	    system_osd("state %d saved", state_slot);
	    break;

	case NPEV_GUI_STATE_SLOT_DECREMENT:
	    state_slot = (state_slot+9)%10;
	    system_osd("slot %d selected", state_slot);
	    break;
	case NPEV_GUI_STATE_SLOT_INCREMENT:
	    state_slot = (state_slot+1)%10;
	    system_osd("slot %d selected", state_slot);
	    break;

	case NPEV_LAST:
	    break;
	}
    }
    else {
	switch (ev) {
	case NPEV_JOY_UP:
	case NPEV_JOY_DOWN:
	case NPEV_JOY_LEFT:
	case NPEV_JOY_RIGHT:
	case NPEV_JOY_BUTTON_A:
	case NPEV_JOY_BUTTON_B:
	case NPEV_JOY_OPTION:
	    ram[JOYPORT_ADDR] &= ~joy_mask[ev-NPEV_JOY_UP];
	    break;

	default:
	    break;
	}
    }
}



static void
set_fullscreen(int val)
{
    system_graphics_fullscreen(val);
    system_osd("fullscreen %s", fs_mode ? "on" : "off");
}



static void
set_mute(int val)
{
    if (!have_sound) {
	system_osd("no sound");
	return;
    }

    mute = val;
    system_osd("sound %s", mute ? "off" : "on");
}



static void
set_paused(int val)
{
    system_osd_pause(val);
    paused = val;
}
