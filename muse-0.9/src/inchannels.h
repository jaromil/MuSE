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
 * "$Id$"
 *
 * different classes for different IN channels
 * they are instantiated and used by the Stream_mixer class (jmixer.cpp)
 
   $Id$

 */

/**
   @file inchannels.h
   @brief Input Channel container class
*/

#ifndef __INCHANNELS_H__
#define __INCHANNELS_H__

#define MP3CHAN 1
#define OGGCHAN 2

#include <pthread.h>

/* muse generic tweakin headers */
#include <generic.h>
#include <decoder.h>
#include <playlist.h>
#include <pipe.h>
#include <sys/time.h>

struct timecode {
  int h;
  int m;
  int s;
  float f;
};

/* resampler function prototype
   instances are in audioproc.cpp */
typedef int (Resampler )(IN_DATATYPE*, IN_DATATYPE*, IN_DATATYPE*, unsigned num, float volume);


/* parent class Channel
   this class shadows codec specific classes to the jmixer
   never instantiated: it's being inherited from decoders
*/

class Channel {

 private:
  /* resample */
  IN_DATATYPE *resample(IN_DATATYPE *audio);
  /* resampling buffer */
  IN_DATATYPE buffo[IN_CHUNK*32];
  /* saved samples for interpolation */
  IN_DATATYPE prev_smp[4];
  /* resample routine pointer */
  Resampler *munch;
  /* AM: need this variable to safely call clean() - to delete the
     decoder/buffers etc. when the run() loop is not using them.  */
  bool idle;

  /* pthread stuff */
  void _thread_init();
  void _thread_destroy();
  pthread_t _thread;
  pthread_attr_t _attr;
  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
  /* ------------- */

  /* total seconds */
  int secs;
  
 public:
  Channel();
  virtual ~Channel();


  bool play();
  bool stop();

  /* the followings are wrappers for methods
     implemented inside the decoder classes
     means that to do a new decoder you have just
     to implement the following public methods in
     your class (inheriting Decoder from decoder.h) */

  MuseDec *dec; /* that's the decoder object superclass
		   the specific implementation is instantiated in load()
		   where it is recognized by parsing the filename
		   @TODO: better ways to recognize file/stream types in Channel */

  /* load returns:
     0 = error
     1 = stream is seakable
     2 = stream is not seekable  */  
  virtual int load(char *file);

  /* seek takes from 0.0 to 1.0 float position */
  virtual bool pos(float pos);

  /* clean cleanups variables and destroys floating
     buffers in the decoder implementation */
  virtual void clean();
  
  /* ============== end of decoder implementation wrappers */


  void skip(); ///< just an accessor to next(). Maintained for backward compatibility 
  void next();  ///< select and load the next track in the playlist 
  void prev(); ///< the same of next() but tries to select the previous track instead of the next
  void sel(int newpos); ///< select and load a specific position in the playlist (if present) 
  
  float upd_time();
  void upd_eos();
  void upd_err();

  bool set_resampler(MuseDec *ndec);

  void report(); // DEBUGGING PURPOSES: call it to print out channel state

  Playlist *playlist;

  Pipe *erbapipa;

  float volume;
  float position;
  int speed;
  struct timecode time;

  char lcd[64];

  /* 0.0 - 1.0 oppure 2.0 se EOF, 3.0 se errore */
  float state;
  /*
  int samplerate;
  int channels;
  int bitrate;
  */
  
  int frames; ///< number of 16bit audio values (double if stereo)

  uint8_t playmode; /* possible values are : 
                     * PLAYMODE_PLAY (play one shot song) , PLAYMODE_LOOP (loop on one song) , 
                     * PLAYMODE_CONT (continuos play), PLAYMODE_PLAYLIST (play entire playlist and then stop) */

  bool opened;
  bool on;
  bool update;
  bool seekable;
  bool running;
  bool quit;
  bool fill_prev_smp;

  //  int frametot, framepos, fps;  

  /* pthread stuff */
  void start() {
    pthread_create(&_thread, &_attr, &kickoff, this); };
  void run();
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  /* ------------- */
  
  long tick_interval;
  struct timeval lst_time; ///< time struct
 protected:
  static void* kickoff(void *arg) { ((Channel *) arg)->run(); return NULL; };

};

/* dsp-in Channel // get sound from soundcard's dsp */
  
class LiveIn {
 private:
  int *dsp;
  unsigned int num_samples;
  int sample_rate;
  int opt;

  int get_audio();  

 public:
  LiveIn();
  ~LiveIn();
  
  void init(int smpr, int chans, int *thedsp);
  int mix(int *mixpcm);

  IN_DATATYPE *gotin;
  int channels;
  int rate;
  bool on;
};

#endif
