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
 *
 * "$Id$"
 *
 */

#include <in_mpeg.h>
#include <jutils.h>
#include <generic.h>
#include <config.h>


MpegChannel::MpegChannel() : Channel() {
  func("MpegChannel::MpegChannel()");
  type = MP3CHAN;

  loader = NULL;
  server = NULL;
}

MpegChannel::~MpegChannel() {
  func("MpegChannel::~MpegChannel()");
  if(on) stop();
  clean();
}

/* cleans decoder memory but does'nt kills the thread */
void MpegChannel::clean() {
  if(loader) {
    delete loader;
    loader = NULL;
  }

  if(server) {
    delete server;
    server = NULL;
  }
}

int MpegChannel::load(char *file) {
  func("MpegChannel::set(%s)",file);
  int res = 0;
  lock();
  on = false;
  
  if(opened)
    clean();
  
  loader = Soundinputstream::hopen(file,&res);
  if(loader == NULL) {
    warning("MpegChannel::set : i don't see any stream flowing from %s",file);
    clean(); // QUAAAAAAA
    unlock();
    return(0);
  } // else func("MpegChannel->loader %p - OK",loader);

  server = new Mpegtoraw(loader);
  if(server == NULL) {
    warning("MpegChannel::set(%s) - uuops! NULL Mpegtoraw",file);
    clean();
    unlock();
    return(0);
  } // else func("MpegChannel->server %p - OK",server);

  server->initialize(file);

  samplerate = server->getfrequency();
  channels = (server->getmode()<3) ? 2 : 1;

  bitrate = server->getbitrate();

  if(!set_resampler()) {
    error("MpegChannel::set(%s) - argh! set_mixer() reported failure",file);
    clean(); unlock(); return(0);
  }

  /* pcm position */
  framepos = 0;
  /* time position */
  time.h = time.m = time.s = 0;
  /* floating point position */
  position = time.f = 0.0;

  /* check if seekable */
  res = (loader->seekable) ? 1: 2;
  seekable = (loader->seekable) ? true : false;

  /* get samples in a chunk
     framesize is frames in a chunk (samples / channels) */
  samples = server->getpcmperframe();
  framesize = samples * channels;
  if(samplerate<=22050) {
    //   if(channels==2) framesize = framesize>>1;
    framesize /=2;
  }
  //  framesize = server->getpcmperframe();
  //  if(samplerate==44100) framesize = framesize<<1;
  /* ?FIX? division needed for 11khz? */
  //  if(channels==1) framesize = framesize;
  //samples = framesize / channels;

  if(seekable) { /* setup position tracking variables */
    /* total chunks into bitstream (if seekable) */
    frametot = server->gettotalframe();
    /* how much frames make second */
    fps = samplerate/samples;
  }

  opened = true;
  quit = false;
  func("MpegChannel::set bitrate[%u] channels[%u]%sseekable",
       bitrate,channels,(seekable)?" ":" non ");
  //  func("for every chunk there are samples[%i] and bytes[%i]",samples,framesize);
  unlock();

  return(res);
}

IN_DATATYPE *MpegChannel::_get_audio() {

  samples = 0;

  if(!server->run(1)) {
    switch(server->geterrorcode()) {
    case SOUND_ERROR_FINISH:
      state = 2.0;
      return(NULL);
    case SOUND_ERROR_FILEOPENFAIL:
      error("mpeglib: failed opening file (FILEOPENFAIL)");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_FILEREADFAIL:
      error("mpeglib: failed reading from file (FILEREADFAIL)");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_UNKNOWNPROXY:
      error("mpeglib: unknown proxy");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_UNKNOWNHOST:
      error("mpeglib: unknown host");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_SOCKET:
      error("mpeglib: socket error");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_CONNECT:
      error("mpeglib: connect error");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_FDOPEN:
      error("mpeglib: FDOPEN");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_HTTPFAIL:
      error("mpeglib: http failure");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_HTTPWRITEFAIL:
      error("mpeglib: http write failed");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_TOOMANYRELOC:
      error("mpeglib: TOOMANYRELOC");
      state = 3.0;
      return(NULL);
    case SOUND_ERROR_THREADFAIL:
      error("mpeglib: thread failure");
      state = 3.0;
      return(NULL);
    default:
      error("mpeglib: unknown error :(");
      state = 3.0;
      return(NULL);
    }
  }

  /*
    framesize is the number of frames of data in input
    frames stores how much data we have in
    for mp3 it's fixed coz the framesize never changes
    (maybe with VBR does? anyway VBR is no good for streaming) */
  frames = framesize;
  //  samples = frames / channels;

  if(seekable) {
    framepos = server->getcurrentframe();
    state = upd_time();
  } else state = 0.0;

  /* returns a pointer to decoded buffer */
  return((IN_DATATYPE *)server->rawdata);
}

bool MpegChannel::pos(float pos) {

  if(pos==0.0)
    server->setframe(0);
  else
    server->setframe((int) (frametot*pos));
  /* no error codes out of here :( */
  
  time.f = position = pos;

  fill_prev_smp = true;
  
  return(true);
}
