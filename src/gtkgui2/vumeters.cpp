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

#include <gtkdial.h>
#include <vumeters.h>
#include <gen.h>

#include <jmixer.h>

#include <config.h>

struct vumeters *vu = NULL;
GtkWidget *winvu = NULL;

void vumeters_new(void)
{
	GtkWidget *hbox, *tmpbox; /* one hbox, two vbox */
	GtkWidget *tmpwid;

	if(!vu)
	  vu = (struct vumeters *) malloc(sizeof(struct vumeters));

	if(!winvu) {
		
	  winvu = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	  gtk_window_set_title(GTK_WINDOW(winvu), _("MuSE - Vumeters"));
	  gtk_container_set_border_width(GTK_CONTAINER(winvu), 5);
	  g_signal_connect(G_OBJECT(winvu), "destroy",
			   G_CALLBACK(vumeters_close), winvu);
	  
	  hbox = gtk_hbox_new(FALSE, 6);
	  gtk_container_add(GTK_CONTAINER(winvu), hbox);
	  
	  /* volume - adj - widget - label - packing */
	  
	  tmpbox = gtk_vbox_new(FALSE, 0);
	  //vu->vu_adjvol = gtk_adjustment_new(0.0, -32767.0, 32767.0, 1.0, 1.0, 0.0);
	  vu->vu_adjvol = gtk_adjustment_new(0.0, 0.0, 32767.0, 1.0, 1.0, 0.0);
	  tmpwid = gtk_dial_new(GTK_ADJUSTMENT(vu->vu_adjvol));
	  vu->vu_labvol = gtk_label_new(_("volume"));
	  gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(tmpbox), vu->vu_labvol, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hbox), tmpbox, FALSE, FALSE, 5);
	  /* bandwith - the same */
	  
	  tmpbox = gtk_vbox_new(FALSE, 0);
	  /* 6144 = 2048*3 */
	  vu->vu_adjband = gtk_adjustment_new(0.0, 0.0, 61440, 1.0, 1.0, 0.0);
	  tmpwid = gtk_dial_new(GTK_ADJUSTMENT(vu->vu_adjband));
	  vu->vu_labband = gtk_label_new(_("0 byte/s"));
	  gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(tmpbox), vu->vu_labband, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hbox), tmpbox, FALSE, FALSE, 5);

	}
	vu_status=true;
	gtk_widget_show_all(winvu);
	
}

void vumeters_close(GtkWidget *w)
{
	vu_status = false;
	func("DESTROY");
	gtk_widget_destroy(winvu);
	func("DESTROY DONE");
	//gtk_widget_hide(winvu);
	winvu = NULL;
	//	g_free(vu);
	free(vu);
	vu = NULL;
	//	vu = NULL;
}

