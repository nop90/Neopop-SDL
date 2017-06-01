/* $NiH: system_language.c,v 1.5 2003/10/16 17:29:45 wiz Exp $ */
/*
  system_language.c -- language strings needed by NeoPop core
  Copyright (C) 2002-2003 Thomas Klausner

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

/* copied from Win32/system_language.c */
typedef struct {
    char label[9];
    char string[256];
} STRING_TAG;

static STRING_TAG string_tags[]={
    { "SDEFAULT",     "Are you sure you want to revert to the default control setup?" }, 
    { "ROMFILT",      "Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0" }, 
    { "STAFILT",      "State Files (*.ngs)\0*.ngs\0\0" },
    { "FLAFILT",      "Flash Memory Files (*.ngf)\0*.ngf\0\0" },
    { "BADFLASH",     "The flash data for this rom is from a different version of NeoPop, it will be destroyed soon." },
    { "POWER",        "The system has been signalled to power down. You must reset or load a new rom." },
    { "BADSTATE",     "State is from an unsupported version of NeoPop." },
    { "ERROR1",       "An error has occured creating the application window" },
    { "ERROR2",       "An error has occured initialising DirectDraw" },
    { "ERROR3",       "An error has occured initialising DirectInput" },
    { "TIMER",        "This system does not have a high resolution timer." },
    { "WRONGROM",     "This state is from a different rom, Ignoring." },
    { "EROMFIND",     "Cannot find ROM file" },
    { "EROMOPEN",     "Cannot open ROM file" },
    { "EZIPNONE",     "No roms found" } ,
    { "EZIPBAD",      "Corrupted ZIP file" },
    { "EZIPFIND",     "Cannot find ZIP file" },

    { "ABORT",	      "Abort" },
    { "DISCON",	      "Disconnect" },
    { "CONNEC",	      "Connected" }
};

char *
system_get_string(STRINGS string_id)
{
    if (string_id >= STRINGS_MAX)
	return "Unknown String";

    return string_tags[string_id].string;
}
