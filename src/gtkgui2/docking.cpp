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


#include <stdlib.h>
#include <gtk/gtk.h>
#include <gen.h>
#include <gchan.h>
#include <utils.h>
#include <docking.h>
#include <listpack.h>

#include <jmixer.h>

#include <xpm2/dock.xpm>
//#include <xpm/undock.xpm>

#include <config.h>

void dockchan(GtkWidget *w, struct gchan *c)
{
	GtkWidget *win;
	struct gchan *tmp;
	char buf[30];
	
	tmp = c;
	g_object_ref(G_OBJECT(tmp->frami));
	gtk_container_remove(GTK_CONTAINER(GTK_WIDGET(tmp->frami)->parent), 
			tmp->frami);
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	c->windock = win;
	list_swap(listachan, tmp->frami, c->windock);
	
	/* docking/crossfade bug hotfix --night */
	switch(c->pos) {
		case 1: gtk_widget_hide(pack1.hscale);
			break;
		case 2: gtk_widget_hide(pack1.hscale);
			break;
		case 3: gtk_widget_hide(pack2.hscale);
			break;
		case 4: gtk_widget_hide(pack2.hscale);
			break;
		case 5: gtk_widget_hide(pack3.hscale);
			break;
		case 6: gtk_widget_hide(pack3.hscale);
			break;
	}
	
	list_set_pos(listachan, c->windock, 0);
	
	snprintf(buf, sizeof(buf), _("Channel[%u]"), tmp->idx);
	gtk_window_set_title(GTK_WINDOW(win), buf);
	g_signal_connect(G_OBJECT(win), "delete_event",
			G_CALLBACK(deletedocked), c);
	gtk_container_add(GTK_CONTAINER(win), tmp->frami);
	gtk_widget_destroy(GTK_WIDGET(c->dock));
	c->dock = createpixmap(win, c->dock, dock_xpm, 
			_("Dock Channel"), FALSE);
	gtk_box_pack_start(GTK_BOX(c->hbox), c->dock, FALSE, FALSE, 0);
	
	
	g_signal_connect(G_OBJECT(c->dock), "clicked",
			G_CALLBACK(undock), c);
	g_object_unref(G_OBJECT(tmp->frami));
	gtk_widget_show_all(win);
	
	pack_refresh(listachan, NULL, true);
}

void undock(GtkWidget *w, struct gchan *c)
{
	GtkWidget *container;
	
	container = c->frami;
	list_swap(listachan, c->windock, c->frami);
	
	g_object_ref(G_OBJECT(container));
	gtk_container_remove(GTK_CONTAINER(c->windock), container);
	
	pack_chan_insert(container);
	
	gtk_widget_destroy(c->windock);
	gtk_widget_destroy(c->dock);
	c->dock = createpixmap(window, c->dock, dock_xpm, 
			_("Undock Channel"), FALSE);
	gtk_box_pack_start(GTK_BOX(c->hbox), c->dock, FALSE, FALSE, 0);
	
	gtk_widget_show_all(c->dock);
	g_object_unref(G_OBJECT(container));
	g_signal_connect(G_OBJECT(c->dock), "clicked", 
			G_CALLBACK(dockchan), c);

}

void deletedocked(GtkWidget *w, GdkEvent *s, struct gchan *c)
{
	gtk_widget_destroy(w);
	list_remove(&listachan, c->idx);	
	mixer->delete_channel(c->idx-1);
}

