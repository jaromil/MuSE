/* A Gui in gtk+ for MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 nightolo <night@autistici.org>
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

#ifndef LISTPACK_H
#define LISTPACK_H

struct listitem {
	unsigned int pos, idx;
	GtkWidget *container;
	void *data;
};

void list_init(GList **);
bool list_add(GList **, void *, unsigned int, unsigned int, GtkWidget *);
bool list_remove(GList **, unsigned int);

struct listitem *list_find_bypos(GList *, unsigned int);
struct listitem *list_find_byidx(GList *, unsigned int);

bool list_set_pos(GList *, GtkWidget *, unsigned int);
void *list_get_data(GList *, unsigned int, unsigned int);
void list_debug(GList *);

void list_swap(GList*, GtkWidget *, GtkWidget *);

unsigned int pack_insert(GList *, GtkWidget *, GtkWidget *);
unsigned int pack_chan_insert(GtkWidget *);
void pack_refresh(GList *, GtkWidget *, bool);

#endif

