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

#include "scroll.h"
#include "muse_console.h"

CDKScroll::CDKScroll()
{
	item = 0;
}

CDKScroll::~CDKScroll()
{
	warning ("cdkscroll destructor");
	if (item) {
		free (item);
	}
}

void CDKScroll::setparm(char * title, int xpos, int ypos, int height, int width)
{

	func ("CDKScroll::setparm()");
	//count = CDKgetDirectoryContents ("finta/", &item);

	// playlist finta
	item = (char **) malloc (512);
	count = 0;
	
	scroll = newCDKScroll (
			cdkscreen,
			xpos,
			ypos,
			LEFT,
			height, 
			width,
			title,
			item,
			count,
			NUMBERS,
			A_REVERSE,
			TRUE,
			FALSE
			);

}

void CDKScroll::setparm(char * title, int xpos, int ypos, int height, int width, char **item, int count, bool numbers, chtype highlight)
{
	scroll = newCDKScroll (
			cdkscreen,
			xpos,
			ypos,
			LEFT,
			height, 
			width,
			title,
			item,
			count,
			numbers,
			highlight,
			TRUE,
			FALSE
			);
}

void CDKScroll::ulhook (chtype ch)
{
	setCDKScrollULChar (scroll, ch);
}

void CDKScroll::urhook (chtype ch)
{
	setCDKScrollURChar (scroll, ch);
}

void CDKScroll::llhook (chtype ch)
{
	setCDKScrollLLChar (scroll, ch);
}

void CDKScroll::lrhook (chtype ch)
{
	setCDKScrollLRChar (scroll, ch);
}

void CDKScroll::mark (void)
{
	setCDKScrollBoxAttribute (scroll, A_BOLD);
	setCDKScrollBackgroundColor (scroll, "</21>");
}

void CDKScroll::unmark (void)
{
	setCDKScrollBoxAttribute (scroll, A_NORMAL);
	setCDKScrollBackgroundColor (scroll, "</0>");
}

void CDKScroll::selnext (void)
{
	injectCDKScroll (scroll, KEY_DOWN);
}

void CDKScroll::selprev (void)
{
	injectCDKScroll (scroll, KEY_UP);
}

void CDKScroll::additem (char *item)
{
	addCDKScrollItem (scroll, item);
}

void CDKScroll::delitem (int pos)
{
	deleteCDKScrollItem (scroll, pos);
}

void CDKScroll::draw(void)
{
	drawCDKScroll (scroll, TRUE);
}

void CDKScroll::hide(void)
{
	eraseCDKScroll (scroll);
}

void CDKScroll::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void CDKScroll::reg (void)
{
	registerCDKObject (cdkscreen, vSCROLL, scroll);
}

void CDKScroll::unreg (void)
{
	unregisterCDKObject (vSCROLL, scroll);
}

void CDKScroll::activate (void)
{
	activateCDKScroll (scroll, 0);
}

void CDKScroll::destroy (void)
{
	destroyCDKScroll (scroll);
}

void CDKScroll::waitkey (void)
{
	wgetch (scroll->win);
}

void CDKScroll::scrolldown (void)
{
	scrolldownCDKScroll (scroll);
}

void CDKScroll::scrollup (void)
{
	scrollupCDKScroll (scroll);
}

int CDKScroll::curritem (void)
{
	return scroll->currentItem;
}

int CDKScroll::currhigh (void)
{
	return scroll->currentHigh;
}

int CDKScroll::listsize (void)
{
	return scroll->listSize;
}

int CDKScroll::viewsize (void)
{
	return scroll->viewSize;
}
