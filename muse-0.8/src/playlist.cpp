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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <jutils.h>
#include <playlist.h>
#include <config.h>

Url::Url(const char *file) : Entry() {
  path = strdup(file);
}

Url::~Url() {
  if(path) free(path);
}

Playlist::Playlist()
  : Linklist() {
  filename = NULL;
}

Playlist::~Playlist() {
  cleanup();
}

void Playlist::cleanup() {
  int c;
  Url *p = (Url*)begin();
  while(p!=NULL) {
    c++;
    rem(1);
    delete p;
    p = (Url*) begin();
  }
  clear();
  if(filename) free(filename);
}

char *Playlist::addurl(const char *file) {
  Url *url = new Url(file);
  append((Entry*)url);
  return(url->path);
}

char *Playlist::addurl(const char *file, int pos) {
  Url *url = new Url(file);
  insert((Entry*)url,pos);
  return(url->path);
}

char *Playlist::song(int pos) {
  Url *sel = (Url*) pick(pos);
  
  if(sel) return(sel->path);
  
  warning("Playlist::song(%i) : invalid song requested",pos);
  return NULL;
}

/*
bool Playlist::sel(int pos) {
  Url *sel = (Url*)pick(pos);

  if(sel) {
    selected = sel;
    return(true);
  }
  
  warning("Playlist::sel(%i) : invalid selection",pos);
  selected = NULL;
  return(false);
}

void Playlist::sel(Url *sel) {
  if(!sel) {
    warning("Playlist::sel(NULL *Entry) : misbehaviour",sel);
    return;
  }
  selected = sel;
}

int Playlist::sel() {
  if(!selected) {
    warning("Playlist::sel() : selected = %p",selected);
    return 1;
  }
  
  return pos((Entry*)selected);
}
*/
