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

#ifndef __RUB_ABOUT_H__
#define __RUB_ABOUT_H__

#include "scroll.h"

class aboutbox : public CDKScroll
{
	private:
		char **item;
		int count;
	public:
		aboutbox();
		~aboutbox();
		void setparm (char *, int, int, int, int, bool, chtype);
		void setitem (void);
		void activate (void);
		char ** splitstring (char *);

};




#endif
