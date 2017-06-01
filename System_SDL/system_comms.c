/* $NiH: system_comms.c,v 1.9 2004/07/25 07:21:28 dillo Exp $ */
/*
  system_comms.c -- comm port support functions
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

/* #define COMMS_DEBUG */

#include "NeoPop-SDL.h"
#include "config.h"

int comms_mode;
char *comms_host;
int comms_port;

#ifdef HAVE_LIBSDL_NET
#include <SDL_net.h>

static SDLNet_SocketSet sockset;
static TCPsocket sock;

static int buffered_data;

static void comms_shutdown(void);
#endif

static void comms_write_message(int, _u8);

#define COMMS_DATA	0
#define COMMS_PAUSE	1



BOOL
system_comms_connect(void)
{
#ifndef HAVE_LIBSDL_NET
    if (comms_mode != COMMS_NONE) {
	printf("no comms support\n");
	return FALSE;
    }
    return TRUE;
#else
    IPaddress ip;

    sock = NULL;
    sockset = NULL;
    buffered_data = -1;

    switch (comms_mode) {
    case COMMS_NONE:
	return TRUE;;

    case COMMS_CLIENT:
	if (comms_host == NULL) {
	    printf("no remote host set\n");
	    return FALSE;
	}
	if (SDLNet_ResolveHost(&ip, comms_host, comms_port) == -1) {
	    printf("cannot resolve host `%s': %s\n",
		   comms_host, SDLNet_GetError());
	    return FALSE;
	}
	sock = SDLNet_TCP_Open(&ip);
	if(sock == NULL) {
	    printf("cannot connect to %s:%d: %s\n",
		   comms_host, comms_port, SDLNet_GetError());
	    return FALSE;
	}
	break;

    case COMMS_SERVER:
        {
	    TCPsocket servsock;
	    
	    if (SDLNet_ResolveHost(&ip, NULL, comms_port) == -1) {
		printf("cannot create listen address for port %d: %s\n",
		       comms_port, SDLNet_GetError());
		return FALSE;
	    }
	    servsock = SDLNet_TCP_Open(&ip);
	    if(servsock == NULL) {
		printf("cannot listen on port %d: %s\n",
		       comms_port, SDLNet_GetError());
		return FALSE;
	    }
	    sockset = SDLNet_AllocSocketSet(1);
	    if (sockset == NULL) {
		printf("cannot create socket set: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(servsock);
		comms_shutdown();
		return FALSE;
	    }
	    if (SDLNet_TCP_AddSocket(sockset, servsock) < 0) {
		printf("cannot add socket to socket set: %s\n",
		       SDLNet_GetError());
		SDLNet_TCP_Close(servsock);
		comms_shutdown();
		return FALSE;
	    }
	    if (SDLNet_CheckSockets(sockset, -1) < 1) {
		printf("cannot check socket set: %s\n",
		       SDLNet_GetError());
		SDLNet_TCP_Close(servsock);
		comms_shutdown();
		return FALSE;
	    }
	    sock = SDLNet_TCP_Accept(servsock);
	    SDLNet_TCP_Close(servsock);
	    if (sock == NULL) {
		printf("cannot accept: %s\n", SDLNet_GetError());
		comms_shutdown();
		return FALSE;
	    }
	    SDLNet_TCP_DelSocket(sockset, servsock);
	}
	break;

    default:
	return FALSE;
    }
    
    if (sockset == NULL) {
	sockset = SDLNet_AllocSocketSet(1);
	if (sockset == NULL) {
	    printf("cannot create socket set: %s\n", SDLNet_GetError());
	    comms_shutdown();
	    return FALSE;
	}
    }
    
    if (SDLNet_TCP_AddSocket(sockset, sock) < 0) {
	printf("cannot add socket to socket set: %s\n", SDLNet_GetError());
	    comms_shutdown();
	return FALSE;
    }

#ifdef COMMS_DEBUG
    printf("comms mode %s entered\n", comms_names[comms_mode]);
#endif

    return TRUE;
#endif /* HAVE_LIBSDL_NET */
}

void
system_comms_pause(BOOL pause)
{
    comms_write_message(COMMS_PAUSE, (pause ? 1 : 0));
}

BOOL
system_comms_poll(_u8* buffer)
{
    return system_comms_read(buffer);
}

BOOL
system_comms_read(_u8* buffer)
{
#ifndef HAVE_LIBSDL_NET
    return FALSE;
#else
    _u8 msg[2];
    
    if (sock == NULL)
	return FALSE;

    if (buffered_data != -1) {
#ifdef COMMS_DEBUG
	printf("%s (buffered) byte %02x\n",
	       (buffer ? "read" : "peeked"), buffered_data);
#endif
	if (buffer) {
	    *buffer = buffered_data;
	    buffered_data = -1;
	}
	return TRUE;
    }

    for (;;) {
	switch (SDLNet_CheckSockets(sockset, 0)) {
	case -1:
	    printf("cannot check socket set: %s\n", SDLNet_GetError());
	    return FALSE;

	case 0:
	    return FALSE;

	default:
	    if (SDLNet_TCP_Recv(sock, msg, 2) != 2) {
		printf("read error from socket: %s\n", SDLNet_GetError());
		comms_shutdown();
		return FALSE;
	    }

	    switch (msg[0]) {
	    case COMMS_DATA:
#ifdef COMMS_DEBUG
		printf("%s byte %02x\n",
		       (buffer ? "read" : "peeked"), msg[1]);
#endif
		if (buffer)
		    *buffer = msg[1];
		else
		    buffered_data = msg[1];
		return TRUE;

	    case COMMS_PAUSE:
		/* XXX: handle pause message */
		break;
		
	    default:
		/* XXX: unknown message type */
		break;
	    }
	}
    }
#endif /* HAVE_LIBSDL_NET */
}

void
system_comms_write(_u8 data)
{
    comms_write_message(COMMS_DATA, data);
}



#ifdef HAVE_LIBSDL_NET
static void
comms_shutdown(void)
{
    if (sock) {
	SDLNet_TCP_Close(sock);
	sock = NULL;
    }
    if (sockset) {
	SDLNet_FreeSocketSet(sockset);
	sockset = NULL;
    }
}
#endif /* HAVE_LIBSDL_NET */



static void
comms_write_message(int type, _u8 data)
{
#ifndef HAVE_LIBSDL_NET
    return;
#else
    _u8 msg[2];

    if (sock == NULL) {
	/* XXX: warn? */
	return;
    }

#ifdef COMMS_DEBUG
    if (type == COMMS_DATA)
	printf("wrote byte %02x\n", data);
#endif

    msg[0] = type;
    msg[1] = data;

    if (SDLNet_TCP_Send(sock, msg, 2) != 2) {
	printf("write error on socket: %s\n", SDLNet_GetError());
	comms_shutdown();
    }
#endif /* HAVE_LIBSDL_NET */
}
