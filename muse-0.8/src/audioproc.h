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
 */

#ifndef __AUDIOPROC_H
#define __AUDIOPROC_H

#include "generic.h"

/* stream mixers */

/* TERMINOLOGY
   samples are 32bit if stereo, 16bit if mono (assuming 16bit sound)
   frames = samples * channels;
   bytes = frames * ( sizeof( typeof( frames ) ) );
*/

/*
  int mixxx_mono_11(int *dest, short *chan, unsigned int num, float volume);
  int mixxx_stereo_11(int *dest, short *chan, unsigned int num, float volume);
  int mixxx_mono_16(int *dest, short *chan, unsigned int num, float volume);
  int mixxx_stereo_16(int *dest, short *chan, unsigned int num, float volume);
*/
#ifdef MOP_RESAMP
int resample_mono_16(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		  unsigned int num, float volume);
int resample_stereo_16(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		  unsigned int num, float volume);
#endif
int resample_mono_22(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		  unsigned int num, float volume);

int resample_stereo_22(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		    unsigned int num, float volume);

#ifdef MOP_RESAMP
int resample_mono_32(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		  unsigned int num, float volume);
int resample_stereo_32(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		  unsigned int num, float volume);
#endif
/*
int mixxx_mono_32(int *dest, IN_DATATYPE *chan, IN_DATATYPE *prev,
		  unsigned int num, float volume);
int mixxx_stereo_32(int *dest, IN_DATATYPE *chan, IN_DATATYPE *prev,
		    unsigned int num, float volume);
*/

int resample_mono_44(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev, 
		     unsigned int num, float volume);

int resample_stereo_44(IN_DATATYPE *dest, IN_DATATYPE *src, IN_DATATYPE *prev,
		       unsigned int num, float volume);

/* souncard in mixer */
int mixxx_stereo_44_novol(int *dest, short *chan, int num);

//int resample(short *in, long int lenght, short *out, int speed);

#endif
