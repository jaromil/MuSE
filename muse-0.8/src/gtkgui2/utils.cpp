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
#include <about.h>
#include <listpack.h>
#include <vumeters.h>
#include <utils.h>
#include <radiosched.h>

#include <jmixer.h>

#include <ice.h>

#include <xpm2/stream.xpm>
#include <xpm2/newch.xpm>
#include <xpm2/about.xpm>
#include <xpm2/mic.xpm>
#include <xpm2/speaker.xpm>
#include <xpm2/vu.xpm>

#include <config.h>

GtkWidget *createbbox(GtkWidget *bbox)
{
	GtkWidget *ice, *clame, *addch, *help, *vumeters, *scheduler;
	GtkWidget *tmpwid;

	ice = clame = addch = help = vumeters = NULL;

	bbox=gtk_hbutton_box_new();
	gtk_button_box_set_child_size(GTK_BUTTON_BOX(bbox), 7, 10);
	gtk_hbutton_box_set_spacing_default(0);
	gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_START);
	
	ice = createpixmap(window, ice, stream_xpm, _("Let's stream!"), FALSE);
	g_signal_connect(G_OBJECT(ice), "clicked", 
			G_CALLBACK(ice_window), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), ice);

	addch = createpixmap(window, addch, newch_xpm, 
			_("Add Channel"), FALSE);
	g_signal_connect_swapped(G_OBJECT(addch), "clicked",
			G_CALLBACK(createch), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), addch);
		
	/* 
	   il talk sarebbe da mettere solo come hotkey
	   
	mic = createpixmap(window, mic, stock_mic_xpm, _("Talk"), TRUE);
	g_signal_connect(G_OBJECT(mic), "clicked",
			G_CALLBACK(gcb_set_talk), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), mic);
	*/

	linein = createpixmap(window, linein, mic_xpm, _("Line In"), TRUE);
	g_signal_connect(G_OBJECT(linein), "clicked",
			G_CALLBACK(gcb_set_linein), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), linein);

	speakout = createpixmap(window, speakout, speaker_xpm, 
			_("Speaker"), TRUE);
	g_signal_connect(G_OBJECT(speakout), "clicked",
			G_CALLBACK(gcb_set_speakout), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), speakout);
	
	vumeters = createpixmap(window, vumeters, vu_xpm, 
			_("Vumeters"), FALSE);
	g_signal_connect(G_OBJECT(vumeters), "clicked",
			G_CALLBACK(vumeters_new), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), vumeters);
	
	tmpwid = gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_BUTTON);
	scheduler = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(scheduler), tmpwid);
	set_tip(scheduler, "Radio Scheduler");
	g_signal_connect(G_OBJECT(scheduler), "clicked",
			G_CALLBACK(rsched_new), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), scheduler);

	help = createpixmap(window, help, about_xpm, 
			_("Hall of Fame"), FALSE);
	g_signal_connect(G_OBJECT(help), "clicked",
			G_CALLBACK(about_win), NULL);
	gtk_container_add(GTK_CONTAINER(bbox), help);

	return bbox;

}

void createtab(GtkWidget **tab)
{
	*tab=gtk_table_new(3, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(*tab), 5);
	gtk_table_set_col_spacings(GTK_TABLE(*tab), 5);
/*	gtk_container_add(GTK_CONTAINER(vbox), *tab); */
}

void pack_new(void)
{
	GtkWidget *tmpbox;
	
	pack1.id = 1;
	pack1.hbox = gtk_hbox_new(FALSE, 0);
	pack1.adj = gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
	tmpbox = gtk_hbox_new(FALSE, 0);
	pack1.hscale = gtk_hscale_new(GTK_ADJUSTMENT(pack1.adj));
	g_signal_connect(G_OBJECT(pack1.adj), "value_changed",
			G_CALLBACK(cfade_set), (void *) &pack1);
	gtk_scale_set_digits(GTK_SCALE(pack1.hscale), 2);
	gtk_box_pack_start(GTK_BOX(tmpbox), pack1.hscale, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), pack1.hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tmpbox, FALSE, FALSE, 0);

	pack2.id = 3;
	pack2.hbox = gtk_hbox_new(FALSE, 0);
	pack2.adj = gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
	pack2.hscale = gtk_hscale_new(GTK_ADJUSTMENT(pack2.adj));
	tmpbox = gtk_hbox_new(FALSE, 0);
	g_signal_connect(G_OBJECT(pack2.adj), "value_changed",
			G_CALLBACK(cfade_set), (void *) &pack2);
	gtk_scale_set_digits(GTK_SCALE(pack2.hscale), 2);
	gtk_box_pack_start(GTK_BOX(tmpbox), pack2.hscale, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), pack2.hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tmpbox, FALSE, FALSE, 0);
	
	pack3.id = 6;
	pack3.hbox = gtk_hbox_new(FALSE, 0);
	pack3.adj = gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
	pack3.hscale = gtk_hscale_new(GTK_ADJUSTMENT(pack3.adj));
	tmpbox = gtk_hbox_new(FALSE, 0);
	g_signal_connect(G_OBJECT(pack3.adj), "value_changed",
			G_CALLBACK(cfade_set), (void *) &pack3);
	gtk_scale_set_digits(GTK_SCALE(pack3.hscale), 2);
	gtk_box_pack_start(GTK_BOX(tmpbox), pack3.hscale, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), pack3.hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), tmpbox, FALSE, FALSE, 0);
}

void cfade_set(GtkWidget *w, struct pack *p)
{
	struct gchan *tmp1, *tmp2;
	float value;
	gchar tmp[4];

	//	func(_("cfade_set get data for pos %u"), p->id);
	tmp1 = (struct gchan *) list_get_data(listachan, 0, p->id);
	tmp2 = (struct gchan *) list_get_data(listachan, 0, p->id+1);

	value = GTK_ADJUSTMENT(p->adj)->value;
	mixer->crossfade(tmp1->idx-1, 1.0-value, tmp2->idx-1, value);

	g_signal_handler_block(G_OBJECT(tmp1->adjvol), tmp1->volid);
	g_signal_handler_block(G_OBJECT(tmp2->adjvol), tmp2->volid);
	
	gtk_adjustment_set_value(GTK_ADJUSTMENT(tmp1->adjvol), value);
	snprintf(tmp, sizeof(tmp), "%d", (int) 
			((1.0-GTK_ADJUSTMENT(tmp1->adjvol)->value)*100));
	gtk_label_set_text(GTK_LABEL(tmp1->vol_lab), (gchar *) tmp);
	
	gtk_adjustment_set_value(GTK_ADJUSTMENT(tmp2->adjvol), 
			1.0-value);
	snprintf(tmp, sizeof(tmp), "%d", (int) 
			((1.0-GTK_ADJUSTMENT(tmp2->adjvol)->value)*100));
	gtk_label_set_text(GTK_LABEL(tmp2->vol_lab), (gchar *) tmp);
	
	g_signal_handler_unblock(G_OBJECT(tmp1->adjvol), tmp1->volid);
	g_signal_handler_unblock(G_OBJECT(tmp2->adjvol), tmp2->volid);

}


void putstatusbar(void)
{

	statusbar = gtk_statusbar_new();
	contextid = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar),
			_("_The_ status bar"));
	g_signal_connect(GTK_OBJECT(statusbar), "button_press_event",
			(GtkSignalFunc) status_window, NULL);
	gtk_box_pack_start(GTK_BOX(vbox),  statusbar, FALSE, FALSE, 0);
	
}

void status_window(GtkWidget *w, GdkEventButton *s)
{
	func("PROVAAAAAAAAAAAAAAAAAAAAAAAA");

}

GtkWidget *createpixmap(GtkWidget *w, GtkWidget *but, 
		gchar **pippo, gchar *tip, bool istoggled)
{
	/* create and return button with pixmap */

	GtkWidget *image;
	GtkTooltips *tooltip;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	GtkStyle *style;

	style=gtk_widget_get_style(w);
	gtk_widget_realize(w);
	pixmap=gdk_pixmap_create_from_xpm_d(w->window, &mask, 
			&style->bg[GTK_STATE_NORMAL], pippo);
	image = gtk_image_new_from_pixmap(pixmap, mask);
	
	if(!istoggled)
		but=gtk_button_new();
	else
		but=gtk_toggle_button_new();

	gtk_container_add(GTK_CONTAINER(but), image);
	tooltip = gtk_tooltips_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltip), but, tip, NULL);
	return but;
	
}

void win_error(gchar *text)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			text);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

}

void win_warning(gchar *text)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_CLOSE,
			text);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

}

void win_profile_save(GtkWidget *item, void *i)
{
	GtkWidget *winsave, *hbox, *tmpwid, *vbox1;
	gchar title[30];
	bool encoder = true;
	gchar *tmp;
	gint result;

	tmp = (gchar *) g_object_get_data(G_OBJECT(item), "type");
	
	if(tmp[0] == 'i') {
		encoder = false;
		strncpy(title, _("Save an Icecast profile"), sizeof(title));
	}
	
	else if(tmp[0] == 'l')
		strncpy(title, _("Save a Lame profile"), sizeof(title));
	else if(tmp[0] == 'o')
		strncpy(title, _("Save an Ogg/Vorbis profile"), sizeof(title));
		

	winsave = gtk_dialog_new_with_buttons(title,
			NULL, /* parent window */
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL,
			NULL);
	
	gtk_container_set_border_width(GTK_CONTAINER(winsave), 12);

	hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(winsave)->vbox), hbox);

	tmpwid = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
			GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 6);

	vbox1 = gtk_vbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 6);

	tmpwid = gtk_label_new(_("Profile Name"));
	gtk_box_pack_start(GTK_BOX(vbox1), tmpwid, FALSE, FALSE, 6);
	/* FIXME: insert a gtk combo here */
	profentry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox1), profentry, FALSE, FALSE, 6);
	
	gtk_widget_show_all(winsave);

	result = gtk_dialog_run(GTK_DIALOG(winsave));

	switch(result) {
		case GTK_RESPONSE_ACCEPT:
			if(encoder) 
				gcb_enc_save((struct encdata *) i);
			else
				gcb_ice_save((struct icedata *) i);
			break;
		case GTK_RESPONSE_CANCEL:
			break;
	}
	gtk_widget_destroy(winsave);

}

void win_profile_remove(GtkWidget *item, void *i)
{
	GtkWidget *winsave, *hbox, *tmpwid, *vbox1;
	gchar title[30];
	bool encoder = true;
	gchar *tmp;
	gint result;

	tmp = (gchar *) g_object_get_data(G_OBJECT(item), "type");
	
	if(tmp[0] == 'i') {
		encoder = false;
		strncpy(title, _("Remove an Icecast profile"), sizeof(title));
	}
	
	else if(tmp[0] == 'l')
		strncpy(title, _("Remove a Lame profile"), sizeof(title));
	else if(tmp[0] == 'o')
		strncpy(title, _("Remove a Ogg/Vorbis profile"), sizeof(title));
		

	winsave = gtk_dialog_new_with_buttons(title,
			NULL, /* parent window */
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL,
			NULL);
	
	gtk_container_set_border_width(GTK_CONTAINER(winsave), 12);

	hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(winsave)->vbox), hbox);

	tmpwid = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
			GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(hbox), tmpwid, FALSE, FALSE, 6);

	vbox1 = gtk_vbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 6);

	tmpwid = gtk_label_new(_("Profile Name"));
	gtk_box_pack_start(GTK_BOX(vbox1), tmpwid, FALSE, FALSE, 6);
	/* FIXME: insert a gtkcombo here */
	/* profentry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox1), profentry, FALSE, FALSE, 6);
	*/
	gtk_widget_show_all(winsave);

	result = gtk_dialog_run(GTK_DIALOG(winsave));

	switch(result) {
		case GTK_RESPONSE_ACCEPT:
			/*if(encoder) 
				gcb_enc_save((struct encdata *) i);
			else
				gcb_ice_save((struct icedata *) i);
			break;*/
		case GTK_RESPONSE_CANCEL:
			break;
	}
	gtk_widget_destroy(winsave);


}

void set_tip(GtkWidget *w, gchar *tip)
{
	GtkTooltips *tooltip;

	tooltip = gtk_tooltips_new();
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltip), w, tip, NULL);
}

void gcb_set_speakout(GtkWidget *w)
{
	bool res = false;
	
	if(GTK_TOGGLE_BUTTON(w)->active)
		res = mixer->set_lineout(true);
	else
		mixer->set_lineout(false);

	if(!res)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
}

void gcb_set_linein(GtkWidget *w)
{
	bool res = false;

	if(GTK_TOGGLE_BUTTON(w)->active) 
		res = mixer->set_live(true);
	else 
		mixer->set_live(false);

	if(!res) 
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
}

void gcb_set_talk(GtkWidget *w)
{
	unsigned int i;
	float voltmp[MAX_CHANNELS];
	struct gchan *c = NULL;

	if(GTK_TOGGLE_BUTTON(w)->active) {
		for(i = 0; i < MAX_CHANNELS; i++) {
			c = (struct gchan *) list_get_data(listachan, i+1, 0);
			if(c) {
				storevol[i]= GTK_ADJUSTMENT(c->adjvol)->value;
				if(storevol[i] < TALK_VOLUME)
					voltmp[i]=0.0;
				else
					voltmp[i]= TALK_VOLUME;
				gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjvol), voltmp[i]);
			}
		}
		mixer->set_all_volumes(&voltmp[0]);
		
			
	}
	else {
		for(i = 0; i < MAX_CHANNELS; i++) {
			c = (struct gchan *) list_get_data(listachan, i+1, 0);
			if(c)
				gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjvol), 
						storevol[i]);
		}
		mixer->set_all_volumes(&storevol[0]);
	}
	
}
	
void gcb_exit(GtkWidget *w, GdkEvent *s)
{
	state=false;
}

