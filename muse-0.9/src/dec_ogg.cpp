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


#include <dec_ogg.h>
#include <jutils.h>
#include <config.h>

#include "httpstream.h"


#ifdef HAVE_VORBIS

/* ----- OggVorbis input channel ----- */

MuseDecOgg::MuseDecOgg() : MuseDec() {
  func("MuseDecOgg::MuseDecOgg()");

  old_section = current_section = -1;

  oggfile = NULL;

  strncpy(name,"Ogg",4);
}

MuseDecOgg::~MuseDecOgg() {
  func("MuseDecOgg::~MuseDecOgg()");
  ov_clear(&vf);
}

IN_DATATYPE *MuseDecOgg::get_audio() {
  int res;

  old_section = current_section;  
  
  if(seekable) {
    /* we check the position here 'cause ov_pcm_tell returns the NEXT frame */
    framepos = ov_pcm_tell(&vf);
    fps = samplerate;
  }

  do {
    res = 
      //      env  buffer  length    BENDIAN WORD SIGNED  bitstream
      ov_read(&vf, _inbuf, IN_CHUNK, 0,      2,   1,      &current_section);
  } while (res == OV_HOLE);
  
  if(res<0) {
    warning("MuseDecOgg:_get_audio() : bitstream error %d", res);
    err = true;
    return(NULL);
  }

  if((res==0)||(old_section != current_section && old_section != -1)) {
    // with this we check when entering into a new logical bitstream
    eos = true;
    return(NULL);
  }

  frames = res>>1;

  return((IN_DATATYPE *)_inbuf);
}

int MuseDecOgg::load(char *file) {
  int res = 0;

  oggfile = hopen(file,"rb");
  if(oggfile==NULL) {
    error("MuseDecOgg::open(%s) : can't open file",file);
    return(res);
  }

  if(ov_open(oggfile,&vf,NULL,0) < 0) {
    error("MuseDecOgg::open(%s): not a valid OggVorbis audio stream",file);
    fclose(oggfile);
    return(res);
  }

  vc = ov_comment(&vf, -1);
  vi = ov_info(&vf, -1);

  samplerate = vi->rate;
  channels = vi->channels;
  seekable = ov_seekable(&vf);

  /* pcm position */
  framepos = 0;

  /* check if seekable */
  res = (ov_seekable(&vf)) ? 1 : 2;
  seekable = (res>1) ? false : true;

  if(seekable)
    frametot = ov_pcm_total(&vf,-1);

  return(res);
}

bool MuseDecOgg::seek(float pos) {

  if(pos==0.0) {
    if(ov_pcm_seek(&vf,1)!=0) {
      error("MuseDecOgg::pos : error in ov_pcm_seek(%p,1)",&vf);
      return(false);
    }

  } else {

    if(ov_pcm_seek_page(&vf,(ogg_int64_t)((double)frametot * pos))!=0) {
      error("MuseDecOgg::play : error in ov_pcm_seek_page(%p,%u)",
	    (ogg_int64_t)((double)frametot * pos));
      return(false);
    }
  }

  return(true);
}

#endif /* HAVE VORBIS */
