/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <out_vorbis.h>
#ifdef HAVE_VORBIS

#include <jutils.h>
#include <generic.h>
#include <config.h>

OutVorbis::OutVorbis()
  : OutChannel("vorbis") {
  func("OutVorbis::OutVorbis() %p",this);

  headersize = 0;

  sprintf(name,"Ogg/Vorbis encoder");
  sprintf(version," ");

  tipo = OGG;

  /* initialize resampler */

  pcm = (float*) malloc(INBUF);
  rsmpled = (float*) malloc(RSMPBUF);
  rsmp_state = NULL;

  encoder_initialized = false;
}


bool OutVorbis::init() {
  func("OutVorbis::init() %p",this);

  //  vorbis_info_init(&vi);  

  //  act("initializing %s %i",name,vi.version);

  
  if(!apply_profile()) {
    error("problems in setting up codec parameters");
    //    vorbis_info_clear(&vi);
    return false;
  }

  erbapipa->set_output_type("copy_int16_to_float");

  initialized = true;
  return initialized;
}

/* note: num is number of samples in the L (or R) channel
   not the total number of samples in pcm[] */
int OutVorbis::encode() {  
  int num = 0, samples = 0;
  int rsmperr, res;
  _pbyte = (int8_t*)buffer;

  if(headersize) {
    memcpy(buffer,header,headersize);
    encoded = headersize;
    shout(); dump();
  }
  
  encoded = headersize = 0;

  if(erbapipa->size() < out_chunk_len) { // bytes, not samples, 16bit stereo stuff
    //    func("OutVorbis::encode : not enough data in pipe");
    return -1;
  }

  /* this now takes bytes and returns bytes */
  num = erbapipa->read(OUT_CHUNK,pcm);
  // QUAA or?:   num = erbapipa->read(OUT_CHUNK*2,pcm);


  if(num<OUT_CHUNK)
    func("OutVorbis::encode() : erbapipa->read_float_bidi reads %i samples instead of %i",
	 num,OUT_CHUNK);
  if(num<1) return num;
  
  rsmp_data.data_in = pcm;
  rsmp_data.input_frames = num;
  rsmp_data.data_out = rsmpled;
  rsmp_data.output_frames = RSMPBUF;

  /* do the resampling */
  rsmperr = src_process(rsmp_state,&rsmp_data);
  if(rsmperr) {
    error("error in ogg resampler: %s",src_strerror(rsmperr));
    if(rsmperr==6) error("resampling ratio is %.4f",rsmp_data.src_ratio);
  }

  samples = rsmp_data.output_frames_gen;
  //func("%i frames resampled in %i with ratio %.4f (num:%u OUT_CHUNK:%u)",
    //   rsmp_data.input_frames_used, samples, rsmp_data.src_ratio, num, OUT_CHUNK);

  /* initialize the vorbis encoder to work on the resampled size */
  _intbuf = vorbis_analysis_buffer(&vd,samples);
  /* move resampled pcm into the vorbis float array 
     i tryed to avoid in every way this extra mov of memory
     but there is no way to predict the resampled buffer size exactly
     before actually doing the resampling. this prevents from initializing
     the vorbis buffer BEFORE doing the resampling, to store the resampled
     pcm directly into it (both the output of the resampler and the input
     of the vorbis encoder take a bidimensional float array).
     currently this causes a bit of overhead, but is the only way. */
  prepare(rsmpled,_intbuf,samples);
  res = vorbis_analysis_wrote(&vd,samples);
  if(res) { /* nonzero is error */
    error("OutVorbis::encode : error %i from vorbis_analysis_wrote");
    return 0; }
  
  /* encode one block at a time... */
  while(vorbis_analysis_blockout(&vd,&vb)==1) {
    /* Do the main analysis, creating a packet */
    res = vorbis_analysis(&vb, NULL); /* here as a second argument darkice uses 
					 &op while oggenc uses NULL */
    if(res) { /* nonzero is error */
      error("OutVorbis::encode : error %i from vorbis_analysis_wrote");
      return 0;
    }
  
    res = vorbis_bitrate_addblock(&vb);
    if(res) { /* nonzero is error */
      error("OutVorbis::encode : error %i from vorbis_bitrate_addblock");
      return 0;
    }

    while(vorbis_bitrate_flushpacket(&vd, &op)) {
      /* Add packet to bitstream */
      res = ogg_stream_packetin(&os,&op);
      if(res) { /* nonzero is error */
	error("OutVorbis::encode : error %i from ogg_stream_packetin");
	return 0;
      }

      while(ogg_stream_pageout(&os,&og)) { 
	memcpy(_pbyte,og.header,og.header_len);
	_pbyte += og.header_len;
	memcpy(_pbyte,og.body,og.body_len);
	_pbyte += og.body_len;
	
	encoded += og.header_len + og.body_len;

	/* TODO: use throw / catch mechanism for EOS */
	//	if(ogg_page_eos(&og)) { func("Ogg encoder EOS!"); return encoded; }
      }
      
    }
  }
  return encoded;
}

int OutVorbis::prepare(float *buf, float **fbuf, int num) {
  // this function distributes interlaced data in two l/r arrays

  int i=0;
  switch(channels()) {
  case 1:
    for(i=num-1; i>=0; i--)
      fbuf[0][i] = buf[i]; // short buf: (buf[i*2] + buf[i*2+1]) / 65536.0f; // /2 /32768.0f;
    break;
  case 2:
    for(i=0; i<num; i++) {
      fbuf[0][i] = buf[i<<1]; // short buf: buf[i*2] /32768.0f;
      fbuf[1][i] = buf[(i<<1)+1]; // short buf: buf[i*2+1] /32768.0f;
    }
    break;
  default:
    error("error in OutVorbis::prepare");
    error("no more than 2 audio channels are supported");
    break;
  }

  return i;

  /*
    the way oggenc does this is by using a 16bit large pointer and shifting around: 
    fbuf[0][i] = ( ( p[i*4+1] << 8 ) | ( p[i*4] & 0xff ) ) / 32768.0f;
    fbuf[1][i] = ( ( p[i*4+3] << 8 ) | ( p[i*4+4] & 0xff ) ) / 32768.0f;
    (or something like this, check it out in oggenc/audio.c to be sure)
  */
}


void OutVorbis::flush() {
  //  lock();
  vorbis_analysis_wrote(&vd,0); /* oggenc L255 */
  //  unlock();
}

bool OutVorbis::apply_profile() {
  func("OutVorbis::apply_profile() q%.4f r%i b%i c%i",
       _quality,freq(),bps(),channels());
  bool res = true;
  int rsmp_err = 0;

  if(rsmp_state) src_delete(rsmp_state);
  // SRC_SINC_(BEST|MEDIUM)_QUALITY or SRC_SINC_FASTEST
  rsmp_state = src_new(SRC_SINC_FASTEST, channels() ,&rsmp_err);
  if(!rsmp_state)
    error("Ogg/Vorbis can't initialize resampler: %s",src_strerror(rsmp_err));
  else func("ogg resampler %s initialized",src_get_name(SRC_SINC_FASTEST));
  
  /* set ratio for resampling with ogg vorbis */
  {
    double ratio = (double)((float)freq() / 44100.0f);
    rsmp_data.src_ratio = ratio/2; // input is always stereo
  }
  if(!src_is_valid_ratio(rsmp_data.src_ratio))
    error("invalid resampling ratio: %.4f", rsmp_data.src_ratio);
  func("resample ratio for freq %i is %.4f", freq(), rsmp_data.src_ratio);


  ////////////////////////////////////
  /// vorbis encoder initialization
  if(encoder_initialized) {
    // clear the encoder for new initialization
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_info_clear(&vi);
  } else encoder_initialized = true;

  vorbis_info_init(&vi);
  if( vorbis_encode_init
      (&vi, channels(), freq(), bps()*1000, bps()*1000, bps()*1000) ) {
    error("vorbis_encode_init failed: invalid parameters");
    return false;
  }
  /* Now, set up the analysis engine, stream encoder, and other
     preparation before the encoding begins. */
  vorbis_analysis_init(&vd,&vi);
  vorbis_block_init(&vd,&vb);
  ogg_stream_init(&os, time(NULL));

  /* sets up our comments */
  {
    char tmp[128];
    sprintf(tmp,"%s version %s",PACKAGE,VERSION);
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc,"Streamed with",tmp);
  }

  /* Now, build the three header packets and send through to the stream 
     output stage (but defer actual file output until the main encode loop) */

  /* Build the packets */
  vorbis_analysis_headerout
    (&vd,&vc,&header_main,&header_comments,&header_codebooks);

  /* And stream them out */
  ogg_stream_packetin(&os,&header_main);
  ogg_stream_packetin(&os,&header_comments);
  ogg_stream_packetin(&os,&header_codebooks);
  
  /* write out headers */
  headersize = 0;
  _pbyte = (int8_t*)header; 
  while(ogg_stream_flush(&os,&og)) {
    memcpy(_pbyte,og.header,og.header_len);
    _pbyte += og.header_len;
    memcpy(_pbyte,og.body,og.body_len);
    _pbyte += og.body_len;
    headersize += og.header_len + og.body_len;
  }

  vorbis_comment_clear(&vc);
  ////////////////////////////////////////



  Shouter *ice = (Shouter*)icelist.begin();
  while(ice) {
    char tmp[256];

    snprintf(tmp,256,"%u",bps());       ice->bps( tmp );
    snprintf(tmp,256,"%u",freq());      ice->freq( tmp );
    snprintf(tmp,256,"%u",channels());  ice->channels( tmp );
    //    snprintf(tmp,256,"%.2f",_quality);  ice->quality( tmp );
    
    ice = (Shouter*)ice->next;
  }

  /* to save this calculation */
  out_chunk_len = OUT_CHUNK * sizeof(float) * 2;

  /* Turn off management entirely (if it was turned on) */
  //  vorbis_encode_ctl(&vi, OV_ECTL_RATEMANAGE_SET, NULL);
  
  //  if( vorbis_encode_setup_managed
  //      (&vi, channels(), freq(), -1, bps()*1000, -1 ) ) {
  //	(&vi, 2, SAMPLE_RATE, -1, bps()*1000, -1 ) ) {
  //    error("vorbis_encode_setup_managed failed: invalid bitrate %i",bps());
  //    res = false;
  //  }
  
  return res;
}
  
OutVorbis::~OutVorbis() {
  func("OutVorbis::~OutVorbis() %p",this);
  //  if(running) flush();
  act("closing ogg/vorbis encoder");

  if(rsmp_state) src_delete(rsmp_state);
  free(pcm);
  free(rsmpled);

  if(encoder_initialized) {
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_info_clear(&vi);
  }
}

#endif
