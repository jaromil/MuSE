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

#include <stdlib.h>
#include <gui.h>

#include <gtkgui2/gen.h>
#include <gtkgui2/musegtk.h>
#include <gtk2_gui.h>

#include <jmixer.h>
#include <jutils.h>
#include <config.h>

GTK2_GUI::GTK2_GUI(int argc, char **argv, Stream_mixer *mix)
  : GUI(argc,argv,mix) {
  gtkgui_init(argc,argv,mix);
  for(int i=0;i<MAX_CHANNELS;i++) {
    new_pos[i] = false;
    new_lcd[i] = false;
    new_sel[i] = 0;
  }
}

GTK2_GUI::~GTK2_GUI() { 

}

void GTK2_GUI::run() {
  while(!quit) {
    if(quit = !gtkgui_get_state()) {
      _mix->quit = true;
    } else {
      int i;
      lock();
      wait();

      /* update channels */
      for(i=0;i<MAX_CHANNELS;i++) {
	if(new_pos[i]) {
	  gtkgui_set_pos(i,ch_pos[i]);
	  new_pos[i] = false;
	}
	if(new_lcd[i]) {
	  gtkgui_set_lcd(i,ch_lcd[i]);
	  new_lcd[i] = false;
	}
	if(new_sel[i]!=0) {
	  gtkgui_sel_playlist(i,new_sel[i]);
	  new_sel[i] = 0;
	}
      }

      /* update vumeters */
      if(vu_status) {
	gtkgui_set_vuband(vuband);
	gtkgui_set_vuvolume(vumeter);
      }

      gtkgui_refresh();
    
      unlock();
    }

    /* dalle parti mie si dice:
       piglia o'tiemp pe sputa' n'terr */
    //    jsleep(0,20);
    
  }
  gtkgui_refresh();
  gtkgui_exit();
}

void GTK2_GUI::set_title(char *txt) {
  gtkgui_set_maintitle(txt);
}
 
void GTK2_GUI::set_status(char *txt) {
  gtkgui_set_statustext(txt);
}

void GTK2_GUI::add_playlist(unsigned int ch, char *txt) {
  gtkgui_add_into_pl(ch,txt);
}

void GTK2_GUI::sel_playlist(unsigned int ch, int row) {
  new_sel[ch] = row;
}

bool GTK2_GUI::meter_shown() { return vu_status; }
