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

#ifndef __RUB_LABEL_H__
#define __RUB_LABEL_H__

#include "screen.h"

extern "C" {
#include <libcdk/cdk.h>
}

class CDKLabel {
	private:
		CDKLABEL * cdklabel;
		CDKSCREEN *cdkscreen;
	public:
		
		CDKLabel();
		~CDKLabel();
		bool box;
		bool shadow;
		void setparm (int, int, int, int);
		void setscreen (CDKSCREEN *);
		CDKSCREEN * getscreen (void);
		void reg (void);
		void unreg (void);
		void setvalue (char **, int);
		void setvalueNR (char **, int);
		void setvalue (char *);
		void setvalueNR (char *);
		void draw (void);
		void ulhook(chtype);
		void urhook(chtype);
		void llhook(chtype);
		void lrhook(chtype);
		void mark(void);
		void unmark (void);
		void waitkey (void);
		CDKLABEL *label (void);
		void erase (void);
		void destroy (void);
		void raise (void);
};

#endif
