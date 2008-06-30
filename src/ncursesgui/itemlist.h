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

#ifndef __RUB_Itemlist_H__
#define __RUB_Itemlist_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class CDKItemlist
{
	CDKITEMLIST *itemlist;
	CDKSCREEN *cdkscreen;
//	char *title;
	public:
	bool box;

	CDKItemlist();
	~CDKItemlist();
	void setparm (char *, char *, int, int, char **, int, int, bool);
	void setscreen (CDKSCREEN *);
	CDKSCREEN * getscreen (void);
	void reg(void);
	void unreg(void);
	void draw(void);
	int activate (void);
	void setitem (int);
	int getcurritem (void);
	void erase(void);
	void destroy (void);
	chtype lastkey (void);
};

#endif
