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

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <jutils.h>
#include <playlist.h>

Url::Url(const char *file) : Entry() {
  path = strdup(file);
#ifdef HAVE_SCHEDULER
  mn=NULL;   hr=NULL;
  day=NULL;  mon=NULL;
  wkd=NULL;  cmnplay=NULL;
  comment=NULL;
  csecs=NULL;
#endif
}

Url::~Url() {
  if(path) free(path);
#ifdef HAVE_SCHEDULER
  if(mn) free((char*)mn);
  if(hr) free((char*)hr);
  if(day) free((char*)day);
  if(mon) free((char*)mon);
  if(wkd) free((char*)wkd);
  if(cmnplay) free((char*)cmnplay);
  if(comment) free((char*)comment);
  if(csecs) free((char*)csecs);
#endif
}

Playlist::Playlist()
  : Linklist() {
}

Playlist::~Playlist() {
  cleanup();
}

void Playlist::cleanup() {
  Url *p = (Url*)begin();
  while(p!=NULL) {
    rem(1);
    delete p;
    p = (Url*) begin();
  }
  clear();
}

char *Playlist::addurl(const char *file) {
  Url *url = new Url(file);
  if(!url)
    error("%i:%s %s url is NULL",__LINE__,__FILE__,__FUNCTION__);
  append((Entry*)url);
  return(url->path);
}

char *Playlist::addurl(const char *file, int pos) {
  Url *url = new Url(file);
  insert((Entry*)url,pos);
  return(url->path);
}

#ifdef HAVE_SCHEDULER
#define SAFE(x) (x?x:"")
void Playlist::addurl(Url *url) {
  append((Entry*)url);
}

Url *Playlist::addurl(const char *file, const char *mn, const char *hr, 
          const char *day, const char *mon, const char *wkd, 
		  const char *cmnplay, const char *comment )
{
  Url *url = new Url(file);
  if(!url)
    error("%i:%s %s url is NULL",__LINE__,__FILE__,__FUNCTION__);
  append((Entry*)url);
  url->mn = strdup(SAFE(mn)); 
  url->hr = strdup(SAFE(hr)); 
  url->day = strdup(SAFE(day)); 
  url->mon = strdup(SAFE(mon)); 
  url->wkd = strdup(SAFE(wkd)); 
  url->cmnplay = strdup(SAFE(cmnplay)); 
  if (cmnplay) {
    url->mnplay = atoi(cmnplay);
  } else {
	url->mnplay = 0;
  }
  url->mnleft = 0;
  url->comment = strdup(SAFE(comment)); 
  return(url);
}
#endif

char *Playlist::song(int pos) {
  Url *sel = (Url*) pick(pos);
  
  if(sel) return(sel->path);
  
  warning("Playlist::song(%i) : invalid song requested",pos);
  return NULL;
}

char *Playlist::selection() {
  Url *sel = (Url*) selected();
  if(sel) return(sel->path);
  warning("Playlist::selected() : no selection");
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
