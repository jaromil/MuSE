/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 jaromil <jaromil@dyne.org>
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

#ifndef __GTK2_GUI_H__
#define __GTK2_GUI_H__

#include <gui.h>

class GTK2_GUI : public GUI {
 private:
  bool new_pos[MAX_CHANNELS];
  bool new_lcd[MAX_CHANNELS];
  unsigned int new_sel[MAX_CHANNELS];

  int vuband, vumeter;

 public:
  GTK2_GUI(int argc, char **argv, Stream_mixer *mix);
  ~GTK2_GUI();

  void run();

  void set_pos(unsigned int chan, float pos) { new_pos[chan] = true; };
  void set_lcd(unsigned int chan, char *lcd) { new_lcd[chan] = true; };
  void set_title(char *txt);
  void set_status(char *txt); 
  void add_playlist(unsigned int ch, char *txt);
  void sel_playlist(unsigned int ch, int row);
  void bpsmeter_set(int n) { vuband = n; };
  void vumeter_set(int n) { vumeter = n; };
  bool meter_shown();
};

#endif
