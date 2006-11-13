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

#ifndef __MUSE_TUI_H__
#define __MUSE_TUI_H__

#include "muse_console.h"
#include "panel.h"

class muse_tui
{
	private:
		char *version[1];
		char *status[1];
		char *minibox[1];
		char *shortcuts[1];
		int activechan;
		CDKScreen f;
		CDKLabel title;
		CDKLabel statusbar;
		CDKLabel opchan;
		CDKLabel helpbar;
		SChannel chan[MAX_CHANNELS];
	public:
		muse_tui();
		~muse_tui();
		void start();
		void stop();
		int userinput ();
		void setactivechan (int chanID);
		void changewin (int);
		int getactivechan ();
		void tabchan ();

		void refresh(void);
		void setlabels (void);
		void setdialogs (void);
		void showhelp (void);
		void showabout (void);
		void newurl (void);
		void newfile (void);


};

#endif
