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

#include <math.h>
#include <out_lame.h>
#ifdef HAVE_LAME

#include <jutils.h>
#include <generic.h>
#include <config.h>

/* I think a tighter bound could be:  (mt, March 2000)
 * MPEG1:
 *    num_samples*(bitrate/8)/samplerate + 4*1152*(bitrate/8)/samplerate + 512
 * MPEG2:
 *    num_samples*(bitrate/8)/samplerate + 4*576*(bitrate/8)/samplerate + 256
 */

/* mp3 encoder */

OutLame::OutLame() 
  : OutChannel("lame") {
  func("OutLame::OutLame()");
  
  sprintf(name,"Lame MP3 encoder");
  sprintf(version,"version %s",get_lame_version());

  tipo = MP3;
  enc_flags = NULL;
}

/* note: num is number of samples in the L (or R) channel
   not the total number of samples in pcm[] */
int OutLame::encode() {    
  int num, smp;

  /* num is number of bytes read from pipe
     sound in pipe is always 16bit stereo ... */
  num = erbapipa->read(OUT_CHUNK<<2,pcm);
  //jsleep(0,2000);
  if(num<OUT_CHUNK<<2) return num;
  /* ... therefore samples are num/4 */
  smp = num>>2;

  if(!enc_flags) {
    error("Lame encoder is not initialized");
    return -5;
  }

  encoded = 
    lame_encode_buffer_interleaved
    (enc_flags, pcm, smp, (unsigned char *)buffer, 0); //ENCBUFFER_SIZE);
  
  if(encoded<0)
    switch(encoded) {
    case -1:
      error("lame encoder: mp3 buffer is too small");
      break;
    case -2:
      error("lame encoder: malloc() problem");
      break;
    case -3:
      error("lame encoder: lame_init_params() not called");
      break;
    case -4:
      error("lame encoder: psycho acoustic problems (yes, shamanic indeed)");
      break;
    default:
      error("lame encoder: internal error");
      break;
    }
  
  return encoded;
}

void OutLame::flush() {
  if(!enc_flags) {
    error("Lame encoder is not initialized");
    return;
  }
  lock();
  encoded = lame_encode_flush_nogap
    (enc_flags,(unsigned char*)buffer,ENCBUFFER_SIZE);
  shout();
  unlock();
}

bool OutLame::init() {
  func("initializing %s %s",name,version);

  if(!apply_profile()) {
    error("problems in setting up codec parameters");
    lame_close(enc_flags);
    enc_flags = NULL;
    return false;
  }

  initialized = true;
  return initialized;
}

bool OutLame::apply_profile() {


  if(enc_flags) lame_close(enc_flags);
  enc_flags = lame_init();

  lame_set_errorf(enc_flags,(void (*)(const char*, va_list))error);
  lame_set_debugf(enc_flags,(void (*)(const char*, va_list))func);
  lame_set_msgf(enc_flags,(void (*)(const char*, va_list))act);
  
  lame_set_num_samples(enc_flags,OUT_CHUNK);
  lame_set_num_channels(enc_flags,2); // the mixed input stream is stereo
  lame_set_in_samplerate(enc_flags,SAMPLE_RATE); // the mixed input stream is 44khz
  lame_set_error_protection(enc_flags,1); // 2 bytes per frame for a CRC checksum
  lame_set_compression_ratio(enc_flags,0);
  lame_set_quality(enc_flags,2); // 1..9 1=best 9=worst (suggested: 2, 5, 7)
  //  lame_set_VBR(enc_flags,vbr_abr);


  /* BITRATE */
  lame_set_brate(enc_flags,bps());

  Shouter *ice = (Shouter*)icelist.begin();
  while(ice) {
    char tmp[256];

    snprintf(tmp,256,"%u",bps());       ice->bps( tmp );
    snprintf(tmp,256,"%u",freq());      ice->freq( tmp );
    snprintf(tmp,256,"%u",channels());  ice->channels( tmp );
    
    ice = (Shouter*)ice->next;
  }

  lame_set_out_samplerate(enc_flags,freq());

  /* CHANNELS
     mode 2 (double channel) is unsupported */
  int mode;
  switch( channels() ) {
    /* 0,1,2,3 stereo,jstereo,dual channel,mono */
  case 1: mode = 3; break;
  case 2: mode = 1; break;
  default: mode = 3; break;
  }
  lame_set_mode(enc_flags,(MPEG_mode_e)mode);


  /* in case of VBR 
  func("reversed quality is %i, guessed bps %i",(int)fabs( 10 - quality()),bps());
  lame_set_VBR_q(enc_flags,(int)fabs( 10 - quality() ));
  lame_set_VBR_mean_bitrate_kbps(enc_flags,bps());
  */

  /* lame chooses for us frequency filtering when values are 0 */
  lame_set_lowpassfreq(enc_flags,lowpass());
  lame_set_highpassfreq(enc_flags,highpass());  
  
  int res = lame_init_params(enc_flags);
  if(res<0) {
    error("lame_init_params failed");
    lame_close(enc_flags);
    enc_flags = NULL;
  }

  return (res<0)?false:true;

}

OutLame::~OutLame() {
  func("OutLame::~OutLame() %p",this);
  act("closing lame encoder");
  //  if(running) flush();
  if(enc_flags) lame_close(enc_flags);
}

#endif
