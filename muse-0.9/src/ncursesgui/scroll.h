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

#ifndef __RUB_SCROLLER_H__
#define __RUB_SCROLLER_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class CDKScroll
{
	private:

	public:
		CDKSCROLL *scroll;
		CDKSCREEN *cdkscreen;
		char **item;
		int count;
		CDKScroll();
		~CDKScroll();
		void setscreen(CDKSCREEN *);
		void reg (void);
		void unreg (void);
		void setparm (char *, int, int, int, int);
		void setparm (char *, int, int, int, int, char**, int, bool, chtype);
		void draw(void);
		void hide(void);
		void selnext(void);
		void selprev(void);
		void additem (char *);
		void delitem (int);
		void ulhook(chtype);
		void urhook(chtype);
		void llhook(chtype);
		void lrhook(chtype);
		void mark(void);
		void unmark(void);
		void activate (void);
		void destroy (void);
		void waitkey (void);
		void scrolldown (void);
		void scrollup (void);
		int curritem (void);
		int currhigh (void);
		int listsize (void);
		int viewsize (void);
};


#endif
