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

#include "play_infobox.h"
#include "muse_console.h"

PlayInfoBox::PlayInfoBox()
{
  /*
	song = (char *) malloc (255) ;
	elapsed = (char *) malloc (32) ;
	mode = (char *) malloc (32);
	act = (char *) malloc (32);
	*/

	message[0] = (char *) malloc (255);
	message[1] = (char *) malloc (255);
  
	snprintf (song,255, "null");
	snprintf (elapsed,32, "</B>Elapsed:<!B> 00:00:00");
	snprintf (mode,32, " </B>Mode:<!B> null");
	snprintf (curract,32, " ");

	snprintf (message[0],255,"%s", song);
	snprintf (message[1],255,"%s %s %s", elapsed,mode,curract);
}

PlayInfoBox::~PlayInfoBox()
{
  func("PlayInfoBox::~PlayInfoBox()");
	free (message[0]);
	free (message[1]);
}

void PlayInfoBox::setscreen (CDKSCREEN *screen)
{
	cdkscreen=screen;
}

void PlayInfoBox::reg (void)
{
	func ("PlayInfoBox::reg");
	registerCDKObject (cdkscreen, vLABEL, infobox);
}

void PlayInfoBox::unreg (void)
{
	func ("PlayInfoBox::unreg() screen=%p",cdkscreen);
	unregisterCDKObject (vLABEL, infobox);
	func ("end PlayInfoBox::unreg()");
}

void PlayInfoBox::setparm(int xpos, int ypos, int width)
{
  /*int i;
    
  for (i=0; i<width-5; i++)
  {
  strncat (message[0], " ", 1);
  } //jrml*/
  memset(message[0],' ',width-1);
  memset(message[1],' ',width-1);
  
  *(message[0]+width-1) = '\0';
  *(message[1]+width-1) = '\0';
  
  infobox = newCDKLabel (
			cdkscreen,		    
			xpos, 
			ypos,
			(char **)message, 
			2,
			TRUE, 
			FALSE
			);

	setCDKLabelLLChar (infobox, ACS_LTEE);
	setCDKLabelLRChar (infobox, ACS_RTEE);
}

void PlayInfoBox::setsong(char * song)
{
	sprintf (message[0], "%s",song); // bof??
	setCDKLabel (
			infobox,
			(char**)message,
			2,
			TRUE
			);
}

void PlayInfoBox::setsongNR(char * song)
{
	sprintf (message[0], "%s",song); // bof??
	setCDKLabelNR (
			infobox,
			(char**)message,
			2,
			TRUE
			);
}

void PlayInfoBox::setelapsed(char * newelap)
{
	snprintf(elapsed,32,"</B>Elapsed:<!B>%s",newelap);
	snprintf(message[1],255,"%s %s %s",elapsed,mode,curract);

	setCDKLabel (
			infobox,
			(char**)message,
			2,
			TRUE
		      );
}

void PlayInfoBox::setelapsedNR(char * newelap)
{
	snprintf(elapsed,32,"</B>Elapsed:<!B>%s",newelap);
	snprintf(message[1],255,"%s %s %s",elapsed,mode,curract);

	setCDKLabelNR (
			infobox,
			(char**)message,
			2,
			TRUE
		      );
}

void PlayInfoBox::setmode (char * newmode)
{
	func ("PlayInfoBox::setmode()");
	
  	snprintf(mode,32,"</B>Mode:<!B> %s",newmode);
	snprintf(message[1],255,"%s %s %s",elapsed,mode,curract);
	setCDKLabel (
			infobox,
			(char**)message,
			2,
			TRUE
			);
}

void PlayInfoBox::setmodeNR (char * newmode)
{
	func ("PlayInfoBox::setmode()");
	
  	snprintf(mode,32,"</B>Mode:<!B> %s",newmode);
	snprintf(message[1],255,"%s %s %s",elapsed,mode,curract);
	setCDKLabelNR (
			infobox,
			(char**)message,
			2,
			TRUE
			);
}

void PlayInfoBox::setact (const char * newact)
{
	func ("PlayInfoBox::setact()");
	snprintf(curract,32,"%s",newact);
	sprintf(message[1],"%s %s %s",elapsed,mode,curract);
	
	setCDKLabelMessage (
			infobox,
			message,
			2
			);

}

void PlayInfoBox::setactNR (const char * newact)
{
	snprintf(curract,32,"%s",newact);
	sprintf(message[1],"%s %s %s",elapsed,mode,curract);
	
	setCDKLabelMessage_NR (
			infobox,
			message,
			2
			);

}

void PlayInfoBox::setctrl (void)
{
	func ("PlayInfoBox::setctrl()");
	setact (" [CTRL]");
	setCDKLabelBoxAttribute (infobox, A_BOLD);
	setCDKLabelBackgroundColor (infobox, "</21>");
}

void PlayInfoBox::setctrlNR (void)
{
	setactNR (" [CTRL]");
	setCDKLabelBoxAttribute (infobox, A_BOLD);
	setCDKLabelBackgroundColor (infobox, "</21>");
}

void PlayInfoBox::unsetctrl (void)
{
	setact (" ");
	setCDKLabelBoxAttribute (infobox, A_NORMAL);
	setCDKLabelBackgroundColor (infobox, "</0>");
}

void PlayInfoBox::unsetctrlNR (void)
{
	setactNR (" ");
	setCDKLabelBoxAttribute (infobox, A_NORMAL);
	setCDKLabelBackgroundColor (infobox, "</0>");
}


void PlayInfoBox::draw(void)
{
	drawCDKLabel (infobox, TRUE);
}

void PlayInfoBox::hide(void)
{
	eraseCDKLabel (infobox);
}

