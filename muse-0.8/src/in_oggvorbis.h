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

#ifndef __IN_OGGVORBIS_H__
#define __IN_OGGVORBIS_H__

#include <config.h>
#ifdef HAVE_VORBIS

#include <inchannels.h>

/* oggvorbis lib */
extern "C" {
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
}

/* OggVorbis Channel // ogg-vorbis decoding */

class OggChannel: public Channel {
 private:
  OggVorbis_File   vf;
  vorbis_info      *vi;
  vorbis_comment   *vc;

  int current_section, old_section;

  FILE *oggfile;
  IN_DATATYPE *_get_audio();
  char _inbuf[MIX_CHUNK+2];

 public:
  OggChannel();
  ~OggChannel();

  int load(char *file);
  bool pos(float pos);
  void clean();
};

#endif /* HAVE VORBIS */
#endif
