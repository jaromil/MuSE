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
 */

#ifndef __OUT_VORBIS_H__
#define __OUT_VORBIS_H__

#include <outchannels.h>

#include <config.h>
#ifdef HAVE_VORBIS

extern "C" {
#include <vorbis/vorbisenc.h>
}

/* safe bounds! */
#define INBUF OUT_CHUNK*sizeof(float)*8
#define RSMPBUF INBUF*sizeof(float)*12

class OutVorbis : public OutChannel {
 private:
  ogg_stream_state os;
  ogg_page 		 og;
  ogg_packet 		 op;
  
  vorbis_dsp_state vd;
  vorbis_block     vb;
  vorbis_info      vi;
  vorbis_comment   vc;

  ogg_packet header_main;
  ogg_packet header_comments;
  ogg_packet header_codebooks;

  short int header[8192];
  int headersize;

  /* the audio jumps in three buffers, with the resampling in between
     one step could be saved if the resampling routine would result in a bidimensional array
     or if vorbis library could accept interleaved float.
     to save steps i did my best with the pipe class, which now read in intl|bidi float. */
  float *pcm; /* read from pipe to here */
  float *rsmpled; /* resample from pcm to here */
  float **_intbuf; /* process from resampled to a bidimensional array here */
  int8_t *_pbyte;
  
  int out_chunk_len;
  int prepare(float *buf, float **fbuf, int num);
  
  SRC_STATE *rsmp_state;
  SRC_DATA rsmp_data;  

  bool encoder_initialized;

 public:
  OutVorbis();
  ~OutVorbis();
  int encode();
  bool init();
  void flush();
  
  bool apply_profile();


};

#endif
#endif
