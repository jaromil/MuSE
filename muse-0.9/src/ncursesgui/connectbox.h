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

#ifndef __CONNECTBOX_H__
#define __CONNECTBOX_H__

#include "screen.h"
#include "entry.h"
#include "menu.h"
#include "label.h"
#include "encsetbox.h"
#include "streamsetbox.h"
#include "itemlist.h"
#include "jmixer.h"
#include "encdata.h"

extern "C" {
#include <libcdk/cdk.h>
}

#define CONNBOX_WIDTH 70
#define CONNBOX_HEIGHT 23

class ConnectBox {
	private:
		CDKSCREEN *cdkscreen;
		WINDOW *subwin;
		CDKLabel bottom;
		StreamSetBox streamsetbox;
		EncSetBox encsetbox;

		int xbase, ybase;

		void showcurrval (void);

		Stream_mixer *mixer;
		encdata *enc;

	public:
		ConnectBox(Stream_mixer *mixer);
		~ConnectBox();
		void setparm (void);
		void refresh (void);
		void erase(void);
		void draw (void);
		void activate (void);
};

#endif
