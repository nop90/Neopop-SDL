/* $NiH: system_bindings.c,v 1.4 2004/07/10 02:41:26 wiz Exp $ */
/*
  system_bindings.c -- default key bindings
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

#include "NeoPop-SDL.h"

struct binding {
    int key;
    enum neopop_event ev;
};

static struct binding builtin[] = {
    { NPKS_KEY(NPKS_SH_NONE, SDLK_ESCAPE),  NPEV_GUI_QUIT },
    { NPKS_KEY(NPKS_SH_CTRL, 'q'),          NPEV_GUI_QUIT },
    { NPKS_KEY(NPKS_SH_NONE, '1'),          NPEV_GUI_MAGNIFY_1 },
    { NPKS_KEY(NPKS_SH_NONE, '2'),          NPEV_GUI_MAGNIFY_2 },
    { NPKS_KEY(NPKS_SH_NONE, '3'),          NPEV_GUI_MAGNIFY_3 },
    { NPKS_KEY(NPKS_SH_NONE, 'm'),          NPEV_GUI_MUTE_TOGGLE },
    { NPKS_KEY(NPKS_SH_NONE, 'n'),          NPEV_GUI_SMOOTH_TOGGLE },
    { NPKS_KEY(NPKS_SH_NONE, 'v'),          NPEV_GUI_PAUSE_TOGGLE },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_RETURN),   NPEV_GUI_PAUSE_TOGGLE },
    { NPKS_KEY(NPKS_SH_NONE, 'z'),          NPEV_GUI_FULLSCREEN_TOGGLE },
    { NPKS_KEY(NPKS_SH_ALT , SDLK_PAUSE),  NPEV_GUI_FULLSCREEN_TOGGLE },
    { NPKS_KEY(NPKS_SH_NONE, 's'),          NPEV_GUI_FRAMESKIP_DECREMENT },
    { NPKS_KEY(NPKS_SH_NONE, 't'),          NPEV_GUI_FRAMESKIP_INCREMENT },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_F3),      NPEV_GUI_STATE_LOAD },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_F4),      NPEV_GUI_STATE_SAVE },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_F12),     NPEV_GUI_SCREENSHOT },

    { NPKS_KEY(NPKS_SH_NONE, SDLK_UP),      NPEV_JOY_UP },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_DOWN),    NPEV_JOY_DOWN },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_LEFT),    NPEV_JOY_LEFT },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_RIGHT),   NPEV_JOY_RIGHT },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_LSHIFT),  NPEV_JOY_BUTTON_A },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_RSHIFT),  NPEV_JOY_BUTTON_A },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_LCTRL),   NPEV_JOY_BUTTON_B },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_RCTRL),   NPEV_JOY_BUTTON_B },
    { NPKS_KEY(NPKS_SH_NONE, SDLK_TAB),     NPEV_JOY_OPTION },

    { NPKS_KEY(NPKS_SH_NONE, 'u'),          NPEV_JOY_UP },
    { NPKS_KEY(NPKS_SH_NONE, 'j'),          NPEV_JOY_DOWN },
    { NPKS_KEY(NPKS_SH_NONE, 'h'),          NPEV_JOY_LEFT },
    { NPKS_KEY(NPKS_SH_NONE, 'k'),          NPEV_JOY_RIGHT },
    { NPKS_KEY(NPKS_SH_NONE, 'a'),          NPEV_JOY_BUTTON_A },
    { NPKS_KEY(NPKS_SH_NONE, 'b'),          NPEV_JOY_BUTTON_B },
    { NPKS_KEY(NPKS_SH_NONE, 'x'),          NPEV_JOY_OPTION },

    { NPKS_JOY_AXIS(0, 1),                  NPEV_JOY_UP },
    { NPKS_JOY_AXIS(0, 1)+1,                NPEV_JOY_DOWN },
    { NPKS_JOY_AXIS(0, 0),                  NPEV_JOY_LEFT },
    { NPKS_JOY_AXIS(0, 0)+1,                NPEV_JOY_RIGHT },
    { NPKS_JOY_BUTTON(0, 0),                NPEV_JOY_BUTTON_A },
    { NPKS_JOY_BUTTON(0, 1),                NPEV_JOY_BUTTON_B },

    { -1, NPEV_NONE }
};

enum neopop_event bindings[NPKS_SIZE];



void
system_bindings_init(void)
{
    int i;

    for (i=0; builtin[i].key != -1; i++)
	bindings[builtin[i].key] = builtin[i].ev;
}
