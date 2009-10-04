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

#ifndef __LAME_WRAP_H__
#define __LAME_WRAP_H__

#include <stdarg.h>

#define LIBLAME_SO_PATH "libmp3lame.so"

// definitions from lame header
typedef struct lame_global_struct lame_global_flags;
typedef lame_global_flags *lame_t;

enum MpegMode {
  STEREO = 0,
  JOINT_STEREO,
  DUAL_CHANNEL,
  MONO,
  NOT_SET,
  MAX_INDICATOR
};

class LameWrap {
  // functions types
  typedef const char* (constcharpgetter_t)(void);
  typedef lame_t (lameflagsgetter_t)(void);
  typedef int (lameflagssetter_t)(lame_t);
  typedef int (funcsetter_t)(lame_t, void (*func)(const char *, va_list));
  typedef int (unsignedlongsetter_t)(lame_t, unsigned long);
  typedef int (intsetter_t)(lame_t, int);
  typedef int (floatsetter_t)(lame_t, float);
  typedef int (modesetter_t)(lame_t, MpegMode);
  typedef int (encodebufferinterleaved_t)(lame_t, short int*, int,
                                            unsigned char*, int);
  typedef int (encodeflushnogap_t)(lame_t, unsigned char*, int);

  public:
    static bool load(const char* path=LIBLAME_SO_PATH);
    static void unload();

    static const char* get_version(void);

    static lame_t init(void);
    static int close(lame_t gfp);

    static int set_errorf(lame_t gfp, void (*func)(const char *, va_list));
    static int set_debugf(lame_t gfp, void (*func)(const char *, va_list));
    static int set_msgf(lame_t gfp, void (*func)(const char *, va_list));

    static int set_num_samples(lame_t gfp, unsigned long samples);
    static int set_num_channels(lame_t gfp, int channels);

    static int set_in_samplerate(lame_t gfp, int samplerate);
    static int set_out_samplerate(lame_t gfp, int samplerate);

    static int set_error_protection(lame_t gfp, int protection);
    static int set_compression_ratio(lame_t gfp, float ratio);
    static int set_quality(lame_t gfp, int quality);
    static int set_brate(lame_t gfp, int brate);

    static int set_mode(lame_t gfp, MpegMode mode);

    static int set_lowpassfreq(lame_t gfp, int freq);
    static int set_highpassfreq(lame_t gfp, int freq);

    static int init_params(lame_t gfp);

    static int encode_buffer_interleaved(lame_t gfp, short int pcm[],
                                         int num_samples, unsigned char* mp3buf,
                                         int mp3buf_size);
    static int encode_flush_nogap(lame_t gfp, unsigned char* mp3buf, int size);

  private:
    static void *_libhandle;
    static bool _setsymbolpointer(const char *symbol, void** pointer);

    static constcharpgetter_t *_get_version;
    static lameflagsgetter_t *_init;
    static lameflagssetter_t *_close;
    static funcsetter_t *_set_errorf;
    static funcsetter_t *_set_debugf;
    static funcsetter_t *_set_msgf;
    static unsignedlongsetter_t *_set_num_samples;
    static intsetter_t *_set_num_channels;
    static intsetter_t *_set_in_samplerate;
    static intsetter_t *_set_out_samplerate;
    static intsetter_t *_set_error_protection;
    static floatsetter_t *_set_compression_ratio;
    static intsetter_t *_set_quality;
    static intsetter_t *_set_brate;
    static modesetter_t *_set_mode;
    static intsetter_t *_set_lowpassfreq;
    static intsetter_t *_set_highpassfreq;
    static lameflagssetter_t *_init_params;
    static encodebufferinterleaved_t *_encode_buffer_interleaved;
    static encodeflushnogap_t *_encode_flush_nogap;
};

#endif // __LAME_WRAP_H__
