/* $NiH: system_sound.c,v 1.22 2004/07/22 00:20:55 wiz Exp $ */
/*
  system_sound.c -- sound support functions
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

#define MAX_FRAMES	60

#define BUFFER_SIZE	(MAX_FRAMES*bpf)

#define FRAME_INC(f)	(((f)+bpf)%BUFFER_SIZE)
#define FRAME_DIFF(a, b)	((((a)-(b))/bpf+MAX_FRAMES)%MAX_FRAMES)

#define DEFAULT_FORMAT		AUDIO_S16 //AUDIO_U16SYS
#define DEFAULT_CHANNELS	1
/* following may need to be set higher power of two */

int samplerate;		/* desired output sample rate */
static int spf;		/* samples per frame */
static int bpf;		/* bytes per frame */
static int dac_bpf;	/* bytes of DAC data per frame */

static SDL_AudioCVT acvt;
static Uint8 silence_value;

static char *sound_buffer;
static char *dac_data;
static int sound_frame_read;		/* read position */
static int sound_frame_read_old;	/* read position at last VBL */
static int sound_frame_write;		/* write position */

static SDL_sem *rsem, *wsem;

void
system_sound_chipreset(void)
{

    sound_init(samplerate);
    return;
}

BOOL
system_sound_init(void)
{
    SDL_AudioSpec desired;

    spf = samplerate/NGP_FPS;
    bpf = spf*2;

    rsem = SDL_CreateSemaphore(0);
    wsem = SDL_CreateSemaphore(0);

    memset(&desired, '\0', sizeof(desired));

    desired.freq = samplerate;
    desired.channels = DEFAULT_CHANNELS;
    desired.format = DEFAULT_FORMAT;
    desired.samples = spf;
    desired.callback = system_sound_callback;
    desired.userdata = NULL;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
	fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
	return FALSE;
    }
    if (SDL_OpenAudio(&desired, NULL) == -1) {
	fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
	return FALSE;
    }

    /* build conversion structure for DAC sound data */
    if (SDL_BuildAudioCVT(&acvt, AUDIO_U8, 1, 8000, DEFAULT_FORMAT,
			  DEFAULT_CHANNELS, samplerate) == -1) {
	fprintf(stderr, "Cannot build converter: %s\n", SDL_GetError());
	SDL_CloseAudio();
	return FALSE;
    }

    dac_bpf = (bpf+acvt.len_mult-1)/acvt.len_mult;

    if ((sound_buffer=malloc(BUFFER_SIZE)) == NULL) {
	fprintf(stderr, "Cannot allocate sound buffer (%d bytes)\n",
		bpf);
	SDL_CloseAudio();
	return FALSE;
    }
    if ((dac_data=malloc(dac_bpf*acvt.len_mult)) == NULL) {
	fprintf(stderr, "Cannot allocate sound buffer (%d bytes)\n",
		bpf+1);
	SDL_CloseAudio();
	free(sound_buffer);
	return FALSE;
    }

    sound_init(samplerate);
    silence_value = desired.silence;
    sound_frame_read = 0;
    sound_frame_read_old = 0;
    sound_frame_write = 0;

    return TRUE;
}

void
system_sound_shutdown(void)
{
    SDL_SemPost(rsem);
    SDL_CloseAudio();
    SDL_DestroySemaphore(rsem);
    SDL_DestroySemaphore(wsem);
    free(sound_buffer);
    sound_buffer = NULL;

    return;
}

void
system_sound_silence(void)
{

    if (mute == TRUE)
	return;

    memset(sound_buffer, silence_value, bpf);
    return;
}

void
system_sound_callback(void *userdata, Uint8 *stream, int len)
{
    if (sound_buffer == NULL)
	return;

    SDL_SemWait(rsem);
    memcpy(stream, sound_buffer+sound_frame_read, len);
    sound_frame_read = FRAME_INC(sound_frame_read);
    SDL_SemPost(wsem);
}

void
system_sound_update(int nframes)
{
    int i;
    int consumed;

    /* SDL_LockAudio(); */

    /* number of unread frames  */
    i = FRAME_DIFF(sound_frame_write, sound_frame_read);
    /* number of frames read since last call */
    consumed = FRAME_DIFF(sound_frame_read, sound_frame_read_old);
    sound_frame_read_old = sound_frame_read;

    /* SDL_UnlockAudio(); */

    for (; i<nframes; i++) {
		if (mute || paused)
			memset(sound_buffer+sound_frame_write, silence_value, bpf);
		else {
			dac_update(dac_data, dac_bpf);
			/* convert to standard format */
			acvt.buf = dac_data;
			acvt.len = dac_bpf;
			if (SDL_ConvertAudio(&acvt) == -1) {
			fprintf(stderr,
				"DAC data conversion failed: %s\n", SDL_GetError());
			return;
			}
			
			/* get sound data */
			sound_update((_u16 *)(sound_buffer+sound_frame_write), bpf);
			
			/* mix both streams into one */
			SDL_MixAudio(sound_buffer+sound_frame_write,
				 dac_data, bpf, SDL_MIX_MAXVOLUME);
		
		
		}
		sound_frame_write = FRAME_INC(sound_frame_write);
		if (sound_frame_write == sound_frame_read) {
			fprintf(stderr, "your machine is much too slow.\n");
			/* XXX: handle this */
			exit(1);
		}
		SDL_SemPost(rsem);
    }

    if (nframes > 1) {
	for (i=0; i<consumed; i++)
	    SDL_SemWait(wsem);
    }
    else
	SDL_SemWait(wsem);
}
