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

#ifdef HAVE_VORBIS
#include <dec_ogg.h>
#endif
#ifdef HAVE_SNDFILE
#include <dec_snd.h>
#endif
#include <dec_mp3.h>

//#ifdef DEBUG
#define PARADEC if(!dec) error("%s:%s %i :: decoder is NULL",__FILE__,__FUNCTION__,__LINE__);
//#endif

/* ----- Parent Class Channel ----- */

typedef void* (kickoff)(void*);

Channel::Channel() {
  func("Channel::Channel()");
  volume = 1.0;
  speed = 100;
  time.h = time.m = time.s = 0;
  position = time.f = 0.0;
  state = 0.0;
  playmode = PLAYMODE_PLAY;
  opened = false;
  on = false;
  update = false;
  running = false;
  quit = true;

  _thread_init();
  erbapipa = new Pipe(IN_PIPESIZE);
  playlist = new Playlist();
  dec = NULL;
  fill_prev_smp = true;
  lcd[0] = '\0';
}

Channel::~Channel() {
  func("Channel::~Channel()");

  /* paranoia */
  //stop();
  //  clean();
  quit = true;
  
  while(running) jsleep(0,20);

  /* clean up specific channel implementation */

  delete erbapipa;
  delete playlist;
  if(dec) delete dec;

  _thread_destroy();
}

void Channel::run() {

  IN_DATATYPE *buff; // pointer to buffers to pass them around
  lock();
  func("InChanThread! here i am");
  running = true;
  unlock();
  signal(); // signal to the parent thread we are born!

  quit = false;

  while(!quit) {
    
    if(on) {
      PARADEC
      dec->lock();
      /* now call get_audio() which
	 returns the *IN_DATATYPE pointer to filled buffer
	 setting up the following parameters:
         dec->state = 0.0-1.0 is the position of the stream
                      2.0 means end of the stream
	  	      3.0 means error decoding stream
	 dec->frames  is updated with number of decoded 16bit frames (double if stereo)
	 dec->samplerate and dec->channels tell about the audio format */
      buff = dec->get_audio();
      dec->unlock();
    /* then call resample() which sets up:
       samples = number of 44khz stereo samples
       and returns *IN_DATATYPE pointing to the resampled buffer */
      if(buff) {
	buff = resample(buff);
	/* at last pushes it up into the pipe
	   bytes are samples<<2 being the audio 16bit stereo */
	while( erbapipa->write
	       (samples<<2,buff) <0
	       && !quit) jsleep(0,20);
	/* then calculates the position and time */
	if(dec->seekable) state = upd_time();

      } else /* if get_audio returns NULL then is eos or error */
	if(dec->eos) upd_eos();
	else if(dec->err) upd_err();
	else { /* should never be here */
	  error("unknown state on %s channel",dec->name);
	  report(); state = 0.0;
	} // if(buf) else      

    } else { // if(on)

      // just hang on
      jsleep(0,20);
      
    }
    
  } // while(!quit)
  running = false;
}

IN_DATATYPE *Channel::resample(IN_DATATYPE *audio) {

  /* there is no previous samples saved 
     fill in with the first */
  if(fill_prev_smp) {
    prev_smp[0] = audio[0];
    prev_smp[1] = audio[1];
    prev_smp[2] = audio[2];
    prev_smp[3] = audio[3];
    fill_prev_smp = false;
    //    erbapipa->flush();
  }

  frames = dec->frames;
  frames = (*munch)(buffo,audio,prev_smp,frames,volume);
  samples = frames/2; /// dec->channels;

  /* save last part of the chunk
     for the next resampling */
  prev_smp[0] = audio[dec->frames-4];
  prev_smp[1] = audio[dec->frames-3];
  prev_smp[2] = audio[dec->frames-2];
  prev_smp[3] = audio[dec->frames-1];

  return(buffo);
}

bool Channel::play() {
  if(on) return(true);

  if(!running) {
    error("%i:%s %s channel thread not launched",
	  __LINE__,__FILE__,__FUNCTION__);
    return(false);
  }

  if(!opened) {
    Url *url;
    warning("Channel::play() : no song loaded");
    url = (Url*) playlist->selected();
    if(!url) {
      warning("Channel::play() : no song selected in playlist");
      url = (Url*)playlist->begin();
      if(!url) {
	error("Channel::play() : playlist is void");
	return(false);
      }
    }

    if( !load( url->path ) ) {
      error("Channel::play() : can't load %s",url->path);
      return(false);
    } else url->sel(true);
  }

  if(time.f!=position) {
    pos(position);
  } else fill_prev_smp = true;

  on = true;
  return(on);
}

bool Channel::stop() {
  //  lock();
  on = false;
  if(opened) {
    //  unlock();
    pos(0.0);
    state = 0.0;
    erbapipa->flush();
    fill_prev_smp = true;
  }
  return(!on);
}

int Channel::load(char *file) {
  MuseDec *ndec = NULL;
  int res;
  /* returns:
     0 = error
     1 = stream is seakable
     2 = stream is not seekable  */  

  /* parse supported file types */

  if(strncasecmp(file+strlen(file)-4,".ogg",4)==0) {
#ifdef HAVE_VORBIS
    func("creating Ogg decoder");
    ndec = new MuseDecOgg();
#else
    error("can't open OggVorbis (support not compiled)");
#endif
  }
  if(strncasecmp(file+strlen(file)-4,".mp3",4)==0) {
    func("creating Mp3 decoder");
    ndec = new MuseDecMp3();
  }
  // pallotron: aggiungo lo string compare per i formati
  // sndfile, per ora metto solo voc e wav.
  // TODO: vedere come si chiamano le altre estensioni
  // ed aggiungerle.
  if(strncasecmp(file+strlen(file)-4,".wav",4)==0
     || strncasecmp(file+strlen(file)-4,".aif",4)==0
     || strncasecmp(file+strlen(file)-5,".aiff",4)==0
     || strncasecmp(file+strlen(file)-4,".snd",4)==0
     || strncasecmp(file+strlen(file)-3,".au",4)==0
     || strncasecmp(file+strlen(file)-4,".raw",4)==0
     || strncasecmp(file+strlen(file)-4,".paf",4)==0
     || strncasecmp(file+strlen(file)-4,".iff",4)==0
     || strncasecmp(file+strlen(file)-4,".svx",4)==0
     || strncasecmp(file+strlen(file)-3,".sf",4)==0
     || strncasecmp(file+strlen(file)-4,".voc",4)==0
     || strncasecmp(file+strlen(file)-4,".w64",4)==0
     || strncasecmp(file+strlen(file)-4,".pvf",4)==0
     || strncasecmp(file+strlen(file)-3,".xi",4)==0
     || strncasecmp(file+strlen(file)-4,".htk",4)==0
     || strncasecmp(file+strlen(file)-4,".mat",4)==0
     ) {
#ifdef HAVE_SNDFILE
    func("creating LibSndFile decoder");
    ndec = new MuseDecSndFile();
#else
    error("can't open sound file (support not compiled)");
#endif
  }
  
  if(!ndec) {
    error("can't open %s (unrecognized extension)",file);
    return(0);
  }

  lock();

  ndec->lock();
  res = ndec->load(file); // try to load the file/stream into the decoder
  ndec->unlock();

  if(!res) { // there is an error: we keep everything as it is
    error("decoder load returns error",file);
    unlock();
    delete ndec;
    return(0);
  }
  
  res = set_resampler(ndec);

  if(!res) {
    error("invalid input samplerate %u",ndec->samplerate);
    unlock();
    delete ndec;
    return(0);
  }

  // decoder succesfully opened the file
  // here if res == 2 then we have a stream
  // TODO: oggvorbis stream playing using libcurl
  if(dec) delete dec; // delete the old decoder if present
  dec = ndec;
  opened = true;

  res = (dec->seekable)?1:2;
  seekable = (res==1) ? true : false;
  state = 0.0;
  
  unlock();

  notice("loaded %s",file);
  notice("%s %iHz %s %s %iKb/s",
	 dec->name, dec->samplerate,
	 (dec->channels==1) ? "mono" : "stereo",
	 (dec->seekable) ? "file" : "stream",
	 dec->bitrate);
  return res;
}
     
bool Channel::pos(float pos) {
  PARADEC
  if(!dec->seekable) return false;
  pos = (pos<0.0) ? 0.0 : (pos>1.0) ? 1.1 : pos;
  dec->lock();
  if(!dec->seek(pos))
    error("Channel::seek : error calling decoder seek to %f",position);
  else
    position = time.f = pos;
  dec->unlock();
  return true;
}

void Channel::clean() {
  on = false;
  //  dec->lock();
  //  dec->clean();
  //  dec->unlock();
  opened = false;
  if(dec) delete dec;
  dec = NULL;
}

bool Channel::set_resampler(MuseDec *ndec) {
  switch(ndec->samplerate) {
  case 44100:
    if(ndec->channels==2) munch = resample_stereo_44;
    else munch = resample_mono_44;
    break;
  case 32000:
    if(ndec->channels==2) munch = resample_stereo_32;
    else munch = resample_mono_32;
    break;
  case 22050:
    if(ndec->channels==2) munch = resample_stereo_22;
    else munch = resample_mono_22;
    break;
  case 16000:
    if(ndec->channels==2) munch = resample_stereo_16;
    else munch = resample_mono_16;
    break;
  default:
    warning("Channel::set_mixer : i can't mix sound at %uhz",
	    ndec->samplerate);
    return(false);
  }
  return(true);
}


float Channel::upd_time() {
  PARADEC
  float res;

  /* calculate the float 0.0-1.0 position on stream */
  res = (float)dec->framepos/(float)dec->frametot;

  /* calculate the time */
  if( ((res-time.f)>0.003) || (time.f-res)>0.003) {
    time.f = res;
    secs = dec->framepos / dec->fps;
    //    func("secs %i",secs);
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

void Channel::skip() {
  switch(playmode) {
  case PLAYMODE_PLAY:
    stop();
    break;
  case PLAYMODE_LOOP:
    pos(0.0);
    break;
  case PLAYMODE_CONT:
    Url *n;
    stop();
    n = (Url*)playlist->selected();
    if(n) do {
      n->sel(false); n = (Url*)n->next;
      if(!n) n = (Url*)playlist->begin();
      if(!n) break;
      n->sel(true);
    } while( ! load(n->path) );
    if(n) {
      play();
      update = true;
    }
    break;
  default: break;
  }
}

/* called on end of stream, manages playmode */
void Channel::upd_eos() {
  PARADEC
    if(!dec->eos) return;
  func("end of %s on %s playing for %i:%i:%i",
       (seekable)?"stream":"file",dec->name,
       time.h,time.m,time.s);
  skip();
  dec->eos = false;
}

void Channel::upd_err() {
  PARADEC
    if(!dec->err) return;
    error("error on %s, skipping %s",
	dec->name,(seekable)?"stream":"file");
  report();
  skip();
  dec->err = false;
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


/* here for debugging purposes */
void Channel::report() {

  warning("Channel | %s | %s | %s | %s |",
	 (opened)?"opened":" ",
	 (running)?"running":" ",
	 (on)?"on":"off",
	 (seekable)?"seekable":" ");

  act("vol %.2f pos %.2f lcd[%s]",volume,position,lcd);
  act("state %.2f playmode %s",state,
      (playmode==PLAYMODE_PLAY) ? "PLAY" :
      (playmode==PLAYMODE_LOOP) ? "LOOP" :
      (playmode==PLAYMODE_CONT) ? "CONT" :
      "ERROR");
  if (dec) {
    act("time: %i:%i:%i framepos %i frametot %i",
        time.h, time.m, time.s, dec->framepos,dec->frametot);
    act("samplerate %i channels %i bitrate %i",
        dec->samplerate,dec->channels,dec->bitrate);
    act("frames %i samples %i",dec->frames,samples);
  } else {
    act("time: %i:%i:%i ", time.h, time.m, time.s);
  }
}

