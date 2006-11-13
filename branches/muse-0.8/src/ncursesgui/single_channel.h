/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2002 Luca 'rubik' Profico <rubik@olografix.org>
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
 * $Id$
 * 
 */

#ifndef __M_SINGLE_CHAN_H__
#define __M_SINGLE_CHAN_H__

extern "C" {
#include <libcdk/cdk.h>
}

#include "screen.h"
#include "slider.h"
#include "scroll.h"
#include "label.h"
#include "play_infobox.h"

class SChannel
{ 
	private:
		int parentWidth, parentHeight;
		int chanID;
		CDKSlider time_slider; 
		CDKSlider volume_slider; 
		CDKScroll playlist_scroll;
		PlayInfoBox infobox;
		int volume_lowvalue , volume_highvalue;
		int time_lowvalue , time_highvalue;
		int volume_currentvalue;
		int playmode;
		bool box;
	
	public:
		SChannel (); 
		~SChannel ();
		void init(CDKSCREEN *screen, int xrel, int chanID);
		void defparam (void);
		void show(void);
		void hide(void);
		void reg(void);
		void unreg(void);
		void activate_volume(void);
		void setctrl (void);
		void unsetctrl (void);
		void selnextitem (void);
		void selprevitem (void);
		void add_to_playlist (char *);
		void rem_from_playlist (int);
		float incrvol (void);
		float decrvol (void);
		void zerovol (void);
		float getvol (void);
		int cplitem (void);
		void set_time (char *);
		void set_pos (float);
		int cycmode (void);
		int setmode (int);
};


#endif
