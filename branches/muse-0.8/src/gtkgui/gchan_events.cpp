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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <jutils.h>

#include <gen.h>
#include <gchan.h>
#include <gchan_events.h>
#include <listpack.h>

#include <jmixer.h>
#include <config.h>

void gcb_event_pause_channel(GtkWidget *w, GdkEventButton *s, struct gchan *o)
{
	if(o->channel != 1)
		return;
	
	gcb_pause_channel(w, o);
}

void gcb_event_set_position(GtkWidget *w, GdkEventButton *s, struct gchan *o)
{
	float position;

	if(o->channel != 1) {
		/* get your hands off! */
		gtk_adjustment_set_value(GTK_ADJUSTMENT(o->adjprog), 0.0);
		return;
	}

	position=GTK_ADJUSTMENT(o->adjprog)->value;
	mixer->set_position(o->idx-1, position);
	/* muse-core will play channel with set_position */	
}

void gcb_event_clist_popup(GtkWidget *w, GdkEventButton *s, struct gchan *o)
{
	GtkWidget *tmpwid;
	GtkWidget *menupop=gtk_menu_new();
	gint row=-1, col;
	
	if(s->button == 3) {
		tmpwid=gtk_menu_item_new_with_label("Add File...");
		gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
				(GtkSignalFunc) spawnfilew, o);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);

		tmpwid=gtk_menu_item_new_with_label("Add Url...");
		gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
				(GtkSignalFunc) httpwin, o);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);
		
		tmpwid=gtk_menu_item_new_with_label("Delete");
		
		gtk_clist_get_selection_info(GTK_CLIST(w), (int) s->x, (int) s->y,
				&row, &col);
		o->rem=row;
		gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
				(GtkSignalFunc) gcb_rem_from_playlist, o);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);
		tmpwid=gtk_menu_item_new_with_label("Cancel");
		gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
				(GtkSignalFunc) gtk_widget_destroy, NULL);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);

		gtk_signal_connect(GTK_OBJECT(menupop), "selection-done",
				(GtkSignalFunc) gtk_widget_destroy, menupop);
		gtk_menu_popup(GTK_MENU(menupop), NULL, NULL, NULL, NULL,
				s->button, s->time);
		
		gtk_widget_show_all(menupop);
		
	}
}

