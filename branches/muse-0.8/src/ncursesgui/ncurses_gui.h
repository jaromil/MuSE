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

#ifndef __NCURSES_GUI_H__
#define __NCURSES_GUI_H__

#include <jmixer.h>
#include <muse_tui.h>


class NCURSES_GUI : public GUI {

	private:
		char *version;
		char *status;
		char *minibox;
		char *shortcuts;
		int activechan;
		encdata *enc;
		unsigned short int vischan[2];
		bool canupdate;
		CDKScreen f;
		CDKLabel title;
		CDKLabel statusbar;
		CDKLabel opchan;
		CDKLabel helpbar;
		SChannel curs_chan[MAX_CHANNELS];
		ConnectBox *connbox;
		
	public:
		NCURSES_GUI(int argc, char **argv, Stream_mixer *mix);
		~NCURSES_GUI();
		//		void start();
		//		void stop();
		void userinput ();
		void setactivechan (int chanID);
		void setactivechan (int chanID, bool chkupd);
		void changewin (int);
		int getactivechan ();
		void tabchan ();

		void refresh(void);
		void setlabels (void);
		void setdialogs (void);
		void showhelp (void);
		void showabout (void);
		void newurl (void);
		void setconnect (void);
		void newfile (void);


		void run();
		void set_lcd (unsigned int, char *);
		void set_pos (unsigned int, float);
		void set_title(char *txt);
		void set_status(char *txt); 
		void add_playlist(unsigned int ch, char *txt);
		void sel_playlist(unsigned int ch, int row);
		
		/* da istanziare anche se non fanno nulla
		   perche' ora sono funzioni pure virtuali
		   nella classe parente: cio' evita di risolvere
		   la funzione a runtime, velocizzando l'esecuzione */
		void bpsmeter_set(int n) { return; };
		void vumeter_set(int n) { return; };
		bool meter_shown() { return false; };
};

#endif
