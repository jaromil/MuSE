/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 */

#ifndef __OUTCHANNELS_H__
#define __OUTCHANNELS_H__

#include <pthread.h>
#include <profile.h>
#include <shouter.h>
#include <pipe.h>
#include <linklist.h>
#include <generic.h>
#include <resample/samplerate.h>

#define ENCBUFFER_SIZE 128000 // 65536 // we have ram, isn't it?

/* type of the encoder */
enum codec { MP3, OGG };

class OutChannel: public Entry, public Profile {

 private:
  /* pthread properties and private methods */
  void _thread_init();
  void _thread_destroy();
  bool _thread_initialized;
  
  pthread_t _thread;
  pthread_attr_t _attr;

  pthread_mutex_t _mutex;
  pthread_mutex_t _mutex_ice;
  pthread_cond_t _cond;
  /* ------------- */

  int idseed;

 public:
  OutChannel(char *myname);
  virtual ~OutChannel();

  /* INTERESTING THINGS FOR GUI MAKERS FOLLOW:
     ========================================  */

  /* name and version of the encoder
     filled up in the constructor */
  char name[128], version[128];
  enum codec tipo;

  /* la lista dei server icecast (classe Shouter) */
  Linklist icelist;

  /*======== ICECAST SHOUTERS
    crea un server (configurato a default)
    ritorna l'id del server oppure -1 se c'e' errore*/
  int create_ice();
  bool delete_ice(int iceid); /* cancella un server */
  Shouter *get_ice(int iceid); /* ritorna il puntatore ad un server 
			       (per settarlo vedi: shouter.h) */
  bool apply_ice(int iceid); /* applica la configurazione corrente
			     (se gia' connesso, resta connesso) */
  bool connect_ice(int iceid, bool on); /* connette o disconnette il server ID
					a seconda del flag boolean */

  /* ======= ENCODER SETTINGS HERE
     the following macros declare two functions for each variable:
     variable(value); assign value to variable
     variable(); returns value of variable */
  //  struct settings conf[6+1];
  INT_SET(bps,_bps); // Kbit/s
  INT_SET(freq,_freq); // samplerate in Hz
  INT_SET(channels,_channels); // channels (1 is mono, 2 is stereo)
  FLOAT_SET(quality,_quality); /* VALUE from 0.1 to 9.0 */
  INT_SET(lowpass,_lowpass); // lowpass in Hz
  INT_SET(highpass,_highpass); // highpass in Hz

  /* the following is for semi-internal use
     (classes instantiated by the specific outchannels */
  virtual bool apply_profile() =0;
  bool profile_changed;



  /* file dump */
  bool dump_start(char *file);
  bool dump_stop();



  /* ========================================= */
  /* IF YOU ARE DEALING WITH A GUI
     YOU ARE NOT INTERESTED IN WHAT
     COMES NEXT TO HERE */
  /* ========================================= */

  char *guess_bps();
  char quality_desc[256];

  /* this returns the size of the encoded audio in bytes
     TODO: must return the size of the total streamed bytes */
  unsigned int get_bitrate() { return bitrate; };
  
  /* TODO? float buffered() to return buffered percentage */

  /* === virtual functions implemented by the encoder
     used by run() method */
  virtual bool init() = 0;
  virtual int encode() =0;
  virtual void flush() =0;
  /* === */
  int shout(); /* strimma fuori l'encodato a tutti gli icecast - USO INTERNO */
  bool dump(); /* scrive fuori l'encodato nel file - USO INTERNO */
  FILE *fd;
  char fd_name[MAX_PATH_SIZE];

  /* feeds into the pipe in case there is need to encode
     internal use, from jmixer.cpp
     RETURNS: 
         int>0 = quantity of data pushed into the pipe
	 0 = no need to feed in data: no encoding is configured
	 -1 = pipe locked, wait a bit */
  void push(void *data, int len);
  /* this is where it sucks from the audio to be encoded
     internal use! */
  Pipe *erbapipa;

  bool encoding; /* flag checked by run, (streaming||fd) ? true : false */

  /* pthread methods */
  void start();
  void run();
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void lock_ice() { pthread_mutex_lock(&_mutex_ice); };
  void unlock_ice() { pthread_mutex_unlock(&_mutex_ice); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  void destroy() { _thread_destroy(); };
  /* ------------- */



  bool quit;
  bool running;
  bool initialized; /* buffers have been allocated */
  

  /* this is where the encoded ends up, and
     int encoded is how much */
  int16_t buffer[ENC_BUFFER];

 protected:
  static void* kickoff(void *arg) { ((OutChannel *) arg)->run(); return NULL; };

  /* bitrate stuff */
  bool calc_bitrate(int enc);
  int encoded;
  int bitrate;
  double now, prev;
  unsigned int bytes_accu;

};



#endif
