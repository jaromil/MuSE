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

#ifndef __RUB_SLIDER_H__
#define __RUB_SLIDER_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class CDKSlider 
{
	CDKSLIDER *slider;
	CDKSCREEN *cdkscreen;
	char *title, *label;
	int fieldwidth;
	int i_curpos;
	public:
	int lowValue;
	int highValue;
	int currentValue;
	bool box;
	
	CDKSlider();
	~CDKSlider();
	//CDKSLIDER *slider(void);
	void activate();
	void setscreen(CDKSCREEN *);
	void reg (void);
	void unreg (void);
	void setparm(char * title, char* label, int fieldwidth, int xpos, int ypos);
	void draw(void);
	void hide(void);
	void incr (void);
	void decr (void);
	void set_pos (float);
	float fpos (void);
	};
	
#endif


