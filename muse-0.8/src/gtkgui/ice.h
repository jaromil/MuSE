/* A Gui in gtk+ for MuSE - Multiple Streaming Engine
 * Copyright (C) 2002 nightolo <night@autistici.org>
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
 */

#ifndef ICE_H
#define ICE_H

struct icedata {
	GtkWidget *frami;
	/* ice space */
	GtkWidget *host, *port, *mnt, *name, *url, *desc;
	GtkWidget *logintype, *pass, *conn;
	GList *combi;
	GtkWidget *lista, *profroot;
	/* new field 0.8 */
	unsigned int idx, pos;
	/* shouter stuff */
	int iceid; 
	Shouter *coreice;
	OutChannel *outchan;
	bool mp3; /* is this a OutLame? */
};

/* drawing functions */
void ice_window(GtkWidget *);
void ice_new_from_menu(GtkWidget *);
void ice_new(codec); 
void ice_profmenu(struct icedata *);

void ice_free(struct icedata *);
void ice_put(struct icedata *);
void ice_fill(struct icedata *);

void gcb_set_icecast(GtkWidget *, struct icedata *);
void gcb_rem_icecast(GtkWidget *, struct icedata *);

void gcb_ice_put(GtkWidget *, struct icedata *);
void gcb_ice_save(GtkWidget *, struct icedata *);

#endif
