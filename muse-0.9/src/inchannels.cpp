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
#include <errno.h>

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
#ifdef HAVE_JACK
#include <dec_jack.h>
#endif
#include <dec_mp3.h>

#include "httpstream.h"


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
  idle = true;

  _thread_init();

  // setup the pipe
  erbapipa = new Pipe(IN_PIPESIZE);
  erbapipa->set_output_type("mix_int16_to_int32");
  // blocking input and output, default timeout is 200 ms
  erbapipa->set_block(true,true);
  erbapipa->set_block_timeout(400,200);

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
  quit = false;

  wait();

  while(!quit) {
    
    if(on) {
      idle = false;
      PARADEC
	//      dec->lock();

      /* now call get_audio() which
	 returns the *IN_DATATYPE pointer to filled buffer
	 setting up the following parameters:
         dec->state = 0.0-1.0 is the position of the stream
                      2.0 means end of the stream
	  	      3.0 means error decoding stream
	 dec->frames  is updated with number of decoded 16bit frames (double if stereo)
	 dec->samplerate and dec->channels tell about the audio format */
      buff = dec->get_audio();

      //      dec->unlock();

    /* then call resample() which sets up:
       frames = number of 16bit sound values
       and returns *IN_DATATYPE pointing to the resampled buffer */
      if(buff) {
	buff = resample(buff);

	/* at last pushes it up into the pipe
	   bytes are samples<<2 being the audio 16bit stereo */
	erbapipa->write(frames*2,buff);

	/* then calculates the position and time */
	if(dec->seekable) state = upd_time();

      } else /* if get_audio returns NULL then is eos or error */

	if(dec->eos) upd_eos();
	else if(dec->err) upd_err();
	else { // nothing comes out but we hang on
	  //	  error("unknown state on %s channel",dec->name);
	  //	  report(); state = 0.0;
	  jsleep(0,1);
	}

    } else { // if(on)

      // just hang on
      idle = true;
      jsleep(0,1);

    }

    wait();

  } // while(!quit)
  unlock();
  func("Channel :: run :: end thread %d",pthread_self());
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

  /* save last part of the chunk
     for the next resampling */
  prev_smp[0] = audio[dec->frames-4];
  prev_smp[1] = audio[dec->frames-3];
  prev_smp[2] = audio[dec->frames-2];
  prev_smp[3] = audio[dec->frames-1];

  return(buffo);
}

bool Channel::play() {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(on) return(true);

  if(!running) {
    error(_("%i:%s %s channel thread not launched"),
	  __LINE__,__FILE__,__FUNCTION__);
    return(false);
  }

  if(!opened) {
    Url *url;
    warning(_("Channel::play() : no song loaded"));
    url = (Url*) playlist->selected();
    if(!url) {
      warning(_("Channel::play() : no song selected in playlist"));
      url = (Url*)playlist->begin();
      if(!url) {
	error(_("Channel::play() : playlist is void"));
	return(false);
      }
    }

    if( !load( url->path ) ) {
      error(_("Channel::play() : can't load %s"),url->path);
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
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

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
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  MuseDec *ndec = NULL;
  char tmp[256];
  int res;
  /* returns:
     0 = error
     1 = stream is seakable
     2 = stream is not seekable  */ 
  hstream cod = HS_NONE;

  /* parse supported file types */

  snprintf(tmp,256,"%s",file);
  
  if (strstr(file, "http://")) {
    cod = stream_detect(file);
  }

  if(strncasecmp(file+strlen(file)-4,".ogg",4)==0 || cod==HS_OGG) {
#ifdef HAVE_VORBIS
    func("creating Ogg decoder");
    ndec = new MuseDecOgg();
#else
    error(_("Can't open OggVorbis (support not compiled)"));
#endif
  }
  if(strncasecmp(file+strlen(file)-4,".mp3",4)==0 || cod==HS_MP3) {
    func("creating Mp3 decoder");
    ndec = new MuseDecMp3();
  }
  // pallotron: aggiungo lo string compare per i formati sndfile
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
    error(_("Can't open sound file (support not compiled)"));
#endif
  }

  if(strncasecmp(file,"jack://",7)==0) {
#ifdef HAVE_JACK
    func("creating Jack Audio Daemon input client");
    // setup the filename with a MuSE___ prefix
    // for the jack client name
    //    file[0]='M';file[1]='u';file[2]='S';file[3]='E';
    //    file[4]='_';file[5]='_';file[6]='_';
    ndec = new MuseDecJack();

    snprintf(tmp,256,"MuSE_in_%s",&file[7]);
  
#else
    error(_("Jack audio daemon support is not compiled in this version of MuSE"));
#endif
  }
  
  if(!ndec) {
    error(_("Can't open %s (unrecognized extension)"),file);
    return(0);
  }

  lock();

  ndec->lock();
  res = ndec->load(tmp); // try to load the file/stream into the decoder
  ndec->unlock();

  if(!res) { // there is an error: we keep everything as it is
    error(_("decoder load returns error"),file);
    unlock();
    delete ndec;
    return(0);
  }
  
  res = set_resampler(ndec);

  if(!res) {
    error(_("invalid input samplerate %u"),ndec->samplerate);
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

  notice(_("loaded %s"),file);
  if(dec->bitrate)
    notice("%s %s %iHz %s %iKb/s",
	   dec->name,
	   (dec->seekable) ? "file" : "stream",
	   dec->samplerate,
	   (dec->channels==1) ? "mono" : "stereo",
	   dec->bitrate);
  else
    notice("%s %s %iHz %s",
	   dec->name,
	   (dec->seekable) ? "file" : "stream",
	   dec->samplerate,
	   (dec->channels==1) ? "mono" : "stereo");
	   
  return res;
}
     
bool Channel::pos(float pos) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  PARADEC
  if(!dec->seekable) return false;
  pos = (pos<0.0) ? 0.0 : (pos>1.0) ? 1.1 : pos;
  dec->lock();
  if(!dec->seek(pos))
    error(_("error seeking decoder position to %f"),position);
  else
    position = time.f = pos;
  dec->unlock();
  return true;
}

void Channel::clean() {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  on = false;
  //  dec->lock();
  //  dec->clean();
  //  dec->unlock();
  opened = false;
  while(!idle) jsleep(0,20);
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
    warning(_("Can't mix sound at %uhz"),
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
  //  if( ((res-time.f)>0.003) || (time.f-res)>0.003) {
    time.f = res;
    secs = dec->framepos / dec->fps;
    //    func("secs %i",secs);
    if(secs>3600) {
      time.h = (int) secs / 3600;
      secs -= time.h*3600;
    } else time.h = 0;
    if(secs>60) {
      time.m = (int) secs / 60;
      secs -= time.m*60;
    } else time.m = 0;
    time.s = (int) secs;
    update = true;
    //  }

  return(res);
}

void Channel::skip() { /* here just for backward compatibility */
	next();
}

void Channel::next() {
  int selection = playlist->selected_pos();
  if(selection < playlist->len()) sel(selection+1);
  else sel(1);
}

void Channel::prev() {
  int selection = playlist->selected_pos();
  if(selection>1) sel(selection-1);
  else sel(playlist->len());
}

void Channel::sel(int newpos) {
  float st=state;
  if(newpos) {
    switch(playmode) {
    case PLAYMODE_PLAY:
      stop();
      break;
    case PLAYMODE_LOOP:
      pos(0.0);
      break;
	case PLAYMODE_PLAYLIST:
	  if(newpos==1 && playlist->selected_pos()==playlist->len()) break;
	case PLAYMODE_CONT:
      Url *n;
      stop();
      n = (Url*)playlist->pick(newpos);
      if(n && playlist->sel(newpos)) { 
	    while( ! load(n->path) ) {
          n->sel(false); n = (Url*)n->next;
		  if(!n && playmode==PLAYMODE_PLAYLIST) break;
          if(!n) n = (Url*)playlist->begin();
          if(!n) break;
          n->sel(true);
        }
	  }
      if(n) {
        if(st!=0.0) play();
        update = true;
      }
      break;
    default: break;
    }
  }
}

/* called on end of stream, manages playmode */
void Channel::upd_eos() {
  PARADEC
    if(!dec->eos) return;
  func(_("End of %s on %s playing for %i:%i:%i"),
       (seekable)?"stream":"file",dec->name,
       time.h,time.m,time.s);
  skip();
  dec->eos = false;
}

void Channel::upd_err() {
  PARADEC
    if(!dec->err) return;
  error(_("error on %s, skipping %s"),
	dec->name,(seekable)?_("stream"):_("file"));
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

  gotin = (IN_DATATYPE*)malloc((MIX_CHUNK*4) +128);
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

  do {res = read(*dsp,gotin,MIX_CHUNK*4);} while (res==-1 && errno==EINTR);
  return(res/4);
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

  func("Channel | %s | %s | %s | %s |",
       (opened)?"opened":" ",
	 (running)?"running":" ",
	 (on)?"on":"off",
	 (seekable)?"seekable":" ");

  func("vol %.2f pos %.2f lcd[%s]",volume,position,lcd);
  func("state %.2f playmode %s",state,
      (playmode==PLAYMODE_PLAY) ? "PLAY" :
      (playmode==PLAYMODE_LOOP) ? "LOOP" :
      (playmode==PLAYMODE_CONT) ? "CONT" :
      "ERROR");
  if (dec) {
    func("time: %i:%i:%i framepos %i frametot %i",
        time.h, time.m, time.s, dec->framepos,dec->frametot);
    func("samplerate %i channels %i bitrate %i",
        dec->samplerate,dec->channels,dec->bitrate);
    func("frames (16bit) %i",dec->frames);
  } else {
    func("time: %i:%i:%i ", time.h, time.m, time.s);
  }
}

