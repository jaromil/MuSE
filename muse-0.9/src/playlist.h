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
 */

#ifndef __playlist_h__
#define __playlist_h__

#include <linklist.h>

class Url: public Entry {
 public:
  Url(const char *file);
  ~Url();
  
  char *path;
};


class Playlist : public Linklist {
 public:
  Playlist();
  ~Playlist();

  /* makes a local copy of the path */
  char *addurl(const char *file);
  char *addurl(const char *file, int pos);

  /* returns the full path of the element
     position starts from 1 */
  char *song(int pos) { return ( ((Url*) pick(pos))->path ); };
  //  char *song() { return song( sel() ); };

  /* sets and returns selection
     now directly in linklist
     bool sel(int pos);
     void sel(Url *sel);
     int sel();
  */
  void cleanup();

  //  Url *selected;
  char *filename;
};


#endif
