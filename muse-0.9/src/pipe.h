/*
  Copyright (c) 2001 Charles Samuels <charles@kde.org>
  Copyright (c) 2002 Denis Rojo <jaromil@dyne.org>
  
this pipe class was first written by Charles Samuels
and then heavily mutilated by jaromil

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
  
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
   
You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef __PIPE_H__
#define __PIPE_H__

#include <pthread.h>
#include <string.h>
#include <inttypes.h>

#define PIPE_LATENCY 50 /* latency for every read/write operation on pipes */

class Pipe {
public:
  Pipe(int size=16384);
  ~Pipe();
 
  /* blocking read */
  int read(int length, void *data);

  /* float bidimensional array fill routine
     convenient to feed data into float routines */
  int read_float_intl(int samples, float *buf, int channels);
  int read_float_bidi(int samples, float **buf, int channels);

  
  /* mixxing routines */
  int mix16stereo(int samples, int32_t *mix);
  
  //	int peek(int length, void *data, bool block=true) const; // TODO
  int write(int length, void *data);

  void block(bool val);
  bool blocking;

  int size();
  int space();
  
  void flush();
  void flush(int size);
    
 private:
  pthread_mutex_t _mutex;
  void _thread_init() { pthread_mutex_init (&_mutex,NULL); };
  void _thread_destroy() { pthread_mutex_destroy(&_mutex); };
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  
  void *buffer;
  void *bufferEnd;
  
  void *start;
  void *end;

  int pipesize;
};

#endif
