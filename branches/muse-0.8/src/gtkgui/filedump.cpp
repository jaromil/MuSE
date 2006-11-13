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
#include <jmixer.h>

#include <gen.h>
#include <encoder.h>
#include <filedump.h>

#include <config.h>

GtkWidget *filedump_new(char *text, struct encdata *enc)
{
	GtkWidget *dumpbox;
	GtkWidget *hbox, *tmpwid, *entry;
	
	/*dumpwin = gtk_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(dumpwin), TRUE);
	gtk_window_set_title(GTK_WINDOW(dumpwin), "Record Stream");
	gtk_container_set_border_width(GTK_CONTAINER(dumpwin), 7);
	gtk_signal_connect(GTK_OBJECT(dumpwin), "destroy",
			(GtkSignalFunc) gtk_widget_destroy, NULL);*/

	dumpbox = gtk_vbox_new(FALSE, 4);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dumpbox), hbox, FALSE, FALSE, 0);
	
	if(!(strcmp(text, "Lame"))) 
		tmpwid = gtk_label_new("Record Lame Stream");
	else
		tmpwid = gtk_label_new("Record Ogg/Vorbis Stream");
	
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dumpbox), hbox, FALSE, FALSE, 0);

	tmpwid = gtk_label_new("Enter filename");
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 0);
	
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dumpbox), hbox, FALSE, FALSE, 0);
	
	tmpwid=gtk_button_new_with_label("Browse...");
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc) filedump_sel_file, entry);
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 0);
	
	tmpwid = gtk_toggle_button_new_with_label("Record now!");
	/*if(mixer->fileout)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmpwid), TRUE);*/
	gtk_object_set_data(GTK_OBJECT(entry), "enc", (void *) enc);
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc) gcb_set_filedump, entry);
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 0);

	gtk_widget_show_all(dumpbox);
	return dumpbox;
}

void filedump_sel_file(GtkWidget *w, GtkWidget *entry)
{
	GtkWidget *filew;

	filew=gtk_file_selection_new("Create your mp3 file into a dir");
	gtk_window_set_modal(GTK_WINDOW(filew), TRUE);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filew)->ok_button),
			"clicked", (GtkSignalFunc) filedump_set_entry, entry);
	gtk_signal_connect_object(GTK_OBJECT(
				GTK_FILE_SELECTION(filew)->cancel_button),
			"clicked", (GtkSignalFunc) gtk_widget_destroy, GTK_OBJECT(filew));
	gtk_signal_connect(GTK_OBJECT(filew), "destroy", 
			(GtkSignalFunc) gtk_widget_destroy, NULL);

	gtk_widget_show(filew);

}

void filedump_set_entry(GtkWidget *w, GtkWidget *entry)
{	
	char *pd;

	pd = gtk_file_selection_get_filename(
			GTK_FILE_SELECTION(gtk_widget_get_toplevel(w)));
	gtk_entry_set_text(GTK_ENTRY(entry), pd);
	gtk_widget_destroy(gtk_widget_get_toplevel(w));
	
}

void gcb_set_filedump(GtkWidget *w, GtkWidget *entry)
{
	char *file;
	struct encdata *enc;

	enc = (struct encdata *) gtk_object_get_data(GTK_OBJECT(entry), "enc");
	
	/* encoder is configured yet */

	if(GTK_TOGGLE_BUTTON(w)->active) {
		file = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
		notice("enc->outchan->dump_start(%s)", file);	

		if((strlen(file)))
			enc->outchan->dump_start(file);
		else
			error("gcb_set_filedump:: Not a valid filename");
		g_free(file);
	}
	else
		enc->outchan->dump_stop(); 
}
