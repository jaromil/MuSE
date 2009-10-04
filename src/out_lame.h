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

#ifndef __OUT_LAME_H__
#define __OUT_LAME_H__

#include <outchannels.h>
#include <lame_wrap.h>
#include <config.h>

class OutLame : public OutChannel {
  
 private:
  lame_t enc_flags;
  int16_t pcm[OUT_CHUNK<<5];


 public:
  OutLame();
  ~OutLame();
  int encode();
  bool init();
  void flush();
  bool apply_profile();
  
};


#endif
