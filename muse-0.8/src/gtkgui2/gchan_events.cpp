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

/* drag'n'drop stuff */

void DND_begin(GtkWidget *w, GdkDragContext *dc, struct gchan *o)
{
	notice("drag_begin");
	
}

void DND_end(GtkWidget *w, GdkDragContext *dc, struct gchan *o)
{
	notice("drag_end");
}

gboolean DND_data_motion(GtkWidget *w, GdkDragContext *dc, gint x, gint y,
				guint t, struct gchan *o)
{
	gdk_drag_status(dc, GDK_ACTION_MOVE, t);
	notice("drag_data_motion");
	return FALSE;
}

gboolean DND_data_get(GtkWidget *w, GdkDragContext *dc, 
				GtkSelectionData *selection, guint info, struct gchan *o)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select;
	gchar *title;
	
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
	if(	gtk_tree_selection_get_selected(select, &model, &iter)) {
		
		gtk_tree_model_get(model, &iter, 
				TITLE, &title, 
				-1);

		if(title) {
			gtk_selection_data_set(selection, GDK_SELECTION_TYPE_STRING,
					8, /* 8 bits per character */
					(guchar *) title, strlen(title));
		}
		notice("drag_data_get");
		return TRUE;
	}
	
	return FALSE;
}

void DND_data_received(GtkWidget *w, GdkDragContext *dc, gint x, gint y,
				GtkSelectionData *selection, guint info, guint t,
				struct gchan *o)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select;
	GtkTreePath *path;
	gint row=0;
	gchar *title;

	if(selection == NULL)
		return;
	if(selection->length < 0)
		return;
	
	title = (gchar *) selection->data;
	
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(w),
				x, y, &path, NULL, NULL, NULL)) {
		row = gtk_tree_path_get_indices(path)[0];
		notice("row = %d", row);
		gtk_tree_path_free(path);
	}
	
	gtk_tree_selection_get_selected(select, &model, NULL);
	gtk_list_store_insert(GTK_LIST_STORE(model), &iter, row);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			TITLE, title,
			-1);
	
	notice("drag_data_received");
}

void DND_data_delete(GtkWidget *w, GdkDragContext *dc, struct gchan *o)
{
	notice("drag_data_delete");
}

/* end */

gboolean gcb_event_view_popup(GtkWidget *w, GdkEventButton *s, struct gchan *o)
{
	GtkWidget *tmpwid;
	GtkWidget *menupop = gtk_menu_new();
	GtkTreeSelection *select;
	GtkTreePath *path;
	gint *row = NULL;
	
	if(s->button == 3) {
		select = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
		if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(w), 
				(gint) s->x, (gint) s->y,
				&path, NULL, NULL, NULL)) {
			gtk_tree_selection_unselect_all(select);
			
			g_signal_handler_block(G_OBJECT(select), 
					blockid[o->idx]);
			gtk_tree_selection_select_path(select, path);
			g_signal_handler_unblock(G_OBJECT(select), 
					blockid[o->idx]);
			
			if((row = gtk_tree_path_get_indices(path)))
				act(_("il row e' %d"), row[0]);
			gtk_tree_path_free(path);
		}
		
		tmpwid = gtk_menu_item_new_with_label(_("Add File..."));
		g_signal_connect(G_OBJECT(tmpwid), "activate",
				G_CALLBACK(spawnfilew), o);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);

		tmpwid = gtk_menu_item_new_with_label(_("Add Url..."));
		g_signal_connect(G_OBJECT(tmpwid), "activate",
				G_CALLBACK(httpwin), o);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);
		
		if(row) {
			tmpwid = gtk_menu_item_new_with_label(_("Delete"));
			g_signal_connect(G_OBJECT(tmpwid), "activate",
					G_CALLBACK(gcb_rem_from_playlist), 
					o);
			gtk_menu_append(GTK_MENU(menupop), tmpwid);
		}
		
		tmpwid = gtk_menu_item_new_with_label(_("Cancel"));
		g_signal_connect(G_OBJECT(tmpwid), "activate",
				G_CALLBACK(gtk_widget_destroy), NULL);
		gtk_menu_append(GTK_MENU(menupop), tmpwid);

		g_signal_connect(G_OBJECT(menupop), "selection-done",
				G_CALLBACK(gtk_widget_destroy), menupop);
		gtk_menu_popup(GTK_MENU(menupop), NULL, NULL, NULL, NULL,
				s->button, s->time);
		
		gtk_widget_show_all(menupop);
		return TRUE;
	}
	return FALSE;
	
}

