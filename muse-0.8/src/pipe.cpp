/*
  Copyright (c) 2001 Charles Samuels <charles@kde.org>
  Copyright (c) 2002 - 2003 Denis Rojo <jaromil@dyne.org>
  
this pipe class was first written by Charles Samuels
and then heavily mutilated and optimized by Denis Rojo

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

"$Id$"

*/

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <audioproc.h>
#include <pipe.h>
#include <jutils.h>
#include <config.h>


#define MIN(a,b) (a<=b) ? a : b; 

#define _SIZE(val) \
  int val; \
  if ((char*)end >= (char*)start) \
    val = (char*)end-(char*)start; \
  else  \
    val = ((char*)bufferEnd-(char*)start)+((char*)end-(char*)buffer);

#define _SPACE(val) \
  _SIZE(__size); \
  val = ((char*)bufferEnd-(char*)buffer)-__size;

/*
  start is a pointer to the first character that goes out
  end is a pointer to the last character to go out
*/

Pipe::Pipe(int size) {
  func("Pipe::Pipe(%i)",size);
  pipesize = size;
  buffer = malloc(pipesize);
  if(!buffer)
    error("FATAL: can't allocate %i bytes buffer for audio Pipe: %s",
	  pipesize, strerror(errno));
  bufferEnd=(char*)buffer+size;
  end=start=buffer;
  blocking = true;
  _thread_init();
  unlock();
}

Pipe::~Pipe() {
  func("Pipe::~Pipe : freeing %p",buffer);
  lock();
  free(buffer);
  unlock();
  //  _thread_destroy();
}


int Pipe::read_float_intl(int samples, float *buf, int channels) {
  lock();
  _SIZE(buffered);
  int c, cc;
  int length = samples<<2;
  float *pp = buf;

  if (buffered<length) {
    //    func("Pipe::read_float(%i,%p) : only %i bytes in the pipe",
    //          samples,buf,buffered);
    unlock();
    return -1;
  }

  int origLength=length;
  
  while (buffered && length)	{
				
    /* |buffer*****|end-----------|start********|bufferEnd
       |buffer-----|start*********|end----------|bufferEnd */
    
    int len = MIN(length,buffered);
    
    int currentBlockSize = (char*)bufferEnd-(char*)start;
    currentBlockSize=MIN(currentBlockSize,len);

    /* fill */
    cc = currentBlockSize; //>>2 QUAAA
    switch(channels) {
    case 1:
      for(c=0; c<cc; c++)
	pp[c] = (((IN_DATATYPE*)start)[c*2] +
		 ((IN_DATATYPE*)start)[c*2+1]) / 65536.0f; // /2 /32768.0f
      break;
    case 2:
      for(c=0; c<cc; c++) {
	//	pp[c*2] = ((IN_DATATYPE*)start)[c*2] /32768.0f;
	//	pp[c*2+1] = ((IN_DATATYPE*)start)[c*2+1] /32768.0f;
	pp[c] = ((IN_DATATYPE*)start)[c] / 32768.0f;
      }
      break;
    default: break;
    }
    /* --- */

    (char*)start += currentBlockSize;
    len -= currentBlockSize;
    (char*)pp += currentBlockSize;
    length -= currentBlockSize;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    
    if (len) { /* short circuit */
      
      /* fill */
      cc = len; // >>2 QUAAA
    switch(channels) {
    case 1:
      for(c=0; c<cc; c++)
	pp[c] = (((IN_DATATYPE*)start)[c*2] +
		 ((IN_DATATYPE*)start)[c*2+1]) / 65536.0f; // /2 /32768.0f
      break;
    case 2:
      for(c=0; c<cc; c++) {
	//	pp[c*2] = ((IN_DATATYPE*)start)[c*2] /32768.0f;
	//	pp[c*2+1] = ((IN_DATATYPE*)start)[c*2+1] /32768.0f;
	pp[c] = ((IN_DATATYPE*)start)[c] / 32768.0f;
      }
      break;
    default: break;
    }
    /* --- */
    
    (char*)pp += len;
    (char*)start += len;
    length -= len;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    }
  }
  
  unlock();
  return (origLength-length)>>2;
}  



/* this takes SAMPLES, they can be stereo or mono
   mono will be averaged between the two channels
   it is supposed that the pipes always contains 16bit STEREO (length = samples<<2)
   
   RETURNS: samples read
 */
int Pipe::read_float_bidi(int samples, float **buf, int channels) {
  lock();

  _SIZE(buffered);

  /* if nothing is in, returns -1 */
  if(!buffered) {
    unlock();
    func("Pipe:read_float_bidi(%i,%p,%i) : nothing in the pipe",
	 samples, buf, channels);
    return -1;
  }

  int c, cc;
  int length = samples<<2;
  float **pp = buf;

  if (buffered<length)
    if(blocking) {
      unlock();
      func("Pipe::read_float_bidi(%i,%p,%i) : only %i bytes in the pipe",
	   samples,buf,channels,buffered);
      return -1;
    } else
      length = buffered;

  int origLength=length;
  
  while (buffered && length)	{
				
    /* |buffer*****|end-----------|start********|bufferEnd
       |buffer-----|start*********|end----------|bufferEnd */
    
    int len = MIN(length,buffered);
    
    int currentBlockSize = (char*)bufferEnd-(char*)start;
    currentBlockSize=MIN(currentBlockSize,len);

    /* fill */
    cc = currentBlockSize>>2;
    switch(channels) {
    case 1:
      for(c=0; c<cc; c++)
	pp[0][c] = (((IN_DATATYPE*)start)[c*2] +
		    ((IN_DATATYPE*)start)[c*2+1]) / 65536.0f; // /2 /32768.0f
      break;
    case 2:
      for(c=0; c<cc; c++) {
	pp[0][c] = ((IN_DATATYPE*)start)[c*2] /32768.0f;
	pp[1][c] = ((IN_DATATYPE*)start)[c*2+1] /32768.0f;
      }
      break;
    default:
      error("Pipe:read_float_bidi doesn't supports %i channels",channels);
      break;
    }
    /* --- */

    (char*)start += currentBlockSize;
    len -= currentBlockSize;
    (char*)pp += currentBlockSize;
    length -= currentBlockSize;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    
    if (len) { /* short circuit */
      
      /* fill */
      cc = len>>2;
      switch(channels) {
      case 1:
	for(c=0; c<cc; c++)
	  pp[0][c] = (((IN_DATATYPE*)start)[c*2] +
		      ((IN_DATATYPE*)start)[c*2+1]) / 65536.0f; // /2 /32768.0f
	break;
      case 2:
	for(c=0; c<cc; c++) {
	  pp[0][c] = ((IN_DATATYPE*)start)[c*2] /32768.0f;
	  pp[1][c] = ((IN_DATATYPE*)start)[c*2+1] /32768.0f;
	}
	break;
      default:
	error("Pipe:read_float_bidi doesn't supports %i channels",channels);
	break;
      }
      /* --- */
      
      (char*)pp += len;
      (char*)start += len;
      length -= len;
      if ((end!=buffer) && (start==bufferEnd))
	start = buffer;
    }
  }
  
  unlock();
  return (origLength-length)>>2;
}  


void Pipe::block(bool val) {
  lock();
  blocking = val;
  unlock();
}
    

int Pipe::mix16stereo(int samples, int32_t *mix) {
  lock();
  /* int buffered = size(); */
  _SIZE(buffered);
  int c, cc;

  int length = samples<<2;
  int32_t *pp = mix;

  //  if (!buffered) return -1;  

  if (buffered<length) {
    //    func("Pipe::mix16stereo(%i,%p) : only %i bytes in the pipe",
    //    samples,mix,buffered);
    unlock();
    return -1;
  }
  
  int origLength=length;
  
  while (buffered && length)	{
				
    /* |buffer*****|end-----------|start********|bufferEnd
       |buffer-----|start*********|end----------|bufferEnd */
    
    int len = MIN(length,buffered);
    
    int currentBlockSize = (char*)bufferEnd-(char*)start;
    currentBlockSize=MIN(currentBlockSize,len);

    /* mix */
    cc = currentBlockSize>>1;
    for(c=0; c<cc ;c++)
      pp[c] += (int32_t) ((IN_DATATYPE*)start)[c];
    /* --- */

    (char*)start += currentBlockSize;
    len -= currentBlockSize;
    (char*)pp += currentBlockSize;
    length -= currentBlockSize;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    
    if (len) { /* short circuit */
      
      /* mix */
      cc = len>>1;
      for(c=0; c<cc ;c++)
	pp[c] += (int) ((IN_DATATYPE*)start)[c];
      /* --- */

      (char*)pp += len;
      (char*)start += len;
      length -= len;
      if ((end!=buffer) && (start==bufferEnd))
	start = buffer;
    }
  }

  unlock();
  return origLength-length;
}  
	
int Pipe::read(int length, void *data) {
  lock();

  _SIZE(buffered);

  /* if nothing is in, returns -1 */
  if(!buffered) {
    unlock();
    return -1;
  }

  /* if less than desired is in, then 
     (blocking) returns -1
     (non blocking) returns what's available */
  if (buffered<length)
    if(blocking) {
      unlock();
      return -1;
    } else
      length = buffered;

  
  int origLength=length;
  
  while (buffered && length)	{
				
    /* |buffer*****|end-----------|start********|bufferEnd
       |buffer-----|start*********|end----------|bufferEnd */
    
    int len = MIN(length,buffered);
    
    int currentBlockSize = (char*)bufferEnd-(char*)start;
    currentBlockSize=MIN(currentBlockSize,len);
    /* fill */
    memcpy(data, start, currentBlockSize);
    
    (char*)start += currentBlockSize;
    len -= currentBlockSize;
    (char*)data += currentBlockSize;
    length -= currentBlockSize;
    if ((end!=buffer) && (start==bufferEnd))
      start = buffer;
    
    if (len) { /* short circuit */
      memcpy(data, start, len);
      (char*)data += len;
      (char*)start += len;
      length -= len;
      if ((end!=buffer) && (start==bufferEnd))
	start = buffer;
    }
  }
  
  unlock();
  return origLength-length;
}

int Pipe::write(int length, void *data) {
  int spc;
  lock();

  _SPACE(spc);
  if (length>(spc-1)) {
    //    warning("Pipe::write(%i,%p) : not enough space in the pipe (spc=%i)",length,data,spc);
//    fprintf(stderr,".");
    unlock();
    return -1;
  }

  int origLength=length;

  while (length) {
    
    /* |buffer-----|end***********|start--------|bufferEnd
       |buffer*****|start---------|end**********|bufferEnd */
    //    _SPACE(spc);
    int len=MIN(length, spc-1);
    
    int currentBlockSize = (char*)bufferEnd-(char*)end;
    currentBlockSize=MIN(currentBlockSize, len);
    ::memcpy(end, data, currentBlockSize);
    
    (char*)end += currentBlockSize;
    
    len -= currentBlockSize;
    
    (char*)data += currentBlockSize;
    length -= currentBlockSize;
    if ((start!=buffer) && (end==bufferEnd))
      end = buffer;
		
    if (len) { // short circuit		
      ::memcpy(end, data, len);
      (char*)data += len;
      (char*)end += len;
      length -= len;
      
      if ((start!=buffer) && (end==bufferEnd))
	end = buffer;
    }
  }
  unlock();
  return origLength-length;
}

// |buffer******|end--------------|start**************|bufferEnd
// |buffer-----|start**************|end---------------|bufferEnd
int Pipe::size() {
  /* size macro allocates the result variable by itself */
  lock();
  _SIZE(res);
  unlock();
  return res;
}

// |buffer------|end**************|start--------------|bufferEnd
// |buffer*****|start--------------|end***************|bufferEnd
int Pipe::space() {
  int res;
  lock();
  _SPACE(res);
  unlock();
  return res;
}

void Pipe::flush() {
  lock();
  bufferEnd=(char*)buffer+pipesize;
  end=start=buffer;
  unlock();
}

void Pipe::flush(int bytes) {
  lock();
  void *temp = malloc(bytes);
  read(bytes, temp);
  free(temp);
  unlock();
}
