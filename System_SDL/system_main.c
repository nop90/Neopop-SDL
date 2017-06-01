/* $NiH: system_main.c,v 1.50 2004/07/25 10:34:57 dillo Exp $ */
/*
  system_main.c -- main program
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

#include <errno.h>
#include <math.h>
#include <unistd.h>
#include "config.h"
#include "NeoPop-SDL.h"

#include "3ds.h"

char *prg;
struct timeval throttle_last;
_u8 system_frameskip_key;
_u32 throttle_rate;
int do_exit = 0;
int paused = 0;
int have_sound;
void readrc(void);

static void
printversion(void)
{
    printf(PROGRAM_NAME " (SDL) " NEOPOP_VERSION " (SDL-Version "
	   VERSION ")\n");
}

static void
usage(int exitcode)
{
    printversion();
    printf("NeoGeo Pocket emulator\n\n"
	   "Usage: %s [-cefghjMmSsv] [-C mode] [-P port] [-R remove] [game]\n"
	   "\t-C mode\t\tspecify comms mode (none, server, client; default: none)\n"
	   "\t-c\t\tstart in colour mode (default: automatic)\n"
	   "\t-e\t\temulate English language NeoGeo Pocket (default)\n"
	   "\t-f count\tframeskip: show one in `count' frames (default: 1)\n"
	   "\t-g\t\tstart in greyscale mode (default: automatic)\n"
	   "\t-h\t\tshow this short help\n"
	   "\t-j\t\temulate Japanese language NeoGeo Pocket\n"
	   "\t-l state\tload start state from file `state'\n"
	   "\t-M\t\tdo not use smoothed magnification modes\n"
	   "\t-m\t\tuse smoothed magnification modes (default)\n"
	   "\t-P port\t\tspecify port number to use for comms (default: 7846)\n"
	   "\t-R host\t\tspecify host to connect to as comms client\n"
	   "\t-S\t\tsilent mode\n"
	   "\t-s\t\twith sound (default)\n"
	   "\t-v\t\tshow version number\n"
	   "\t-y mode\t\tspecify for which modes to use YUV\n", prg);

    exit(exitcode);
}

void
system_message(char *vaMessage, ...)
{
    va_list vl;

    va_start(vl, vaMessage);
    vprintf(vaMessage, vl);
    va_end(vl);
    printf("\n");
}

void
system_VBL(void)
{
    static int frameskip_counter = 0;
    static int frame_counter = 0;
    static long time_spent = 0;
    static int last_sec = 0;
    struct timeval current_time, time_diff;
    long throttle_diff;
    int newsec;

    system_input_update();
    system_osd_display();

    if (++frameskip_counter >= system_frameskip_key) {
	system_graphics_update();
	frameskip_counter = 0;
    }

    newsec = 0;
    gettimeofday(&current_time, NULL);
    if (current_time.tv_sec != last_sec) {
	newsec = 1;
	last_sec = current_time.tv_sec;
    }

    if (have_sound) {
	timersub(&current_time, &throttle_last, &time_diff);
	throttle_diff = (time_diff.tv_sec*1000000 + time_diff.tv_usec);

	if (time_spent == 0)
	    time_spent = throttle_diff;
	else
	    time_spent = (time_spent*9 + throttle_diff) / 10;

#if 0
	printf("%4ld", time_spent*10/throttle_rate), fflush(stdout);
	printf("time spent: %ld.%06ld, frames spent: %ld\n",
	       time_diff.tv_sec, time_diff.tv_usec, frames_spent);
#endif

	system_sound_update(time_spent/throttle_rate + 1);

	/* XXX: we should include the time spent calculating samples */
	gettimeofday(&current_time, NULL);
	throttle_last = current_time;
    }
    else {
	throttle_last.tv_usec += throttle_rate;
	if (throttle_last.tv_usec > 1000000) {
	    throttle_last.tv_usec -= 1000000;
	    throttle_last.tv_sec++;
	}
	timersub(&throttle_last, &current_time, &time_diff);
	throttle_diff = (time_diff.tv_sec*1000000 + time_diff.tv_usec);
	
	if (throttle_diff > 0) {
	    SDL_Delay(throttle_diff/1000);
	}
	else if (throttle_diff < -2*throttle_rate) {
	    time_diff.tv_sec = 0;
	    time_diff.tv_usec = -2*throttle_rate;
	    timersub(&current_time, &time_diff, &throttle_last);
	}
    }

    frame_counter++;
    if (newsec) {
	char title[128];

	/* set window caption */
	if (graphics_mag_req > 1)
	    (void)snprintf(title, sizeof(title),
			   PROGRAM_NAME " - %s - %dfps/FS%d",
			   rom.name, frame_counter, system_frameskip_key);
	else
	    (void)snprintf(title, sizeof(title), PROGRAM_NAME " %dfps",
			   frame_counter);
	SDL_WM_SetCaption(title, NULL);

	frame_counter = 0;
    }

    return;
}

int
//main(int argc, char *argv[])
main()
{
    char *start_state;
    int ch;
    int i;

//    prg = argv[0];
    start_state = NULL;

    /* some defaults, to be changed by getopt args */
    /* auto-select colour mode */
    system_colour = COLOURMODE_AUTO;
    /* default to English as language for now */
    language_english = TRUE;
    /* default to smooth graphics magnification */
    graphics_mag_smooth = 1;
    /* default to sound on */
    mute = FALSE;
    /* show every frame */
    system_frameskip_key = 1;
    /* interlink defaults (port taken from System_Win32) */
    comms_mode = COMMS_NONE;
    comms_port = 7846;
    comms_host = NULL;
    /* output sample rate */
    samplerate = DEFAULT_SAMPLERATE;
    /* use YUV overlay (hardware scaling) */
    use_yuv = DEFAULT_YUV;
    use_software_yuv = 0;
    /* directories for save files */

    mkdir("/3ds", 0777);
    mkdir("/3ds/neopop", 0777);
    mkdir("/3ds/neopop/flash", 0777);
    mkdir("/3ds/neopop/screenshot", 0777);
    mkdir("/3ds/neopop/state", 0777);
    flash_dir = strdup("/3ds/neopop/flash");
    screenshot_dir = strdup("/3ds/neopop/screenshot");
    state_dir = strdup("/3ds/neopop/state");
    /* use rom name rather than file name for save files */
    use_rom_name = TRUE;//FALSE;
    /* state save slot */
    state_slot = 0;
    /* colour to display OSD in */
    osd_colour = 0xfff;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
       fprintf(stderr, "cannot initialize SDL: %s\n", SDL_GetError());
       exit(1);
    }
    atexit(SDL_Quit);

    if (system_graphics_init() == FALSE) {
		fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
		exit(1);
	}

    system_bindings_init();
    system_rc_read();

//    while ((ch=getopt(argc, argv, "C:cef:ghjl:MmP:R:SsVy:")) != -1) {
//	switch (ch) {
//	case 'C':
//	    i = system_rc_parse_comms_mode(optarg);
//	    if (i == -1) {
//		fprintf(stderr, "%s: unknown comms mode `%s'\n",
//			prg, optarg);
//		exit(1);
//	    }
//	    else
//		comms_mode = i;
//	    break;
//	case 'c':
	    system_colour = COLOURMODE_COLOUR;
//	    break;
//	case 'e':
	    language_english = TRUE;
//	    break;
//	case 'f':
//	    i = atoi(optarg);
//	    if (i <1 || i > 7) {
//		fprintf(stderr, "%s: illegal frame skip `%s'\n",
//			prg, optarg);
//		exit(1);
//	    }		
	    system_frameskip_key = 1;//i;
//	    break;
//	case 'g':
//	    system_colour = COLOURMODE_GREYSCALE;
//	    break;
//	case 'h':
//	    usage(1);
//	    break;
//	case 'j':
//	    language_english = FALSE;
//	    break;
//	case 'l':
//	    start_state = optarg;
//	    break;
//	case 'M':
//	    graphics_mag_smooth = 0;
//	    break;
//	case 'm':
//	    graphics_mag_smooth = 1;
//	    break;
/*
	case 'P':
	    i = atoi(optarg);
	    if (i == 0) {
		fprintf(stderr, "%s: unknown YUV mode `%s'\n",
			prg, optarg);
		exit(1);
	    }
	    else
		comms_port = i;
	    break;
	case 'R':
	    if (comms_host)
		free(comms_host);
	    comms_host = strdup(optarg);
	    break;
	case 'S':
	    mute = TRUE;
	    break;
	case 's':
*/
	    mute = FALSE;
//	    break;
/*
	case 'V':
	    printversion();
	    exit(0);
	    break;
	case 'y':
	    i = system_rc_parse_yuv(optarg);
	    if (i == -1) {
		// XXX: error message 
	    }
	    else
		use_yuv = i;
	    break;
	default:
	    usage(1);
	}
    }

    argc -= optind;
    argv += optind;
*/
    /* Fill BIOS buffer */
    if (bios_install() == FALSE) {
	fprintf(stderr, "cannot install BIOS\n");
	exit(1);
    }
	
    if (SDL_NumJoysticks() > 0) {
	SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);
    }

//    if (system_graphics_init() == FALSE) {
//	fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
//	exit(1);
//    }

    if (mute == FALSE && system_sound_init() == FALSE) {
	fprintf(stderr, "cannot turn on sound: %s\n", SDL_GetError());
	mute = TRUE;
    }
    have_sound = !mute;

    if (system_osd_init() == FALSE)
	fprintf(stderr, "OSD font unusable, OSD disabled\n");

    if (comms_mode != COMMS_NONE)
	system_comms_connect();

    /*
     * Throttle rate is number_of_ticks_per_second divided by number
     * of complete frames that should be shown per second.
     * In this case, both are constants;
     */
    throttle_rate = 1000000/NGP_FPS;

		if (system_rom_load("/roms/neogeopocket/rom.ngc") == FALSE)
			fprintf(stderr, "no ROM loaded\n");
	
    reset();
	
    SDL_PauseAudio(0);
    if (start_state != NULL)
	state_restore(start_state);

    gettimeofday(&throttle_last, NULL);
    do {
	if (paused == 0)
	    emulate();
	else
	    system_VBL();
    } while (do_exit == 0);

    system_rom_unload();
    system_sound_shutdown();

    return 0;
}
