/*
 * Copyright (C) 2009 - Luca Bigliardi
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <dlfcn.h>

#include <lame_wrap.h>

#include <jutils.h>

void *LameWrap::_libhandle = NULL;

LameWrap::constcharpgetter_t *LameWrap::_get_version = NULL;
LameWrap::lameflagsgetter_t *LameWrap::_init = NULL;
LameWrap::lameflagssetter_t *LameWrap::_close = NULL;
LameWrap::funcsetter_t *LameWrap::_set_errorf = NULL;
LameWrap::funcsetter_t *LameWrap::_set_debugf = NULL;
LameWrap::funcsetter_t *LameWrap::_set_msgf = NULL;
LameWrap::unsignedlongsetter_t *LameWrap::_set_num_samples = NULL;
LameWrap::intsetter_t *LameWrap::_set_num_channels = NULL;
LameWrap::intsetter_t *LameWrap::_set_in_samplerate = NULL;
LameWrap::intsetter_t *LameWrap::_set_out_samplerate = NULL;
LameWrap::intsetter_t *LameWrap::_set_error_protection = NULL;
LameWrap::floatsetter_t *LameWrap::_set_compression_ratio = NULL;
LameWrap::intsetter_t *LameWrap::_set_quality = NULL;
LameWrap::intsetter_t *LameWrap::_set_brate = NULL;
LameWrap::modesetter_t *LameWrap::_set_mode = NULL;
LameWrap::intsetter_t *LameWrap::_set_lowpassfreq = NULL;
LameWrap::intsetter_t *LameWrap::_set_highpassfreq = NULL;
LameWrap::lameflagssetter_t *LameWrap::_init_params = NULL;
LameWrap::encodebufferinterleaved_t *LameWrap::_encode_buffer_interleaved = NULL;
LameWrap::encodeflushnogap_t *LameWrap::_encode_flush_nogap = NULL;

bool LameWrap::_setsymbolpointer(const char *symbol, void** pointer) {
  if (!LameWrap::_libhandle) {
    error("%s: asking symbol but handle not loaded", __PRETTY_FUNCTION__);
    return false;
  }
  *pointer = dlsym(LameWrap::_libhandle, symbol);
#ifdef HAVE_DARWIN
  /* darwin prepends an _ before plugin symbol names */
  if(!*pointer) {
    char _symbol[256];
    snprintf(_symbol,256,"_%s",symbol);
    *pointer = dlsym(LameWrap::_handle, _symbol);
  }
#endif
  if (!*pointer) {
    error("%s: cannot load symbol %s", __PRETTY_FUNCTION__, symbol);
    return false;
  }
  return true;
}

bool LameWrap::load(const char* path){
  if (LameWrap::_libhandle) {
    act("%s: library already loaded", __PRETTY_FUNCTION__);
  }

  LameWrap::_libhandle = dlopen(path,RTLD_LAZY);
  if(!LameWrap::_libhandle) {
    warning("%s cannot load library: %s", __PRETTY_FUNCTION__, dlerror());
    return false;
  }

  if (!LameWrap::_setsymbolpointer("get_lame_version",
                                   (void**)&LameWrap::_get_version))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_init",
                                   (void**)&LameWrap::_init))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_close",
                                   (void**)&LameWrap::_close))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_errorf",
                                   (void**)&LameWrap::_set_errorf))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_debugf",
                                   (void**)&LameWrap::_set_debugf))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_msgf",
                                   (void**)&LameWrap::_set_msgf))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_num_samples",
                                   (void**)&LameWrap::_set_num_samples))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_num_channels",
                                   (void**)&LameWrap::_set_num_channels))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_in_samplerate",
                                   (void**)&LameWrap::_set_in_samplerate))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_out_samplerate",
                                   (void**)&LameWrap::_set_out_samplerate))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_error_protection",
                                   (void**)&LameWrap::_set_error_protection))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_compression_ratio",
                                   (void**)&LameWrap::_set_compression_ratio))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_quality",
                                   (void**)&LameWrap::_set_quality))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_brate",
                                   (void**)&LameWrap::_set_brate))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_mode",
                                   (void**)&LameWrap::_set_mode))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_lowpassfreq",
                                   (void**)&LameWrap::_set_lowpassfreq))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_set_highpassfreq",
                                   (void**)&LameWrap::_set_highpassfreq))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_init_params",
                                   (void**)&LameWrap::_init_params))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_encode_buffer_interleaved",
                                   (void**)&LameWrap::_encode_buffer_interleaved
                                  ))
    goto exit_error;
  if (!LameWrap::_setsymbolpointer("lame_encode_flush_nogap",
                                   (void**)&LameWrap::_encode_flush_nogap))
    goto exit_error;

  return true;

exit_error:
  LameWrap::unload();
  return false;
}

void LameWrap::unload() {
  if (LameWrap::_libhandle != NULL) {
    dlclose(LameWrap::_libhandle);
  }
  LameWrap::_libhandle = NULL;
  LameWrap::_get_version = NULL;
  LameWrap::_init = NULL;
  LameWrap::_close = NULL;
  LameWrap::_set_errorf = NULL;
  LameWrap::_set_debugf = NULL;
  LameWrap::_set_msgf = NULL;
  LameWrap::_set_num_samples = NULL;
  LameWrap::_set_num_channels = NULL;
  LameWrap::_set_in_samplerate = NULL;
  LameWrap::_set_out_samplerate = NULL;
  LameWrap::_set_error_protection = NULL;
  LameWrap::_set_compression_ratio = NULL;
  LameWrap::_set_quality = NULL;
  LameWrap::_set_brate = NULL;
  LameWrap::_set_mode = NULL;
  LameWrap::_set_lowpassfreq = NULL;
  LameWrap::_set_highpassfreq = NULL;
  LameWrap::_init_params = NULL;
  LameWrap::_encode_buffer_interleaved = NULL;
  LameWrap::_encode_flush_nogap = NULL;
}

const char *LameWrap::get_version(void){
  if (!LameWrap::_get_version) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return NULL;
  }
  return (*LameWrap::_get_version)();
}

lame_t LameWrap::init(void){
  if (!LameWrap::_init) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return NULL;
  }
  return (*LameWrap::_init)();
}

int LameWrap::close(lame_t gfp){
  if (!LameWrap::_close) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_close)(gfp);
}

int LameWrap::set_errorf(lame_t gfp, void (*func)(const char *, va_list)){
  if (!LameWrap::_set_errorf) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_errorf)(gfp, func);
}

int LameWrap::set_debugf(lame_t gfp, void (*func)(const char *, va_list)){
  if (!LameWrap::_set_debugf) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_debugf)(gfp, func);
}

int LameWrap::set_msgf(lame_t gfp, void (*func)(const char *, va_list)){
  if (!LameWrap::_set_msgf) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_msgf)(gfp, func);
}

int LameWrap::set_num_samples(lame_t gfp, unsigned long samples){
  if (!LameWrap::_set_num_samples) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_num_samples)(gfp, samples);
}

int LameWrap::set_num_channels(lame_t gfp, int channels){
  if (!LameWrap::_set_num_channels) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_num_channels)(gfp, channels);
}

int LameWrap::set_in_samplerate(lame_t gfp, int samplerate){
  if (!LameWrap::_set_in_samplerate) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_in_samplerate)(gfp, samplerate);
}

int LameWrap::set_out_samplerate(lame_t gfp, int samplerate){
  if (!LameWrap::_set_out_samplerate) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_out_samplerate)(gfp, samplerate);
}

int LameWrap::set_error_protection(lame_t gfp, int protection){
  if (!LameWrap::_set_error_protection) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_error_protection)(gfp, protection);
}

int LameWrap::set_compression_ratio(lame_t gfp, float ratio){
  if (!LameWrap::_set_compression_ratio) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_compression_ratio)(gfp, ratio);
}

int LameWrap::set_quality(lame_t gfp, int quality){
  if (!LameWrap::_set_quality) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_quality)(gfp, quality);
}

int LameWrap::set_brate(lame_t gfp, int brate){
  if (!LameWrap::_set_brate) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_brate)(gfp, brate);
}

int LameWrap::set_mode(lame_t gfp, MpegMode mode){
  if (!LameWrap::_set_mode) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_mode)(gfp, mode);
}

int LameWrap::set_lowpassfreq(lame_t gfp, int freq){
  if (!LameWrap::_set_lowpassfreq) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_lowpassfreq)(gfp, freq);
}

int LameWrap::set_highpassfreq(lame_t gfp, int freq){
  if (!LameWrap::_set_highpassfreq) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_set_highpassfreq)(gfp, freq);
}

int LameWrap::init_params(lame_t gfp){
  if (!LameWrap::_init_params) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_init_params)(gfp);
}

int LameWrap::encode_buffer_interleaved(lame_t gfp, short int pcm[],
                              int num_samples, unsigned char* mp3buf,
                              int mp3buf_size){
  if (!LameWrap::_encode_buffer_interleaved) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_encode_buffer_interleaved)(gfp, pcm, num_samples, mp3buf,
                                                 mp3buf_size);
}

int LameWrap::encode_flush_nogap(lame_t gfp, unsigned char* mp3buf, int size){
  if (!LameWrap::_encode_flush_nogap) {
    error("%s function symbol not loaded", __PRETTY_FUNCTION__);
    return -1;
  }
  return (*LameWrap::_encode_flush_nogap)(gfp, mp3buf, size);
}


