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

#ifndef __CDKSCREEN_H__
#define __CDKSCREEN_H__

#include "screen.h"
#include "muse_console.h"

CDKScreen::CDKScreen() 
{
  cursesWin = initscr();
  keypad (cursesWin, TRUE);
  cdkscreen = initCDKScreen(cursesWin);
  initCDKColor();
}

CDKScreen::~CDKScreen() 
{
	warning ("screen destructor");
	if (cdkscreen) {
		destroyCDKScreen (cdkscreen);
		cdkscreen=NULL;
	}
	endCDK();
	warning ("end screen destructor");
}

void CDKScreen::empty()
{
	eraseCDKScreen (cdkscreen);
}

CDKSCREEN *CDKScreen::screen(void) 
{
  return cdkscreen;
}

int CDKScreen::objcnt(void)
{
	return cdkscreen->objectCount;
}

void CDKScreen::refreshscr(void) 
{
  refreshCDKScreen(cdkscreen);
}

void CDKScreen::erasescr(void) 
{
	eraseCDKScreen(cdkscreen);
}

int CDKScreen::width(void)
{
	return getmaxx(cdkscreen->window);
}

int CDKScreen::height(void)
{
	return getmaxy(cdkscreen->window);
}

void CDKScreen::refresh(void)
{
	refreshCDKScreen (cdkscreen);
}
	
#endif
