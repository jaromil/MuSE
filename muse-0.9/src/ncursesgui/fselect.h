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

#ifndef __RUB_FSELECT_H__
#define __RUB_FSELECT_H__

extern "C" {
#include <libcdk/cdk.h>
}

class CDKFselect
{
	CDKFSELECT *fselect;
	CDKSCREEN *cdkscreen;
	public:
	bool box;

	CDKFselect();
	~CDKFselect();
	char * activate(void);
	char * activateonce (void);
	void setscreen (CDKSCREEN *);
	void setparm (int xpos, int ypos, int height, int width, char *title, char *label);
	void setbox (bool);
	void draw(void);
	void erase(void);
};




#endif
