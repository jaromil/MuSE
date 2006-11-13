/*
  Copyright (c) 2001 Charles Samuels <charles@kde.org>
  Copyright (c) 2002-2004 Denis Rojo <jaromil@dyne.org>
  
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
  
  $Id$

*/

/**
   @file pipe.h
   @brief Buffered and threadsafe FIFO pipe
*/

#ifndef __PIPE_H__
#define __PIPE_H__

#include <pthread.h>
#include <string.h>
#include <inttypes.h>

/**
   This class defines the implementation of the FIFO (first in, first out)
   buffered threadsafe pipe. This component is massively used in MuSE's
   flow of audio data, connecting the Channel to the Stream_mixer and then
   to the OutChannel.
   
   This Pipe implementation is fairly well tested and optimized, usesu
   pthread mutexes for fast atomical locking and has a mixing routine
   embedded for for reading data from the pipe.

   When an application locks the Pipe, will be waiting until it reaches
   to gain the lock, then will  (pthread_mutex_lock)

   This pipe is never blocking, meaning that it never makes the calling
   process wait for result. The Pipe::blocking attribute of this class
   has to be intended in a different way.

   Blocking means that the Pipe::read, Pipe::write and derivative
   functions will be returning -1 without moving any data, in case
   the aumount of data requested don't fit for pipe buffers.
   
   Non-blocking means that the Pipe::read, Pipe::write and derivative
   functions will move all the data they can move, even if less than
   requested, and then return the amount of data moved.

   I know, the above might sound weird, just because i used the word
   'blocking' where i shouldn't. i beg your pardon and plan to change
   it in future.

   @brief fast buffered threadsafe FIFO pipe
*/
class Pipe {
public:

/**
   The constructor initializes all buffers needed, which will be
   in fact the size of the buffer */
  Pipe(int size=16384);
  ///< Pipe constructor class
  
  ~Pipe();
  ///< Pipe destructor class

  /**
     Reads out audio data from the pipe filling up the given buffer
     which has to be properly allocated. 

     If the pipe is set 'blocking' then will return -1 if the pipe
     buffer is full. Otherwise, when non-blocking, will return the
     amount of data that could be pushed in the pipe.
     
     @brief FIFO read from pipe
     @param length amount of data to be read in bytes
     @param data buffer pointer where to read data, must be allocated allready
     @return amount of data read, or -1 on error, if Pipe::blocking is true */
  int read(int length, void *data);
  ///< read from the pipe
 
  /**
     This function is used to read from the pipe directly into a stereo
     interpolated float audio buffer, which is the ideal input for the
     secret rabbit code resampler.

     @brief read from FIFO pipe in a float stereo interpolated l/r array
     @param samples amount of data to be read in  samples (1= l&r float)
     @param buf float buffer to be filled, must be allocated allready
     @param channels number of channels (1=mono, 2=stereo) */
  int read_float_intl(int samples, float *buf, int channels);

  /**
     Read from FIFO pipe in a float bidimensional array.
     It is convenient to read float data for float routines,
     this one reads into a bidimensional buffer.
     TAKE CARE THIS FUNCTION IS NOT WELL TESTED
     i never use it in MuSE i just did it together with the _intl
     
     */
  int read_float_bidi(int samples, float **buf, int channels);
  ///< read from the pipe into a bidimensional float array
   
  /**
     Mixes the audio in the pipe into a given audio buffer.
     Assumes that the audio buffered is 16bit stereo.
     This implements the core mixing routine used in Stream_mixer::cafudda.

     @brief Mix audio from FIFO pipe into a mixing buffer
     @param samples number of samples to be mixed (1= l&r 16bit)
     @param mix 32bit integer mixing buffer
     @return amount of samples mixed */
  int mix16stereo(int samples, int32_t *mix);
  
  //	int peek(int length, void *data, bool block=true) const; // TODO
 
  /**
     Write data inside the FIFO pipe, locking to not overlap read
     operations (thread safe).

     @brief write into FIFO pipe
     @param length amount of bytes of data to be written
     @param data buffer from where to take the data to be written
     @return when Pipe::blocking is set, this functions returns -1, otherwise
     the amount of data written.
    */
  int write(int length, void *data);

  /**
     @brief Setup blocking behaviour of the Pipe.

     The term 'blocking' here is used in a quite improper way, read more
     about in the description of the Pipe class. */
  void block(bool val);

  bool blocking;

  /**
     @brief tell the amount of data contained in the Pipe
     @return amount in bytes */
  int size();
 
  /**
     @brief tell the amount of free space in the Pipe
     @return amount in bytes */
  int space();
  
  void flush(); ///< flush all data contained in the Pipe, loosing it

  void flush(int size);
  ///< flush a certain amount of bytes from FIFO Pipe
    
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
