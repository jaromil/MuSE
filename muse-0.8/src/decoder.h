/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2004 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 
 $Id$"

*/

/**
   @file decoder.h MuSE decoder abstraction
   @desc header file to be included by decoder implementations
*/

#ifndef __DECODER_H__
#define __DECODER_H__

#include <pthread.h>
#include <generic.h>

/**
   This class should be inherited by every decoder implementation.
   
   It defines some pure virtual functions that must then be
   implemented inside the decoder.

   @brief decoder parent abstraction class
*/

class MuseDec { public:

  MuseDec();
  virtual ~MuseDec();

  virtual int load(char *file) = 0; /* open filename */
  virtual bool seek(float pos) = 0; /* seek to position from 0.0 1.0 */
  virtual IN_DATATYPE *get_audio() = 0;  /* decode audio */

  char name[5];
  int samplerate;
  int channels;
  int bitrate;
  int frames;
  int framepos;
  int frametot;
  int fps;

  bool seekable;
  bool eos;
  bool err;

  /* pthread stuff */
  void lock() { pthread_mutex_lock(&mutex); };
  void unlock() { pthread_mutex_unlock(&mutex); };

 private:
  pthread_mutex_t mutex;

};    

#endif
