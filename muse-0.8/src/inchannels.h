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
 *
 * different classes for different IN channels
 * they are instantiated and used by the Stream_mixer class (jmixer.cpp)
 */

#ifndef __INCHANNELS_H__
#define __INCHANNELS_H__

#define MP3CHAN 1
#define OGGCHAN 2

#include <pthread.h>

/* muse generic tweakin headers */
#include <generic.h>

#include <pipe.h>

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
  /* decode audio */
  virtual IN_DATATYPE *_get_audio() = 0;

  /* resample */
  IN_DATATYPE *_resample(IN_DATATYPE *audio);
  /* resampling buffer */
  IN_DATATYPE buffo[IN_CHUNK*32];
  /* saved samples for interpolation */
  IN_DATATYPE prev_smp[4];
  /* resample routine pointer */
  Resampler *munch;

  /* pthread stuff */
  void _thread_init();
  void _thread_destroy();
  
  pthread_t _thread;
  pthread_attr_t _attr;

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
  /* ------------- */

 public:
  Channel();
  virtual ~Channel();

  /* set has to return:
     0 = error
     1 = stream is seakable
     2 = stream is not seekable
  */
  virtual int load(char *file) = 0;
  virtual bool pos(float pos) = 0;
  virtual void clean() =0;

  bool play();
  bool stop();
  float upd_time();
  bool set_resampler();

  void report(); // DEBUGGING PURPOSES: call it to print out channel state

  Pipe *erbapipa;

  float volume;
  float position;
  int speed;
  struct timecode time;

  char lcd[64];

  /* 0.0 - 1.0 oppure 2.0 se EOF, 3.0 se errore */
  float state;

  int samplerate;
  int channels;
  int bitrate;
  int frames;
  int samples;
  
  uint8_t playmode;

  bool opened;
  bool on;
  bool paused;
  bool update;
  bool seekable;
  bool running;
  bool quit;
  bool fill_prev_smp;

  int frametot, framepos, fps;  

  int type;

  /* pthread stuff */
  void start() {
    pthread_create(&_thread, &_attr, &kickoff, this); };
  void run();
  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  /* ------------- */

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
