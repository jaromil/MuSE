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

#include "fselect.h"
#include "muse_console.h"

CDKFselect::CDKFselect()
{
}

CDKFselect::~CDKFselect()
{
	if (fselect) {
		destroyCDKFselect (fselect);
		fselect =NULL;
	}
}

char * CDKFselect::activate (void)
{
	char *data;
	data = activateCDKFselect (fselect, 0);
	return data;
}

char * CDKFselect::activateonce (void)
{
	char * data;
	data = activateCDKFselect (fselect, 0);
	erase();
	return data;
}


void CDKFselect::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void CDKFselect::setparm(int xpos, int ypos, int height, int width, char *title, char *label)
{
	warning ("h: %d, w: %d", height, width);
	fselect = newCDKFselect (
			cdkscreen,
			xpos,
			ypos,
			height,
			width,
			title,
			label,
			A_NORMAL,
			'.',
			A_REVERSE,
			"</5>",
			"</48>",
			"</N>",
			"</N>",
			box,
			FALSE
			);
}

void CDKFselect::setbox (bool b)
{
	box = b;
}

void CDKFselect::draw(void)
{
	drawCDKFselect (fselect, box);

}

void CDKFselect::erase(void)
{
	eraseCDKFselect (fselect);
}
