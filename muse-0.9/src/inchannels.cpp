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
 
"$Id$"
 
 different classes for different IN channels
 they are instantiated and used by the Stream_mixer class (jmixer.cpp)

*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/* mixing and audioprocessing algorithms*/
#include <audioproc.h>

#include <jutils.h>
#include <inchannels.h>
#include <config.h>
#include <generic.h>


/* ----- Parent Class Channel ----- */

typedef void* (kickoff)(void*);

Channel::Channel() {
  func("Channel::Channel()");
  volume = 1.0;
  speed = 100;
  time.h = time.m = time.s = 0;
  position = time.f = 0.0;
  state = 0.0;
  playmode = PLAY;
  opened = false;
  on = false;
  update = false;
  running = false;
  seekable = false;
  rewinding = false;
  bitrate = samplerate = 0;
  quit = true;
  _thread_init();
  erbapipa = new Pipe(IN_PIPESIZE);
  fill_prev_smp = true;
  unlock();
}

Channel::~Channel() {
  func("Channel::~Channel()");

  /* paranoia */
  lock();
  on = false;
  quit = true;
  unlock();
  
  while(running) jsleep(0,100);

  /* clean up specific channel implementation */

  opened = false;
  
  delete erbapipa;

  _thread_destroy();
}

void Channel::run() {
  //  jsleep(0,100);


  IN_DATATYPE *buff; // pointer to buffers to pass them around
  running = true;
  lock();
  func("InChanThread! here i am");
  unlock();
  signal(); // signal to the parent thread we are born!


  if(!opened) {
    warning("InChanThread! Channel::run() : channel uninitialized, thread won't start");
    return;
  }


  while(!quit) {
    lock();

    /* now call get_audio() which sets up:
       state = actual state (0.0-1.0 || 2.0 || 3.0)
       samples = number of decoded samples
       frames = number of decoded 16bit frames
       and returns *IN_DATATYPE pointing to decoded buffer */
    buff = _get_audio();

    if(state>1.0) {
      unlock(); break; }/* end of stream */
    
    /* then call resample() which sets up:
       samples = number of 44khz stereo samples
       and returns *IN_DATATYPE pointing to the resampled buffer */
    buff = _resample(buff);

    unlock();
    
    /* at last pushes it up into the pipe
       bytes are frames<<1 being the audio 16bit */
    while( erbapipa->write
	   (frames<<1,buff) <0
	   && !quit) jsleep(0,10);
  }
  running = false;
}

IN_DATATYPE *Channel::_resample(IN_DATATYPE *audio) {
  int temp = frames;
  /* there is no previous samples saved 
     fill in with the first */
  if(fill_prev_smp) {
    prev_smp[0] = audio[0];
    prev_smp[1] = audio[1];
    prev_smp[2] = audio[2];
    prev_smp[3] = audio[3];
    fill_prev_smp = false;
    erbapipa->flush();
  }

  frames = (*munch)(buffo,audio,prev_smp,temp,volume);
  samples = frames / channels;

  /* save last part of the chunk
     for the next resampling */
  prev_smp[0] = audio[temp-4];
  prev_smp[1] = audio[temp-3];
  prev_smp[2] = audio[temp-2];
  prev_smp[3] = audio[temp-1];

  return(buffo);
}

bool Channel::play() {
  if(on) return(true);
  if(!opened||!running) {
    error("Channel::play() : error starting to play chan opened[%i] running[%i]",
	  opened,running);
    on = false;
    return (on);
  }

  if(time.f!=position) {
    lock();
    if(!pos(position))
      error("Channel::play : error calling Channel::pos(%f)",position);
    position = time.f;
    unlock();
  } else fill_prev_smp = true;
  on = true;
  return(on);
}

bool Channel::stop() {
  lock();
  on = false;
  pos(0.0);
  state = 0.0;
  erbapipa->flush();
  fill_prev_smp = true;
  unlock();
  return(!on);
}

bool Channel::set_resampler() {
  switch(samplerate) {
  case 44100:
    if(channels==2) munch = resample_stereo_44;
    else munch = resample_mono_44;
    break;
  case 32000:
    if(channels==2) munch = resample_stereo_32;
    else munch = resample_mono_32;
    break;
  case 22050:
    if(channels==2) munch = resample_stereo_22;
    else munch = resample_mono_22;
    break;
  case 16000:
    if(channels==2) munch = resample_stereo_16;
    else munch = resample_mono_16;
    break;

    warning("Channel::set_mixer : i can't mix sound at %uhz",samplerate);
    return(false);
  }
  return(true);
}

float Channel::upd_time() {
  long int secs;
  float res;

  update = false;

  res = (float)framepos/(float)frametot;

  if( ((res-time.f)>0.001) || (time.f-res)>0.001) {
    time.f = res;
    secs = framepos/fps;
    
    if(secs>3600) {
      time.h = (int) secs / 3600;
      secs -= time.h*3600;
    }
    if(secs>60) {
      time.m = (int) secs / 60;
      secs -= time.m*60;
    }
    time.s = (int) secs;
    update = true;
  }
  return(res);
}

/* ----- LiveIN DSP Channel ----- */

LiveIn::LiveIn() { 
  on = false;
  gotin = NULL;
}

void LiveIn::init(int smpr, int chans, int *thedsp) {
  dsp = thedsp;
  sample_rate = smpr;
  channels = chans;

  /*
  num_samples=(int)((sample_rate*BUF_SIZE)/(sample_rate<<1));
  opt = (num_samples*channels);
  */

  gotin = (IN_DATATYPE*)malloc((MIX_CHUNK<<2) +128);
}

LiveIn::~LiveIn() {
  if(gotin) free(gotin);
}

int LiveIn::mix(int *mixpcm) {
  int res;

  res = get_audio();
  mixxx_stereo_44_novol(mixpcm,gotin,res);

  return(res);
}


int LiveIn::get_audio() {
  int res;

  res = read(*dsp,gotin,MIX_CHUNK<<2);
  return(res>>2);
}


/* thread stuff */

void Channel::_thread_init() {

  func("Channel::thread_init()");
  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condition"); 
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  
  /* set the thread as detached
     see: man pthread_attr_init(3) */
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);

}

void Channel::_thread_destroy() {
  opened = false;  

  /* we signal and then we check the thread
     exited by locking the conditional */
  while(running) {
    signal();
    lock(); unlock();
  }

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");
}
