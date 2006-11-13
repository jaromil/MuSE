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

/**
   @file dec_mp3.h Mp3 decoder
   @desc Mpeg-1 layer 1,2,3 and Mpeg-2 layer 3 input channel:
         MuseDec implementation 
	 */

#ifndef __DEC_MP3_H__
#define __DEC_MP3_H__

#include <decoder.h>
#include <config.h>

/* mpeg lib (splay) */
#include <libmpeg/mpegsound.h>

/**
   Instances of the Mp3 codec (splay implementation) are created
   internally by the Channel class, publicly available interface to
   the creation is in Stream_mixer::add_to_playlist.

   @class MuseDecMp3
   @brief Mpeg-1 layer 1,2,3 and Mpeg-2 layer 3
   */
class MuseDecMp3: public MuseDec {
 private:
  Soundinputstream *loader;
  Mpegtoraw *server;

  int framesize;
  int chunk_smp;

  char _file[MAX_PATH_SIZE];

 public:
  MuseDecMp3();
  ~MuseDecMp3();
  
  int load(char *file);
  bool seek(float pos);
  void clean();

  IN_DATATYPE *get_audio();

};

#endif
