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
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <gui.h>
#include <jutils.h>

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

GUI::GUI(int argc, char **argv, Stream_mixer *mix) {
  //  char temp[512];

  //  sprintf(temp,"%s %s",PACKAGE,VERSION);
  //  set_title(temp);

  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condtition"); 
  
  _mix = mix;

  quit = false;
  for(int i=0;i<MAX_CHANNELS;i++) {
    ch_pos[i] = 0.0;
    /* ch_lcd[i] = (char*)malloc(8); */
    memset(ch_lcd[i],0,8);
  }
  
}

GUI::~GUI() {

  /*
  for(int i=0;i<MAX_CHANNELS;i++)
    free(ch_lcd[i]);
  */

  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
}
