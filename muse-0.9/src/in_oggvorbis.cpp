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

#include <in_oggvorbis.h>

#ifdef HAVE_VORBIS

#include <jutils.h>
#include <generic.h>

/* ----- OggVorbis Channel ----- */

OggChannel::OggChannel() : Channel() {
  func("OggChannel::OggChannel()");
  type = OGGCHAN;

  old_section = current_section = -1;

  oggfile = NULL;
}

OggChannel::~OggChannel() {
  func("OggChannel::~OggChannel()");
  if(on) stop();
  clean();
}

void OggChannel::clean() {
  ov_clear(&vf);
}

IN_DATATYPE *OggChannel::_get_audio() {
  int res;

  samples = 0;
  old_section = current_section;  
  
  if(seekable)
    /* we check the position here 'cause ov_pcm_tell returns the NEXT frame */
    framepos = ov_pcm_tell(&vf);

  res = 
    ov_read(&vf, _inbuf, IN_CHUNK, 0, 2, 1, &current_section);
  
  if(res<0) {
    warning("OggChannel:_get_audio() : bitstream error");
    state = 3.0;
    return(NULL);
  }

  /*  if((res==0)||(old_section != current_section && old_section != -1)) {
     with this we should check out entering into a new logical bitstream
     left here, will manage later, maybe, one day, oh yes, should do */

  if(res==0) { state = 2.0; return(NULL); }

  frames = res>>1;

  if(seekable)
    state = upd_time();
  else state = 0.0;
  
  return((IN_DATATYPE *)_inbuf);
}

int OggChannel::load(char *file) {
  int res = 0;

  lock();
  on = false;

  if(opened)
    clean();

  oggfile = fopen(file,"rb");
  if(oggfile==NULL) {
    error("OggChannel::set(%s) : can't open file",file);
    unlock();
    return(res);
  }

  if(ov_open(oggfile,&vf,NULL,0) < 0) {
    error("OggChannel::set(%s): not a valid OggVorbis audio stream",file);
    fclose(oggfile);
    unlock();
    return(res);
  }

  vc = ov_comment(&vf, -1);
  vi = ov_info(&vf, -1);

  samplerate = vi->rate;
  channels = vi->channels;
  seekable = ov_seekable(&vf);

  if(!set_resampler()) {
    error("OggChannel::set(%s) : set_mixer() reported failure",file);
    clean();
    unlock();
    return(res);
  } 

  /* pcm position */
  framepos = 0;
  /* that's a bit shamanic but works fine for me */
  fps = samplerate;
  /* time position */
  time.h = time.m = time.s = 0;
  /* floating point position */
  position = time.f = 0.0;

  /* check if seekable */
  res = (ov_seekable(&vf)) ? 1 : 2;
  seekable = (res>1) ? false : true;

  if(seekable)
    frametot = ov_pcm_total(&vf,-1);

  opened = true;
  quit = false;

  func("OggChannel::set : samplerate[%u] channels[%u]%sseekable",
       samplerate,channels,(seekable)?" ":" non ");
  
  unlock();
 
  return(res);
}

bool OggChannel::pos(float pos) {

  if(pos==0.0) {
    if(ov_pcm_seek(&vf,1)!=0) {
      error("OggChannel::pos : error in ov_pcm_seek(%p,1)",&vf);
      return(false);
    }
  } else {
    if(ov_pcm_seek_page(&vf,(ogg_int64_t)((double)frametot * pos))!=0) {
      error("OggChannel::play : error in ov_pcm_seek_page(%p,%u)",
	    (ogg_int64_t)((double)frametot * position));
      return(false);
    }
  }
  time.f = position = pos;

  fill_prev_smp = true;
  
  return(true);
}

#endif /* HAVE VORBIS */
