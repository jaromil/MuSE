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

#include "muse_tui.h"

muse_tui::muse_tui()
{
	version[0] = (char *) malloc (55);
	strcpy (version[0], "MuSE 0.9.9 sturata scappellament0");
	status[0] = (char *) malloc (55);
	strcpy (status[0], " Status::");
	minibox[0] = (char *) malloc (16);
	strcpy (minibox[0], "");
	shortcuts[0] = (char *) malloc (260);
	strcpy (shortcuts[0], SHORTCUTS_HELP);
}

muse_tui::~muse_tui()
{
	free (version[0]);
	free (status[0]);
	free (minibox[0]);
	free (shortcuts[0]);

}

void
muse_tui::refresh ()
{
  //int i;
  setlabels ();
  /*
  for (i = 0; i < MAX_CHANNELS; i++)
    {
      if (i % 2 == 0)		//tutti a sinistraaaa ehehhhhh
	chan[i].init (f.screen (), 1, i);
      else			//e tutti a destra ohhohhhhh
	chan[i].init (f.screen (), f.width () / 2, i);
    }
    */

  f.refresh ();
  setactivechan (activechan);
  /*
  chan[0].show ();
  chan[1].show ();
  */
  // setactivechan (0);
}

void muse_tui::start()
{
	int i;

	setlabels();
	//setdialogs();

	for (i=0;i< MAX_CHANNELS;i++)
	{
		if (i%2 == 0) { //tutti a sinistraaaa ehehhhhh
			chan[i].init (f.screen(),1,i);
		} else { //e tutti a destra ohhohhhhh
			chan[i].init (f.screen(), f.width()/2, i);
		}
	}


	f.refresh();
	chan[0].show();
	chan[1].show();
	setactivechan (0);
	userinput();
}

void muse_tui::setlabels (void)
{
	title.setscreen (f.screen());
	title.box = TRUE;
	title.setparm(1,1,f.width() - 4,1);
	title.setvalue (version,1);
	title.draw();

	statusbar.setscreen (f.screen());
	statusbar.box = TRUE;
	statusbar.setparm(16,f.height()-3,f.width()-19,1);
	statusbar.setvalue (status, 1);
	statusbar.urhook (ACS_RTEE);
	statusbar.ulhook (ACS_TTEE);
	statusbar.llhook (ACS_LTEE);
	statusbar.draw();

	helpbar.setscreen (f.screen());
	helpbar.box = TRUE;
	helpbar.setparm (1,f.height() -5, f.width() - 4, 1);
	helpbar.setvalue (shortcuts,1);
	helpbar.llhook (ACS_LTEE);
	helpbar.lrhook (ACS_RTEE);
	helpbar.draw();

	opchan.setscreen (f.screen());
	opchan.box=TRUE;
	opchan.setparm (1, f.height() -3, 15, 1);
	opchan.ulhook (ACS_LTEE);
	opchan.urhook (ACS_TTEE);
	opchan.lrhook (ACS_BTEE);
	opchan.draw();
}

void muse_tui::setdialogs (void)
{
}

void muse_tui::changewin(int newwin)
{
	int gc;

	gc = newwin * 2 + (activechan %2);
	if (gc != activechan) setactivechan (gc);

}

void muse_tui::stop()
{
	warning ("muse_tui.stop()");
}

int muse_tui::userinput ()
{
	int ch;
	while ((ch = getch ()) != 'q')
	{
		warning ("input: int %d",  ch);
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
				chan[activechan].selprevitem();
				break;
			case KEY_DOWN:
				chan[activechan].selnextitem();
				break;
			case '+':
				chan[activechan].incrvol();
				break;
			case '-':
				chan[activechan].decrvol();
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
			case 'a':
			case 'A':
				showabout();
				refresh();
				break;
			case 's':
			case 'S':
				/* speaker */
				break;
			case 'l':
			case 'L':
				/* line-in */
				break;
			case 'c':
			case 'C':
				/* connect */
				break;
			case 'h':
			case 'H':
				/* connect */
				showhelp();
				refresh();
				break;
			default:
				continue;
		}
	}
	return 0;
}

void muse_tui::setactivechan (int chanID)
{
	chan[activechan].unsetctrl();
	activechan = chanID;
	chan[activechan].setctrl();
	chan[activechan].show();

	if (activechan % 2 == 0) { 
		chan[activechan + 1].show();
	} else { 
		chan[activechan - 1].show();
	}

	snprintf (minibox[0], 15, "win: %d chan: %d", 
			(activechan /2) +1, activechan + 1);

	opchan.setvalue(minibox, 1);
}

int muse_tui::getactivechan (void)
{
	return activechan;
}

void muse_tui::tabchan (void)
{
	warning ("tab chan");
	if (activechan % 2 == 0) { 
		setactivechan (activechan + 1);
	} else { 
		setactivechan (activechan - 1);
	}
}

void muse_tui::newurl (void)
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
	}
}	

void muse_tui::newfile (void)
{
	char *file;
	CDKFselect fileselect;

	warning ("+file select");
	fileselect.setscreen (f.screen());
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
	}
}

void muse_tui::showhelp (void)
{
	CDKScroll help;
	char ** helpmsg;
	int count;

	count = 7;
	helpmsg = (char **) malloc (70*count);

	*(helpmsg + 0) = " DOWN  -  scroll down playlist";
	*(helpmsg + 1) = " UP    -  scroll up playlist";
	*(helpmsg + 2) = " F1    -  flip over win 1";
	*(helpmsg + 3) = " F2    -  flip over win 2";
	*(helpmsg + 4) = " F3    -  flip over win 3";
	*(helpmsg + 5) = " 1-6   -  goto chan (automagically change win)";
	*(helpmsg + 6) = " TAB   -  switch between chan on the same win";

	help.setscreen (f.screen());
	help.setparm (0, 5, 5, f.height()-10, f.width()-10, helpmsg, count, 0, A_NORMAL);
	help.draw();
	help.waitkey();
	help.destroy();
}

void muse_tui::showabout (void)
{
	aboutbox about;

	about.setscreen (f.screen());
	about.setitem();
	about.setparm (0, CENTER, CENTER, f.height()-10, 80, 0, A_NORMAL);
	about.draw();
	about.activate();
	about.destroy();
}
