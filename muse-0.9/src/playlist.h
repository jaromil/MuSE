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

/**
   @file playlist.h Playlist class header
   @desc the playlist class for input channels */

#ifndef __playlist_h__
#define __playlist_h__

#include <linklist.h>

/**
   @brief Url class (Playlist entry)
   */
class Url: public Entry {
 public:

  /** 
     @param file full path or url scheme to the playlist entry
    */ 
  Url(const char *file);
  ///< the class constructor to the playlist entry

  ~Url();
  ///< url destructor 
  
  char *path;
  ///< the full path (or url scheme) to the playlist entry
  
#ifdef HAVE_SCHEDULER
/*
 * Schedule record chain.  @see radiosched.h for details. 
 * Format might change in the future, as scheduler gets more mature. 
 */
  const char  *mn;       ///< start minute - see crontab(5)
  const char  *hr;       ///< start hour
  const char  *day;      ///< start day
  const char  *mon;      ///< start month
  const char  *wkd;      ///< start day_of_week
  const char  *cmnplay;  ///< duration in minutes
  unsigned    mnplay;
  unsigned    mnleft;    ///< minutes left of playing, if playing
  unsigned    port;      ///< 
  const char  *comment;
  const char  *csecs;    ///< seconds - for future use
  unsigned    secs;
#endif
};

/**
   Playlist class serves the methods to add new files or urls, select
   them and such. It basically adds some few functionalities on top of
   the basic ones implemented by the Linklist parent class.

   @brief Playlist queue 
   */
class Playlist : public Linklist {
 public:
  Playlist();
  ~Playlist();

  char *addurl(const char *file);
  ///< append a new url at the end of the playlist queue
  
  char *addurl(const char *file, int pos);
  ///< add a new url at a certain position of the playlist queue

#ifdef HAVE_SCHEDULER
  void addurl(Url *url);
  Url *addurl(const char *file, const char *mn, const char *hr, 
          const char *day, const char *mon, const char *wkd, 
		  const char *cmnplay, const char *comment );
  ///< add a new @see Url and fill the scheduler-specific fields
#endif

  /**
     @param pos position in playlist (starts from 1)
     @return string pointer to the playlist url at pos */
  char *song(int pos);
  ///< return the url at a certain position of the playlist queue


  char *selection();
  ///< return the currently selected url in the playlist queue
  
  void cleanup();
  ///< cleanup and release all playlist queue
};


#endif
