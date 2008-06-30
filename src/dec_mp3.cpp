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

#include <dec_mp3.h>
#include <jutils.h>
#include <config.h>


MuseDecMp3::MuseDecMp3() : MuseDec() {
  loader = NULL;
  server = NULL;
  strncpy(name,"Mp3",4);
}

MuseDecMp3::~MuseDecMp3() {
  if(loader) {
    delete loader;
    loader = NULL;
  }

  if(server) {
    delete server;
    server = NULL;
  }

}

int MuseDecMp3::load(char *file) {
  func("MuseDecMp3::load(%s)",file);
  int res = 0;

  loader = Soundinputstream::hopen(file,&res);
  if(loader == NULL) {
    warning("MuseDecMp3::load : i don't see any stream flowing from %s",file);
    return(0);
  }

  server = new Mpegtoraw(loader);
  if(server == NULL) {
    warning("MuseDecMp3::load(%s) - uuops! NULL Mpegtoraw",file);
    delete loader;
    loader = NULL;
    return(0);
  }

  server->initialize(file);

  samplerate = server->getfrequency();
  channels = (server->getmode()<3) ? 2 : 1;
  bitrate = server->getbitrate();

  /* pcm position */
  framepos = 0;

  /* check if seekable */
  res = (loader->seekable) ? 1: 2;
  seekable = (loader->seekable) ? true : false;

  /* get samples in a chunk
     framesize is frames in a chunk (samples / channels) */
  framesize = server->getpcmperframe() * channels;
  //  framesize = samples * channels;
  if(samplerate<=22050) {
    //   if(channels==2) framesize = framesize>>1;
    framesize /=2;
  }
  
  //framesize = server->getpcmperframe();
  //  if(samplerate==44100) framesize = framesize<<1;
  /* ?FIX? division needed for 11khz? */
  //  if(channels==1) framesize = framesize;
  //samples = framesize / channels;

  if(seekable) { /* setup position tracking variables */
    /* total chunks into bitstream (if seekable) */
    frametot = server->gettotalframe();
    /* how much frames make second */
    fps = samplerate / server->getpcmperframe(); // (framesize / channels);
  }

 
  strncpy(_file,file,MAX_PATH_SIZE); /* HACK! 
					this is normally not needed
					see the seek() method for info */
	  
  return(res);
}



IN_DATATYPE *MuseDecMp3::get_audio() {

  if(!server->run(1)) {
    switch(server->geterrorcode()) {
    case SOUND_ERROR_FINISH:

      if(seekable) {
	framepos = server->getcurrentframe();
	func(_("mpeglib: end of stream at frame position %u"),framepos);
      } else
	func(_("mpeglib: end of stream"));
      eos = true;
      return(NULL);
    case SOUND_ERROR_FILEOPENFAIL:
      error(_("mpeglib: failed opening file (FILEOPENFAIL)"));
      err = true;
      return(NULL);
    case SOUND_ERROR_FILEREADFAIL:
      error(_("mpeglib: failed reading from file (FILEREADFAIL)"));
      err = true;
      return(NULL);
    case SOUND_ERROR_UNKNOWNPROXY:
      error(_("mpeglib: unknown proxy"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_UNKNOWNHOST:
      error(_("mpeglib: unknown host"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_SOCKET:
      error(_("mpeglib: socket error"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_CONNECT:
      error(_("mpeglib: connect error"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_FDOPEN:
      error("mpeglib: FDOPEN");
      err = true;      
      return(NULL);
    case SOUND_ERROR_HTTPFAIL:
      error(_("mpeglib: http failure"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_HTTPWRITEFAIL:
      error(_("mpeglib: http write failed"));
      err = true;      
      return(NULL);
    case SOUND_ERROR_TOOMANYRELOC:
      error("mpeglib: TOOMANYRELOC");
      err = true;      
      return(NULL);
    case SOUND_ERROR_THREADFAIL:
      error(_("mpeglib: thread failure"));
      err = true;      
      return(NULL);
    default:
      error(_("mpeglib: unknown error '%d' :("), server->geterrorcode());
      err = true;      
      return(NULL);
    }
  }

  /*
    framesize is the number of frames of data in input
    frames stores how much data we have in
    for mp3 it's fixed coz the framesize never changes
    (maybe with VBR does?) */
  frames = framesize;
  //  samples = frames / channels;

  if(seekable) {
    framepos = server->getcurrentframe();
  }

  /* returns a pointer to decoded buffer */
  return((IN_DATATYPE *)server->rawdata);
}

bool MuseDecMp3::seek(float pos) {
  func("MuseDecMp3::seek(%f)",pos);
  
  if(pos==0.0) /* HACK! here reload the file
	     this is a problem with the mp3 decoder
	     we must reload in order to seek to start */
    load(_file);
  else
    server->setframe((int) (frametot*pos));
  /* no error codes out of here :( */
  
  return(true);
}
