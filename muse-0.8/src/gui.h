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
 * GUI parent class
 *
 */

#ifndef __GUI_H__
#define __GUI_H__

#include <pthread.h>
#include <generic.h>

class Stream_mixer;

class GUI {
 public:
  GUI(int argc, char **argv, Stream_mixer *mix);

  virtual ~GUI();
  
  /* pthread stuff */
  virtual void run() =0;
  void start() { pthread_create(&_thread, NULL, &kickoff, this); };

  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };
  //  void destroy() { _thread_destroy(); };
  /* ------------- */

  virtual void set_lcd(unsigned int chan, char *lcd) =0;
  virtual void set_pos(unsigned int chan, float pos) =0;

  virtual void set_title(char *txt) =0;
  virtual void set_status(char *txt) =0;
  virtual void add_playlist(unsigned int ch, char *txt) =0;
  virtual void sel_playlist(unsigned int ch, int row) =0;

  virtual void bpsmeter_set(int n) =0;
  virtual void vumeter_set(int n) =0;

  virtual bool meter_shown() =0;

  virtual void lameversion(char *str) { };

  bool quit;
  Stream_mixer *_mix;

  float ch_pos[MAX_CHANNELS];
  char ch_lcd[MAX_CHANNELS][10];

 private:
  pthread_t _thread;
  pthread_mutex_t _mutex;
  pthread_cond_t _cond;

 protected:
  static void* kickoff(void *arg) { ((GUI *) arg)->run(); return NULL; };

};

#endif
