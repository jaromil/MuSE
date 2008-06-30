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

#include <stdlib.h>
#include <config.h>
#include <muse_tui.h>
#include <jmixer.h>
#include <jutils.h>
#include <ncurses_gui.h>

NCURSES_GUI::NCURSES_GUI(int argc, char **argv, Stream_mixer *mix)
: GUI(argc, argv, mix) 
{
	version = strdup("MuSE");
	status = strdup(" Status::");
	minibox = strdup("xxxxxxxxxxxxxxx");
	shortcuts = strdup(SHORTCUTS_HELP);
	activechan=0;

	setlabels();

	//enc = (struct encdata *) malloc (sizeof(struct encdata));
	connbox = new ConnectBox (_mix);
}

NCURSES_GUI::~NCURSES_GUI() {

	func ("NCURSES_GUI::~NCURSES_GUI");
	func ("screen pointer = %p", f.screen());
	func ("title label pointer: %p", title.label());
	func ("statusbar label pointer: %p", statusbar.label());
	func ("helpbar label pointer: %p", helpbar.label());
	func ("opchan label pointer: %p", opchan.label());
	warning ("screen objcnt: %d", f.objcnt());

	/* avoids free mem space overwriting by registering
	 * all channels on the screen before quit */
	for (int i = 0; i<6; i++) { curs_chan[i].reg(); }
				
	free (minibox);
	free (version);
	free (status);
	free (shortcuts);
	connbox->erase();
}

void NCURSES_GUI::refresh ()
{
	//int i;
	//setlabels ();
	/*
	   for (i = 0; i < MAX_CHANNELS; i++)
	   {
	   if (i % 2 == 0)		//tutti a sinistraaaa ehehhhhh
	   chan[i].init (f.screen (), 1, i);
	   else			//e tutti a destra ohhohhhhh
	   chan[i].init (f.screen (), f.width () / 2, i);
	   }
	   */

	canupdate = false;
	setactivechan (activechan, 0);
	canupdate = true;
	warning ("screen pointer = %p", f.screen());
	/*
	   chan[0].show ();
	   chan[1].show ();
	   */
	// setactivechan (0);
	f.refresh ();
}

void NCURSES_GUI::run()
{
	int i;

	while(!quit) {
		//setdialogs();
			connbox->setparm();

		for (i=0;i< MAX_CHANNELS;i++)
		{
			if (i%2 == 0) { //tutti a sinistraaaa ehehhhhh
				curs_chan[i].init (f.screen(),1,i);
			} else { //e tutti a destra ohhohhhhh
				curs_chan[i].init (f.screen(), f.width()/2, i);
			}
			_mix->create_channel (i);
			curs_chan[i].unreg();
			curs_chan[i].defparam();
		}

		f.refresh();
		canupdate = true;
		curs_chan[0].reg();
//		curs_chan[0].show();
		curs_chan[1].reg();
//		curs_chan[1].show();
		setactivechan (0);
		userinput();
		wait();
	}
}

void NCURSES_GUI::setlabels (void)
{

	title.setscreen (f.screen());
	title.box = TRUE;
	title.setparm(1,1,f.width() - 4,1);
	title.setvalue (version);
	//title.draw();

	statusbar.setscreen (f.screen());
	statusbar.box = TRUE;
	statusbar.setparm(16,f.height()-3,f.width()-19,1);
	statusbar.setvalue (status);
	statusbar.urhook (ACS_RTEE);
	statusbar.ulhook (ACS_TTEE);
	statusbar.llhook (ACS_BTEE);
	//statusbar.draw();

	helpbar.setscreen (f.screen());
	helpbar.box = TRUE;
	helpbar.setparm (1,f.height() -5, f.width() - 4, 1);
	helpbar.setvalue (shortcuts);
	helpbar.llhook (ACS_LTEE);
	helpbar.lrhook (ACS_RTEE);
	//helpbar.draw();

	opchan.setscreen (f.screen());
	opchan.box=TRUE;
	opchan.setparm (1, f.height() -3, 15, 1);
	opchan.ulhook (ACS_LTEE);
	opchan.urhook (ACS_TTEE);
	opchan.lrhook (ACS_BTEE);
	//opchan.draw();
}

void NCURSES_GUI::setdialogs (void)
{
}

void NCURSES_GUI::changewin(int newwin)
{
	int gc;

	gc = newwin * 2 + (activechan %2);
	if (gc != activechan) setactivechan (gc);

}

/*
void NCURSES_GUI::stop()
{
	warning ("muse_tui.stop()");
}
*/

void NCURSES_GUI::userinput ()
{
	float f;
	int ch;
	int quit=0;
	//char temp[50];
	while (!quit)
	{
		wait();
		ch = getch();
		// warning ("getch() got %d", ch);
		if ((ch > 48) & (ch <= (48 + MAX_CHANNELS))) {
			setactivechan (ch - 49);
		} else switch (ch) {
			case KEY_TAB:
				tabchan();
				break; 
			case KEY_F1: // F1
			case 80: // does keypad work?
				warning ("request change to win 1");
				changewin (0);
				break;
			case KEY_F2: // F2
			case 81: // does keypad work?
				warning ("request change to win 2");
				changewin (1);
				break;
			case KEY_F3: // F3
			case 82: // does keypad work?
				warning ("request change to win 3");
				changewin (2);
				break;
			case KEY_UP:
				curs_chan[activechan].selprevitem();
				break;
			case KEY_DOWN:
				curs_chan[activechan].selnextitem();
				break;
			case '+':
				f = curs_chan[activechan].incrvol();
				_mix->set_volume (activechan, f);
				break;
			case '-':
				f = curs_chan[activechan].decrvol();
				_mix->set_volume (activechan, f);
				break;
			case '>': // fade right
				f = curs_chan[vischan[0]].decrvol();
				_mix->set_volume (vischan[0], f);
				f = curs_chan[vischan[1]].incrvol();
				_mix->set_volume (vischan[1], f);
				break;
			case '<': //fade left
				f = curs_chan[vischan[0]].incrvol();
				_mix->set_volume (vischan[0], f);
				f = curs_chan[vischan[1]].decrvol();
				_mix->set_volume (vischan[1], f);
				break;
			case 48: // 0
				curs_chan[activechan].zerovol();
				_mix->set_volume (activechan, 0);
				break;
			case 'u':
			case 'U':
				newurl();
				refresh();
				break;
			case 'f':
			case 'F':
				newfile();
				refresh();
				break;
			case 't':
			case 'T':
				/* talk */
				break;
			case 'm':
			case 'M':
				_mix->set_playmode(activechan, curs_chan[activechan].cycmode());
				break;
			case 'a':
			case 'A':
				//showabout();
				refresh();
				break;
			case 'k':
			case 'K':
				if (_mix->dspout==true) _mix->dspout=false; else _mix->dspout=true;
				/* speaker */
				break;
			case 'l':
			case 'L':
				if (_mix->linein==true) _mix->linein=false; else _mix->linein=true;
				/* line-in */
				break;
			case 'c':
			case 'C':
				setconnect();
				/* FIXME adattare alla nuova api
				snprintf (temp, 49, "Muse - streaming http://%s:%u/%s",_mix->ice.ip,_mix->ice.port,_mix->ice.mount);
				if (_mix->ice.running) title.setvalueNR (temp);
				*/
				refresh();
				/* connect */
				break;
			case 'h':
			case 'H':
				/* connect */
				showhelp();
				refresh();
				break;
			case 'r':
				/* for debugging purpose */
				refresh();
				break;
			case 'q':
				quit = 1;
				break;
			case 10:
				warning ("cplitem: %d", curs_chan[activechan].cplitem() + 1);
				_mix->set_channel (activechan, curs_chan[activechan].cplitem() + 1);
				_mix->play_channel (activechan);
				break;
			case 's':
			case 'S':
				_mix->stop_channel (activechan);
				break;
			default:
				continue;
		}

		warning ("quit = %d", quit);
	}

	/* si va a nanna */
	/*FIXME nuova api
	if (_mix->ice.running) _mix->ice.stop();
	*/
	_mix->quit = true;
	jsleep (0,200);
}

void NCURSES_GUI::setactivechan (int chanID, bool chkupd)
{
	func ("NCURSES_GUI::setactivechan()");
	warning ("activechan = %d", activechan);
	if (chkupd) canupdate = false;
	curs_chan[activechan].unsetctrl();
	curs_chan[activechan].unreg();

	if (activechan % 2 == 0) { 
		curs_chan[activechan + 1].unreg();
	} else { 
		curs_chan[activechan - 1].unreg();
	}

	activechan = chanID;
	curs_chan[activechan].setctrl();
	curs_chan[activechan].reg();
	curs_chan[activechan].show();
	vischan[0] = activechan;


	if (activechan % 2 == 0) { 
		curs_chan[activechan + 1].reg();
		curs_chan[activechan + 1].show();
		vischan[1] = activechan +1;
	} else { 
		curs_chan[activechan - 1].reg();
		curs_chan[activechan - 1].show();
		vischan[1] = activechan -1;
	}

	if (vischan[0] > vischan[1]) {
		int temp;
		temp = vischan[0];
		vischan[0] = vischan[1];
		vischan[1] = temp;
	}

	snprintf (minibox, 15, "win: %d chan: %d", 
			(activechan /2) +1, activechan + 1);

	opchan.setvalue(minibox);
	if (chkupd) canupdate = true;
	func ("end NCURSES_GUI::setactivechan()");

}


void NCURSES_GUI::setactivechan (int chanID)
{
	func ("NCURSES_GUI::setactivechan()");
	warning ("activechan = %d", activechan);
	canupdate = false;
	curs_chan[activechan].unsetctrl();
	curs_chan[activechan].unreg();

	if (activechan % 2 == 0) { 
		curs_chan[activechan + 1].unreg();
	} else { 
		curs_chan[activechan - 1].unreg();
	}

	activechan = chanID;
	curs_chan[activechan].setctrl();
	curs_chan[activechan].reg();
	curs_chan[activechan].show();
	vischan[0] = activechan;

	if (activechan % 2 == 0) { 
		curs_chan[activechan + 1].reg();
		curs_chan[activechan + 1].show();
		vischan[1] = activechan +1;
	} else { 
		curs_chan[activechan - 1].reg();
		curs_chan[activechan - 1].show();
		vischan[1] = activechan -1;
	}

	if (vischan[0] > vischan[1]) {
		int temp;
		temp = vischan[0];
		vischan[0] = vischan[1];
		vischan[1] = temp;
	}

	snprintf (minibox, 15, "win: %d chan: %d", 
			(activechan /2) +1, activechan + 1);

	opchan.setvalue(minibox);
	canupdate = true;
}

int NCURSES_GUI::getactivechan (void)
{
	return activechan;
}

void NCURSES_GUI::tabchan (void)
{
	canupdate = false;
	if (activechan % 2 == 0) { 
		setactivechan (activechan + 1);
	} else { 
		setactivechan (activechan - 1);
	}
	canupdate = true;
}

void NCURSES_GUI::setconnect (void)
{
	func ("NCURSES_GUI::setconnect()");
	canupdate = false;
	//connbox.draw();
	//connbox.refresh();

	connbox->activate();
	canupdate = true;
	func ("end NCURSES_GUI::setconnect()");
}

void NCURSES_GUI::newurl (void)
{
	char * url;
	CDKEntry urlentry;

	warning ("+url entry");
	urlentry.setscreen (f.screen());
	urlentry.setparm (NEWURLENTRY_TITLE, 
			NEWURLENTRY_LABEL, 
			40, 
			CENTER, 
			CENTER);

	url = urlentry.activateonce();
	if (url)
	{
		warning ("new url = %s", url);
		_mix->add_to_playlist (activechan, url);
	}
}	

void NCURSES_GUI::newfile (void)
{
	char *file;
	canupdate = false;
	CDKFselect fileselect;

	warning ("+file select");
	fileselect.setscreen (f.screen());
	fileselect.setbox(true);
	fileselect.setparm (
			CENTER, 
			CENTER, 
			f.height()-8, 
			f.width()-8, 
			NEWFILESELECT_TITLE,
			NEWFILESELECT_LABEL
			);

	file = fileselect.activateonce();
	if (file)
	{
		warning ("new file = %s", file);
		_mix->add_to_playlist (activechan, file);
	}
	canupdate = true;
}

void NCURSES_GUI::showhelp (void)
{
	CDKScroll help;
	char ** helpmsg;
	int count;

	count = 11;
	helpmsg = (char **) malloc (70*count);

	*(helpmsg + 0) = " DOWN  -  scroll down playlist";
	*(helpmsg + 1) = " UP    -  scroll up playlist";
	*(helpmsg + 2) = " F1    -  flip over win 1";
	*(helpmsg + 3) = " F2    -  flip over win 2";
	*(helpmsg + 4) = " F3    -  flip over win 3";
	*(helpmsg + 5) = " 1-6   -  goto chan (automagically change win)";
	*(helpmsg + 6) = " TAB   -  switch between chan on the same win";
	*(helpmsg + 7) = " ";
	*(helpmsg + 8) = " ENTER -  play                    [+-] - volume                       ";
	*(helpmsg + 9) = " s     -  stop                    [<>] - fade                         ";
	*(helpmsg + 10) =" m     -  play mode               0    - set volume to 0              ";

	help.setscreen (f.screen());
	help.setparm (0, 5, 5, f.height()-10, f.width()-10, helpmsg, count, 0, A_NORMAL);
	canupdate = false;
	help.draw();
	help.waitkey();
	help.destroy();
	canupdate = true;
	free (helpmsg);
}

void NCURSES_GUI::showabout (void)
{
	aboutbox about;

	about.setscreen (f.screen());
	about.setitem();
	about.setparm (0, CENTER, CENTER, f.height()-10, 80, 0, A_NORMAL);
	canupdate = false;
	about.draw();
	about.activate();
	about.destroy();
	canupdate = true;
}

void NCURSES_GUI::set_lcd (unsigned int chan, char *lcd) {
  //if(ch_lcd[chan]) free(ch_lcd[chan]);
  //ch_lcd[chan] = strdup(lcd);
  if (canupdate) {
	  if ((vischan[0] == chan) | (vischan[1] == chan)) curs_chan[chan].set_time (lcd);
  }
}

void NCURSES_GUI::set_pos(unsigned int chan, float pos) {
  ch_pos[chan] = pos;
  if (canupdate) {
	  if ((vischan[0] == chan) | (vischan[1] == chan)) curs_chan[chan].set_pos (pos);
  }
}

void NCURSES_GUI::set_title(char *txt) {
	func ("NCURSES_GUI::set_title()");
	if (canupdate) {
		title.setvalue (txt);
	} else { 
		title.setvalueNR (txt);
	}
}

void NCURSES_GUI::set_status(char *txt) {
	if (canupdate) {
		statusbar.setvalue (txt); 
	} else {
		statusbar.setvalueNR (txt);
	}
}

void NCURSES_GUI::add_playlist(unsigned int ch, char *txt) {
	curs_chan[ch].add_to_playlist (txt);
}

void NCURSES_GUI::sel_playlist(unsigned int ch, int row) {

}

