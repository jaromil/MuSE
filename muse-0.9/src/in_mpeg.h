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

#ifndef __IN_MPEG_H__
#define __IN_MPEG_H__

#include <inchannels.h>

/* mpeg lib (splay) */
#include <libmpeg/mpegsound.h>

/* Mpeg Channel // MPEG-1 layer 1,2,3 and MPEG-2 layer 3 */

class MpegChannel: public Channel {
 private:
  Soundinputstream *loader;
  Mpegtoraw *server;
  IN_DATATYPE *_get_audio();
  int framesize;
  int chunk_smp;
  
 public:
  MpegChannel();
  ~MpegChannel();
  
  int load(char *file);
  bool pos(float pos);
  void clean();

};

#endif
