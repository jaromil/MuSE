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

#include "itemlist.h"
#include "muse_console.h"

CDKItemlist::CDKItemlist()
{
	itemlist = NULL;
}

CDKItemlist::~CDKItemlist()
{
	func ("CDKItemlist::~CDKItemlist ()");
	if (itemlist) {
		destroyCDKItemlist (itemlist);
	}
	itemlist=NULL;
}

void CDKItemlist::destroy()
{
	func ("CDKItemlist::destroy ()");
	if (itemlist) {
		destroyCDKItemlist (itemlist);
	}
	itemlist=NULL;
	func ("end CDKItemlist::destroy ()");
}

void CDKItemlist::setscreen (CDKSCREEN *s)
{
	cdkscreen = s;
}

CDKSCREEN * CDKItemlist::getscreen (void)
{
	return cdkscreen;
}

void CDKItemlist::setparm (char *title, char *label, int xpos, int ypos, char **list, int count, int defaultitem, bool box)
{
	itemlist = newCDKItemlist (cdkscreen,
			xpos, ypos,
			title,
			label,
			list,
			count,
			defaultitem,
			box,
			FALSE);
}

void CDKItemlist::reg (void)
{
	registerCDKObject (cdkscreen, vITEMLIST, itemlist);
}

void CDKItemlist::unreg (void)
{
	unregisterCDKObject (vITEMLIST, itemlist);
}

void CDKItemlist::draw()
{
	drawCDKItemlist (itemlist, box);
}

int CDKItemlist::activate()
{
	int data;
	data = activateCDKItemlist (itemlist, 0);
	return data;
}

void CDKItemlist::erase ()
{
	eraseCDKItemlist (itemlist);
}

void CDKItemlist::setitem (int i)
{
	setCDKItemlistCurrentItem (itemlist, i);
}

int CDKItemlist::getcurritem (void)
{
	return getCDKItemlistCurrentItem (itemlist);
}

chtype CDKItemlist::lastkey()
{
	return (itemlist->LastKey);
}
