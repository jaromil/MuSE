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

#ifndef __RUB_MENU_H__
#define __RUB_MENU_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class CDKMenu {
	private:
		CDKMENU *menu;
		CDKSCREEN *cdkscreen;
		bool box;
	public:
		CDKMenu();
		~CDKMenu();
		void setscreen (CDKSCREEN *);
		void setparm (char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS], int, int *, int *, int, chtype, chtype );
		void draw (bool);
		void draw (void);
		void activate(void);
		/*
		void erase (void);
		void destroy (void);
		*/
};
	
#endif


