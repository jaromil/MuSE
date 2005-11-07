/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#ifndef __GENERIC_H__
#define __GENERIC_H__

#include <inttypes.h>

#define PLAYMODE_PLAY 0
#define PLAYMODE_LOOP 1
#define PLAYMODE_CONT 2
#define PLAYMODE_PLAYLIST 3

/* buffer settings (take care!) */
#define IN_DATATYPE int16_t
#define OUT_DATATYPE int16_t
#define MIX_CHUNK 1152 //2048
#define IN_CHUNK MIX_CHUNK
#define IN_PIPESIZE IN_CHUNK*(sizeof(IN_DATATYPE))*64
#define OUT_CHUNK MIX_CHUNK // was MIX_CHUNK, i'm not sure it can safely be something different ...
#define OUT_PIPESIZE OUT_CHUNK*(sizeof(OUT_DATATYPE))*64
#define ENC_BUFFER 1024*1000 
#define PROCBUF_SIZE MIX_CHUNK*5 // mixer process buffers size

/* soundcard tweaks */
#define SAMPLE_RATE 44100 // 44100
#define FRAGSIZE MIX_CHUNK *4
#define FRAGCOUNT 0x7fff /* 0x7fff is supposed to mean no limit
			    (from linux sound programming guide) */


/* mp3 channels - the standard interface can't be changed from here
   uhm... you'd better not touch this, it does not what you wanna do */
#define MAX_CHANNELS 6

/* filesystem */
#define MAX_PATH_SIZE 512

/* talk button goes down that much */
#define TALK_VOLUME 0.3

/* clipping values (MOP patch) */
#define MOP_ADAPTIVE_VOL    1 
/* logging of k activation
   #define MOP_LOGGING         1 */
#ifdef MOP_ADAPTIVE_VOL
/* MOP_ADV_RETM  weight of the last k value in the computation of
                   the current k value (50-200 maybe a valid range) */
#define MOP_ADV_RETM       74.0
/* MOP_ADV_KARE  weight of the current clipped area in the computation
                   of the current k value (10-1000 ???) */
#define MOP_ADV_KARE      200.0
#endif

/* experimental resampling for 22 and 32KHz */
#define MOP_RESAMP    1
//#define WITH_SPEED 1

/* string buffer sizes */
#define MAX_OPTION_SIZE 256 // configuration strings
#define MAX_PATH_SIZE 512 // path strings

// use Portaudio sound device interface
#define PORTAUDIO 1

#endif
