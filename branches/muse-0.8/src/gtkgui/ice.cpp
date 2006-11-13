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

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <jutils.h>
#include <jmixer.h>

#include <gen.h>
#include <listpack.h>
#include <utils.h>
#include <filedump.h>
#include <encoder.h>
#include <ice.h>

extern int lameid, oggid;

#include <config.h>

void ice_window(GtkWidget *w)
{
	GtkWidget *nbook;
	GtkWidget *menubar, *menuroot, *menuitem;
	GtkWidget *tmpbox, *tmpwid, *tmplabel;
	//	GtkWidget *scrollwin;
	struct encdata *tmpmp3, *tmpogg;

	if(winil) {
		gtk_widget_show_all(winil);
		return;
	}

	winil=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(winil), "Icecast configuration");
	gtk_window_set_policy(GTK_WINDOW(winil), TRUE, TRUE, TRUE);

	gtk_signal_connect(GTK_OBJECT(winil), "delete_event",
			(GtkSignalFunc) gtk_widget_hide_on_delete, NULL);

	tmpbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(winil), tmpbox);
	

	/* Shouter menu bar */
	menubar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(tmpbox), menubar, FALSE, FALSE, 0);

	menuroot = gtk_menu_new();
#ifdef HAVE_LAME
	menuitem = gtk_menu_item_new_with_label("Lame");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			(GtkSignalFunc) ice_new_from_menu, NULL);
	gtk_menu_append(GTK_MENU(menuroot), menuitem);
#endif

#ifdef HAVE_VORBIS
	menuitem = gtk_menu_item_new_with_label("Ogg/Vorbis");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			(GtkSignalFunc) ice_new_from_menu, NULL);
	gtk_menu_append(GTK_MENU(menuroot), menuitem);
#endif
	menuitem = gtk_menu_item_new_with_label("Add Server");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menuroot);
	gtk_menu_bar_append(GTK_MENU_BAR(menubar), menuitem);

	/* end of Shouter menu bar */
	
	nbook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(tmpbox), nbook, FALSE, FALSE, 0);

#ifdef HAVE_LAME
	/* Create Lame Streaming tab */
	tmpbox=gtk_vbox_new(FALSE, 5);
	tmplabel = gtk_label_new("Lame Streaming (MP3)");
	gtk_notebook_append_page(GTK_NOTEBOOK(nbook), tmpbox, tmplabel);
	
	tmpwid = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	tmpmp3 = enc_new("Lame");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmpmp3->verbox, TRUE, FALSE, 0);

	tmpwid = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	tmplabel = filedump_new("Lame", tmpmp3);
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(tmpwid), tmpmp3->tasti, TRUE, FALSE, 0);
	
	tmpwid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, TRUE, TRUE, 0);

	/* scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(tmpbox), scrollwin, FALSE, FALSE, 0); */
	
	createtab(&lametab);
	gtk_box_pack_start(GTK_BOX(tmpbox), lametab, FALSE, FALSE, 0);

	ice_new(MP3);
	
#endif

#ifdef HAVE_VORBIS
	/* Create Ogg/Vorbis Streaming tab */
	tmpbox=gtk_vbox_new(FALSE, 5);
	tmplabel = gtk_label_new("Ogg/Vorbis Streaming");
	gtk_notebook_append_page(GTK_NOTEBOOK(nbook), tmpbox, tmplabel);
	
	tmpwid = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	tmpogg = enc_new("Ogg/Vorbis");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmpogg->verbox, TRUE, FALSE, 0);

	tmpwid = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	tmplabel = filedump_new("Ogg/Vorbis", tmpogg);
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(tmpwid), tmpogg->tasti, TRUE, FALSE, 0);
	
	tmpwid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, TRUE, TRUE, 0);

	createtab(&oggtab);
	gtk_box_pack_start(GTK_BOX(tmpbox), oggtab, FALSE, FALSE, 0);

	ice_new(OGG);
	
#endif
	gtk_widget_show_all(winil);
#ifdef HAVE_LAME
	gtk_widget_hide_all(tmpmp3->tabbola);
#endif

#ifdef HAVE_VORBIS
	gtk_widget_hide_all(tmpogg->tabbola);
#endif
}

/* wrap your life */
void ice_new_from_menu(GtkWidget *menuitem)
{
	gchar *text;
	GtkWidget *tmpwid;
	
	tmpwid = GTK_BIN(menuitem)->child;
	gtk_label_get(GTK_LABEL(tmpwid), &text);
	
	if(!(strcmp(text, "Lame")))
		ice_new(MP3);
	else
		ice_new(OGG);

}

void ice_new(codec tipoc)
{
	GtkWidget *tmplabel;
	struct icedata *i, *tmp;
	GtkWidget *tmpbox, *tmpwid;
	GtkWidget *tmpbar;
	int id;
	unsigned int pos=0;
	unsigned int idx=1;
	//	bool ok = false;	

	/* inizialization area */
	i=(struct icedata *)g_malloc(sizeof(struct icedata));
	if(!i)
		return;
	i->combi=NULL;
	
	if(tipoc == MP3) {
	  i->outchan = mixer->get_enc(lameid);
	  id = lameid;
	  i->mp3 = true;
	  func("this is a mp3 encoder");
	} else {
	  i->outchan = mixer->get_enc(oggid);
	  id = oggid;
	  i->mp3 = false;
	  func("this is a ogg/vorbis encoder");
	}
	
	func("GTK_GUI:: ice_new: i->outchan pointer is %p", i->outchan);

	
	/* packing zone begin */
	i->frami=gtk_frame_new("");
	
	if(i->mp3) {
		while(( tmp = (struct icedata *) list_get_data(lamelist, idx, 0)))
			idx++;
		i->idx = idx;
		list_add(&lamelist, (void *) i, pos, idx, i->frami);
	} else {
		while(( tmp = (struct icedata *) list_get_data(ogglist, idx, 0)))
			idx++;
		i->idx = idx;	
		list_add(&ogglist, (void *) i, pos, idx, i->frami);
	}
	/* end of packing zone */
	
	func("GTK_GUI:: ice_new : idx for ice frame is %u", idx);

	if(idx > MAX_CHANNELS) {
		g_free(i);
		mixer->delete_enc(id);
		return;
	}
	
	/* muse-core inizialization area */
	i->iceid = i->outchan->create_ice();
	i->coreice = i->outchan->get_ice(i->iceid);
	func("GTKGUI:: created new iceid= %d and coreice= %p", i->iceid, i->coreice);
	/* end of inizialization for muse-core */

	
	tmpbox=gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(tmpbox), 10);
	gtk_container_add(GTK_CONTAINER(i->frami), tmpbox);

	tmpwid=gtk_hbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(tmpbox), tmpwid);

	tmplabel=gtk_label_new("Host");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->host=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->host, FALSE, FALSE, 0);

	tmplabel=gtk_label_new("Port");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->port=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_widget_set_usize(GTK_WIDGET(i->port), 45, 22);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->port, FALSE, FALSE, 0);

	tmplabel=gtk_label_new("Mnt");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->mnt=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_widget_set_usize(GTK_WIDGET(i->mnt), 80, 22);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->mnt, FALSE, FALSE, 0);

	tmpwid=gtk_hbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(tmpbox), tmpwid);

	tmplabel=gtk_label_new("Name");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->name=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_widget_set_usize(GTK_WIDGET(i->name), 150, 22);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->name, FALSE, FALSE, 0);
	
	tmplabel=gtk_label_new("Url");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->url=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->url, TRUE, TRUE, 0);

	tmpwid=gtk_hbox_new(FALSE,4);
	gtk_container_add(GTK_CONTAINER(tmpbox), tmpwid);
	
	tmplabel=gtk_label_new("Description");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->desc=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_widget_set_usize(GTK_WIDGET(i->desc), 300, 22);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->desc, TRUE, TRUE, 0);

	tmpwid=gtk_hbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(tmpbox), tmpwid);
		
	tmplabel=gtk_label_new("Login Type");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->logintype=gtk_combo_new();
	i->combi=g_list_append(i->combi, (void *) "x-audiocast");
	i->combi=g_list_append(i->combi, (void *) "icy");
	i->combi=g_list_append(i->combi, (void *) "http");
	gtk_combo_set_popdown_strings(GTK_COMBO(i->logintype), i->combi);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(i->logintype)->entry), FALSE);
	gtk_widget_set_usize(GTK_WIDGET(i->logintype), 100, 22);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->logintype, FALSE, FALSE, 0);
	
	tmplabel=gtk_label_new("Pass");
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0);
	i->pass=gtk_entry_new_with_max_length(MAX_OPTION_SIZE);
	gtk_entry_set_visibility(GTK_ENTRY(i->pass), FALSE);
	gtk_box_pack_start(GTK_BOX(tmpwid), i->pass, TRUE, TRUE, 0);
		
	tmpwid=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(tmpbox), tmpwid, FALSE, FALSE, 0);
	
	i->conn=gtk_toggle_button_new_with_label("Connect");
	gtk_signal_connect(GTK_OBJECT(i->conn), "clicked", 
			(GtkSignalFunc) gcb_set_icecast, i);
	/*	gtk_signal_connect(GTK_OBJECT(i->conn), "released",
		(GtkSignalFunc) gcb_stop_icecast, i); */
	gtk_box_pack_start(GTK_BOX(tmpwid), i->conn, FALSE, FALSE, 0);

	
	tmpbar=gtk_menu_bar_new();
	
	/* Add profile root menu */
	/*
	i->profroot=gtk_menu_item_new_with_label("Profile...");
	gtk_menu_bar_append(GTK_MENU_BAR(tmpbar), i->profroot);

	ice_profmenu(i);

	gtk_box_pack_start(GTK_BOX(tmpwid), tmpbar, FALSE, FALSE, 0);
	*/

	tmplabel=gtk_toggle_button_new_with_label("Delete");
	gtk_signal_connect(GTK_OBJECT(tmplabel), "clicked", 
			(GtkSignalFunc) gcb_rem_icecast, i);
	gtk_box_pack_start(GTK_BOX(tmpwid), tmplabel, FALSE, FALSE, 0); 

	/* XXX: toggle comment when profiles are ok */
	ice_put(i);
	
	if(i->mp3)  
		pos = pack_insert(lamelist, lametab, i->frami);
	else
		pos = pack_insert(ogglist, oggtab, i->frami);

	func("inserted in POS = %d", pos);
	
}

/* draw profile menu */
void ice_profmenu(struct icedata *i)
{
	char listprof[MAX_SECTION_SIZE*128], *aprof;
	GtkWidget *tmpmenu, *tmplabel;

	aprof = (char *) malloc(MAX_SECTION_SIZE);
	tmpmenu = gtk_menu_new();
	tmplabel = gtk_menu_item_new_with_label("Save Icecast...");
	gtk_signal_connect(GTK_OBJECT(tmplabel), "activate",
			(GtkSignalFunc) win_profilesave, (void *) i);
	gtk_menu_append(GTK_MENU(tmpmenu), tmplabel);

	/* Create profile list */
	i->coreice->list_profiles(listprof);

	aprof = strtok(listprof, ":");
	do {
		if((aprof[0] != '\0')) { 
			// create menu widget 
			tmplabel = gtk_menu_item_new_with_label(aprof);
			gtk_signal_connect(GTK_OBJECT(tmplabel), "activate",
					(GtkSignalFunc) gcb_ice_put, i);
			gtk_menu_append(GTK_MENU(tmpmenu), tmplabel);
			//FIXME : missing remove in profile API 
		}
	} while((aprof = strtok(NULL, ":")));

	gtk_menu_item_remove_submenu(GTK_MENU_ITEM(i->profroot));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(i->profroot), tmpmenu);
	gtk_widget_show_all(tmpmenu);

}

void ice_put(struct icedata *i) 
{
	/* fill GUI */
	char tmp[MAX_VALUE_SIZE];
	int type = 0;

	type = i->coreice->login();

	switch(type) {
		case 1: gtk_entry_set_text(GTK_ENTRY(
			GTK_COMBO(i->logintype)->entry), "x-audiocast");
			break;
		case 2: gtk_entry_set_text(GTK_ENTRY(
			GTK_COMBO(i->logintype)->entry), "icy");
			break;
		case 3: gtk_entry_set_text(GTK_ENTRY(
			GTK_COMBO(i->logintype)->entry), "http");
			break;
			
		default: func("login() gtk error :(");
	}
	
	gtk_entry_set_text(GTK_ENTRY(i->host), i->coreice->host());
	snprintf(tmp, MAX_VALUE_SIZE,"%d", i->coreice->port());
	gtk_entry_set_text(GTK_ENTRY(i->port), tmp);
	gtk_entry_set_text(GTK_ENTRY(i->mnt), i->coreice->mount());
	gtk_entry_set_text(GTK_ENTRY(i->name), i->coreice->name());
	gtk_entry_set_text(GTK_ENTRY(i->url), i->coreice->url());
	gtk_entry_set_text(GTK_ENTRY(i->desc), i->coreice->desc());
	gtk_entry_set_text(GTK_ENTRY(i->pass), i->coreice->pass());
}

void ice_fill(struct icedata *i) {
	/* fill coreice */
	gchar *tmp = NULL;

	if((tmp = gtk_editable_get_chars(GTK_EDITABLE(
						GTK_COMBO(i->logintype)->entry), 0, -1))) {
		func("ice_fill:: logintype = %s", tmp);
		if(tmp[0] == 'x')
			i->coreice->login(1);
		if(tmp[0] == 'i')
			i->coreice->login(2);
		if(tmp[0] == 'h')
			i->coreice->login(3);
	}
	
	i->coreice->host( gtk_editable_get_chars(GTK_EDITABLE(i->host), 0, -1) );
	i->coreice->port( atoi(gtk_editable_get_chars(GTK_EDITABLE(i->port), 0, -1)) );
	i->coreice->pass( gtk_editable_get_chars(GTK_EDITABLE(i->pass), 0, -1) );
	i->coreice->mount( gtk_editable_get_chars(GTK_EDITABLE(i->mnt), 0, -1) );
	i->coreice->name( gtk_editable_get_chars(GTK_EDITABLE(i->name), 0, -1) );
	i->coreice->url( gtk_editable_get_chars(GTK_EDITABLE(i->url), 0, -1) );
	i->coreice->desc( gtk_editable_get_chars(GTK_EDITABLE(i->desc), 0, -1) );
}

void gcb_set_icecast(GtkWidget *w, struct icedata *i)
{
  if((GTK_TOGGLE_BUTTON(w)->active)) {
    
    ice_fill(i);
    
    if( ! i->outchan->apply_ice(i->iceid) )
      error("wrong values in icecast configuration");

    if((i->outchan->connect_ice(i->iceid, true))) 
      gtk_window_set_title(GTK_WINDOW(winil), i->coreice->streamurl);
    else {
      gtk_window_set_title(GTK_WINDOW(winil), "Connection failed");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
    }
  } else
    i->outchan->connect_ice(i->iceid, false);
}

void gcb_rem_icecast(GtkWidget *w, struct icedata *i)
{

	if(i->mp3) {
	  if( i->outchan->delete_ice(i->iceid) ) {
	    list_remove(&lamelist, i->idx);
	    pack_refresh(lamelist, lametab, false);
	  }
	} else {
	  if( i->outchan->delete_ice(i->iceid) ) {
	    list_remove(&ogglist, i->idx);
	    pack_refresh(ogglist, oggtab, false);
	  }
	}
	
}

void gcb_ice_put(GtkWidget *w, struct icedata *i)
{
	GtkWidget *child;
	gchar *text;

	child = GTK_BIN(w)->child;
	gtk_label_get(GTK_LABEL(child), &text);

	func("i->coreice->load_profile(%s)", text);

	i->coreice->load_profile(text);	
	// QUA crasha
	//	g_free(text);
	ice_put(i);
}

void gcb_ice_save(GtkWidget *w, struct icedata *i)
{
	GtkWidget *tmplabel;
	char *profile=NULL;
	
	profile=gtk_editable_get_chars(GTK_EDITABLE(profentry), 0, MAX_SECTION_SIZE);
	if(!profile) return;
	if(profile[0] == ' ') {
		warning("gcb_ice_save:: you should insert a profile name");
		// g_free(profile);
		return;
	}

	/*fill coreice */
	ice_fill(i);
	//	i->outchan->apply_ice(i->iceid);
	
	i->coreice->write_profile(profile);
	g_free(profile);
	/* FIXME: check this */
	/* add in menu*/
	tmplabel = gtk_menu_item_new_with_label(profile);
	gtk_signal_connect(GTK_OBJECT(tmplabel), "activate",
			(GtkSignalFunc) gcb_ice_save, i);
	/* redraw menu */
	ice_profmenu(i);

	gtk_widget_destroy(gtk_widget_get_toplevel(w));
}



