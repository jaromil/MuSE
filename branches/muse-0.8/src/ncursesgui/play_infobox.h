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

#ifndef __PLAY_INFOBOX_H__
#define __PLAY_INFOBOX_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class PlayInfoBox {
	private:
		CDKSCREEN *cdkscreen;
		CDKLABEL *infobox;
		char song[255];
		char elapsed[32];
		char mode[32];
		char curract[32];
		char *message[2];

	public:
		PlayInfoBox();
		~PlayInfoBox();
		void setscreen (CDKSCREEN *);
		void reg (void);
		void unreg (void);
		void setsong (char *);
		void setsongNR (char *);
		void setelapsed (char *);
		void setelapsedNR (char *);
		void setmode (char *);
		void setmodeNR (char *);
		void setact (const char *newact);
		void setactNR (const char *newact);
		void setparm (int, int, int);
		void setparmNR (int, int, int);
		void draw (void);
		void hide (void);
		void setctrl (void);
		void setctrlNR (void);
		void unsetctrl (void);
		void unsetctrlNR (void);
};

#endif
