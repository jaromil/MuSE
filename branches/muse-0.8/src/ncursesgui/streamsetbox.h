/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2003 Luca 'rubik' Profico <rubik@olografix.org>
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

#ifndef __STREAMSETBOX_H__
#define __STREAMSETBOX_H__

#include "screen.h"
#include "entry.h"
#include "menu.h"
#include "label.h"
#include "itemlist.h"
#include "jmixer.h"
#include "encdata.h"

extern "C" {
#include <libcdk/cdk.h>
}

class StreamSetBox {
	private:
		CDKSCREEN *cdkscreen;
		char *logtypelist[4];
		CDKEntry host, port, mnt, name, url, description, pass;
		CDKItemlist logintype;
		CDKLabel label1;
		// CDKMenu icecast;
		int xbase, ybase;
		Stream_mixer *mix;
		encdata *enc;
	public:
		StreamSetBox ();
		~StreamSetBox ();
		void setmixer (Stream_mixer *mix, encdata *enc);
		void setscreen (CDKSCREEN *);
		void setparm (int, int);
		void refresh (void);
		void erase (void);
		void unreg(void);
		void draw (void);
		void activate (void);
		void showcurrval (int);
		void setval (int);
		void setdefaultval (void);
		int tabentry (int);
};


#endif

