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

#ifndef __M_ENTRY__H__
#define __M_ENTRY__H__

#include "entry.h"
#include "muse_console.h"

CDKEntry::CDKEntry()
{
	entry = NULL;
};

CDKEntry::~CDKEntry()
{
	warning ("CDKEntry::~CDKEntry");
	if (entry) {
		destroyCDKEntry (entry);
	}
	entry=NULL;
};

void CDKEntry::destroy()
{
	warning ("CDKEntry::destroy () at %p", entry);
	if (entry) {
		destroyCDKEntry (entry);
	}
	entry=NULL;
	warning ("end CDKEntry::destroy ()");
}

void CDKEntry::draw()
{
	drawCDKEntry (entry, box);
};

void CDKEntry::reg (void)
{
	registerCDKObject (cdkscreen, vENTRY, entry);
}

void CDKEntry::unreg (void)
{
	unregisterCDKObject (vENTRY, entry);
}


void CDKEntry::setparm(char *title, char *label, int fieldwidth, int xpos, int ypos)
{
	entry = newCDKEntry (cdkscreen,
			xpos,
			ypos,
			title,
			label,
			A_NORMAL,
			' ',
			vMIXED,
			fieldwidth,
			0,
			255,
			TRUE,
			FALSE
			);
}


void CDKEntry::setparm(char *title, char *label, int fieldwidth, int xpos, int ypos, bool box, chtype fillerchar)
{
	entry = newCDKEntry (cdkscreen,
			xpos,
			ypos,
			title,
			label,
			A_NORMAL,
			fillerchar,
			vMIXED,
			fieldwidth,
			0,
			255,
			box,
			FALSE
			);
}

void CDKEntry::setparm(char *title, char *label, int fieldwidth, int xpos, int ypos, bool box, chtype fillerchar, chtype fieldattribute, EDisplayType displayType)
{
	entry = newCDKEntry (cdkscreen,
			xpos,
			ypos,
			title,
			label,
			fieldattribute,
			fillerchar,
			displayType,
			fieldwidth,
			0,
			255,
			box,
			FALSE
			);
}

void CDKEntry::setfillerchar (chtype c)
{
	setCDKEntryFillerChar (entry, c);
}

void CDKEntry::erase (void)
{
	eraseCDKEntry (entry);
}

void CDKEntry::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

char * CDKEntry::activate()
{
	char * data;
	data = activateCDKEntry (entry, 0);
	return data;
}

char * CDKEntry::activateonce()
{
	char * data;
	data = activateCDKEntry (entry, 0);
	erase();
	return data;
}

void CDKEntry::setvalue (char *txt)
{
	setCDKEntryValue (entry, txt);
}
	
char * CDKEntry::getvalue (void)
{
	return getCDKEntryValue (entry);
}

chtype CDKEntry::lastkey()
{
	return (entry->LastKey);
}

#endif
