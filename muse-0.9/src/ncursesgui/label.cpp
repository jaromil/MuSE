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

#include "label.h"
#include "muse_console.h"

CDKLabel::CDKLabel()
{
	cdklabel = NULL;
	box=FALSE;
	shadow=FALSE;
}

CDKLabel::~CDKLabel()
{
	warning ("CDKLabel::~CDKLabel()");
	if (cdklabel) {
		warning ("destroying cdklabel at %p (via destructor)", cdklabel);
		destroyCDKLabel (cdklabel);
		warning ("bah");
		cdklabel =NULL;
	}
	warning ("end CDKLabel::~CDKLabel()");
}

void CDKLabel::destroy()
{
	warning ("CDKLabel::destroy()");
	if (cdklabel) {
		warning ("destroying cdklabel at %p (via destroy method)", cdklabel);
		destroyCDKLabel (cdklabel);
		cdklabel =NULL;
	}
	warning ("end CDKLabel::destroy()");
}


void CDKLabel::setparm(int xpos, int ypos, int messageChars, int messageLines)
{
	char * message[messageLines];
	int l;

	for (l=0;l<messageLines ;l++)
	{
		message[l] = (char *) malloc (messageChars);
		memset (message[l], ' ', messageChars -1 );
		* (message[l] + messageChars - 1) = '\0';
	}

	cdklabel = newCDKLabel (
			cdkscreen,
			xpos,
			ypos,
			(char **) message,
			messageLines,
			box,
			shadow
			);

	for (l=0;l<messageLines ;l++)
	{
		free (message[l]);
	}
};

void CDKLabel::setvalue (char ** message, int messageLines)
{
	setCDKLabel (
			cdklabel,
			message,
			messageLines,
			box
		    );
}

void CDKLabel::setvalueNR (char ** message, int messageLines)
{
	setCDKLabelNR (
			cdklabel,
			message,
			messageLines,
			box
		    );
}

void CDKLabel::setvalue (char *message)
{

	setCDKLabel (
			cdklabel,
			&message,
			1,
			box
		    );
}

void CDKLabel::setvalueNR (char *message)
{

	setCDKLabelNR (
			cdklabel,
			&message,
			1,
			box
		    );
}


void CDKLabel::setscreen (CDKSCREEN * screen)
{
	cdkscreen = screen;
}

CDKSCREEN * CDKLabel::getscreen (void)
{
	return cdkscreen;
}

void CDKLabel::raise (void)
{
	raiseCDKObject (vLABEL, cdklabel);
}

void CDKLabel::reg (void)
{
	registerCDKObject (cdkscreen, vLABEL, cdklabel);
}

void CDKLabel::unreg (void)
{
	unregisterCDKObject (vLABEL, cdklabel);
}

void CDKLabel::draw (void)
{
	drawCDKLabel (cdklabel, box);
}

void CDKLabel::ulhook(chtype ch)
{
	setCDKLabelULChar  (cdklabel, ch);
}

void CDKLabel::urhook(chtype ch)
{
	setCDKLabelURChar  (cdklabel, ch);
}

void CDKLabel::llhook(chtype ch)
{
	setCDKLabelLLChar  (cdklabel, ch);
}

void CDKLabel::lrhook(chtype ch)
{
	setCDKLabelLRChar  (cdklabel, ch);
}

void CDKLabel::mark (void)
{
	setCDKLabelBoxAttribute (cdklabel, A_BOLD);
}

void CDKLabel::unmark (void)
{
	setCDKLabelBoxAttribute (cdklabel, A_NORMAL);
}

void CDKLabel::waitkey (void)
{
	wgetch(cdklabel->win);
}

void CDKLabel::erase (void)
{
	eraseCDKLabel (cdklabel);
}

CDKLABEL *CDKLabel::label (void)
{
	return cdklabel;
}
