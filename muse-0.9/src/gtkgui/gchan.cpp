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
#include <string.h>
#include <gtk/gtk.h>
#include <jutils.h>
#include <gen.h>
#include <utils.h>
#include <docking.h>
#include <gchan_events.h>
#include <gchan.h>
#include <listpack.h>

#include <jmixer.h>

#include <tX_dial.h>

#include <xpm/play.xpm>
#include <xpm/stop.xpm>
#include <xpm/pause.xpm>
#include <xpm/rewind.xpm>
#include <xpm/forward.xpm>
#include <xpm/delch.xpm>
#include <xpm/dock.xpm>

#include <config.h>

static GtkWidget *winh;

bool createch(void)
{
	struct gchan *object;
	GtkWidget *fixed;
	GtkWidget *frami1;
	GtkWidget *vbox, *htast, *hbot;
	GtkObject *adj, *adj1;
	GtkWidget *playmenu, *playmenuopt;
	GtkWidget *tmpwid, *tmpwid1, *dock;
	GtkWidget *progress; 
	GtkWidget *volume; 
	GtkWidget *scrollwin, *lista;
	GtkWidget *table, *rmit/*, *file, *http*/;
	GtkWidget *piddi;
	gchar numchan[16];
	unsigned int pos=0;
	unsigned int idx=1;	
	dock=rmit=NULL;


	while(( object = (struct gchan *) list_get_data(listachan, idx, 0) ))
		idx++;
	
	if(idx > MAX_CHANNELS)
		return FALSE;
	
	func("GTK_GUI::createch(%u)", idx);
	object=(struct gchan *)g_malloc(sizeof(struct gchan));
	
	object->idx=idx;	
	object->channel=0; /* nothing */
	
	func("GTK_GUI::createch : chan[%u] is at %p", idx, object);

	mixer->create_channel(idx-1);

	/*frami=gtk_fixed_new();
	object->frami=frami;*/
	snprintf(numchan,16,"Channel[%u]",object->idx);
	frami1=gtk_frame_new(numchan);
	object->frami = frami1;
	gtk_frame_set_shadow_type(GTK_FRAME(frami1), GTK_SHADOW_ETCHED_OUT);
	/*gtk_fixed_put(GTK_FIXED(frami), frami1, 0, 0);*/
	
	vbox=gtk_vbox_new(FALSE,0); 
	gtk_container_add(GTK_CONTAINER(frami1), vbox);
	
	/* at the moment pos is 0 function */
	list_add(&listachan, (void *) object, pos, idx, object->frami);
	

	htast = gtk_hbox_new(TRUE,0); /* same dimension */
	fixed = gtk_fixed_new();
	gtk_fixed_put(GTK_FIXED(fixed), htast, 0, 0);
	//gtk_container_add(GTK_CONTAINER(vbox), fixed);
	gtk_box_pack_start(GTK_BOX(vbox), fixed, FALSE, FALSE, 0);

	/* addiamo il coso del tempo */
	tmpwid=gtk_entry_new_with_max_length(7);
	object->ptime=tmpwid;
	gtk_widget_set_usize(GTK_WIDGET(tmpwid), 40, 22);
	gtk_entry_set_text(GTK_ENTRY(tmpwid), "00:00");
	gtk_entry_set_editable(GTK_ENTRY(tmpwid), FALSE);
	gtk_box_pack_start(GTK_BOX(htast), tmpwid, FALSE, FALSE, 0);
	
	/* enjoy ourselves with buttons */
	
	tmpwid=createpixmap(window, tmpwid, play_xpm, "Play Channel", TRUE);
	object->play=tmpwid;
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc)gcb_play_channel, object);
	gtk_box_pack_start(GTK_BOX(htast),tmpwid,FALSE,TRUE,0);
	
	tmpwid=createpixmap(window, tmpwid, stop_xpm, "Stop Channel", FALSE);
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc)gcb_stop_channel, object);
	gtk_box_pack_start(GTK_BOX(htast),tmpwid,FALSE,TRUE,0);
	
	tmpwid=createpixmap(window, tmpwid, pause_xpm, "Pause Channel", TRUE);
	object->pause=tmpwid;
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc)gcb_pause_channel, object);
	gtk_box_pack_start(GTK_BOX(htast),tmpwid ,FALSE,TRUE,0);

	tmpwid=createpixmap(window, tmpwid, rewind_xpm, "Rewind Channel", FALSE);
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc)gcb_begin_channel, object);
	gtk_box_pack_start(GTK_BOX(htast),tmpwid,FALSE,TRUE,0);
	
	tmpwid=createpixmap(window, tmpwid, forward_xpm, 
			"Forward Channel", FALSE);
	gtk_signal_connect(GTK_OBJECT(tmpwid), "clicked",
			(GtkSignalFunc)gcb_end_channel, object);
	gtk_box_pack_start(GTK_BOX(htast),tmpwid ,FALSE,TRUE,0);
	
	/* second part */
	adj=gtk_adjustment_new(0.0, 0.0, 1.0, 0.1, 0.1, 0.0);
	object->adjprog=adj;
	progress=gtk_hscale_new(GTK_ADJUSTMENT(adj));
	gtk_scale_set_digits(GTK_SCALE(progress), 6);
	object->progress=progress;

	gtk_scale_set_draw_value(GTK_SCALE(progress),FALSE);
	gtk_signal_connect(GTK_OBJECT(progress), "button_press_event",
			   (GtkSignalFunc) gcb_event_pause_channel, object);
	gtk_signal_connect(GTK_OBJECT(progress), "button_release_event",
			(GtkSignalFunc) gcb_event_set_position, object);
	//gtk_container_add(GTK_CONTAINER(vbox),progress);
	gtk_box_pack_start(GTK_BOX(vbox), progress, FALSE, FALSE, 0);

	hbot=gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(vbox),hbot);
	
	/* volume adjustment  */
	adj1=gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
	object->adjvol=adj1;
	object->volid = gtk_signal_connect(GTK_OBJECT(adj1), "value_changed",
			(GtkSignalFunc) gcb_set_volume, object);

#ifdef WITH_SPEED
	/* speed adjustment */
	object->adjspeed=gtk_adjustment_new(1.0, 0.0, 1.0, 0.1, 0.1, 0.0);
	gtk_signal_connect(GTK_OBJECT(object->adjspeed), "value_changed",
			(GtkSignalFunc) gcb_set_speed, object);
#endif
	
	volume=gtk_vscale_new(GTK_ADJUSTMENT(adj1));
	/*gtk_scale_set_digits(GTK_SCALE(volume), 2);*/
	gtk_scale_set_draw_value(GTK_SCALE(volume),FALSE);
	/* gtk_scale_set_value_pos(GTK_SCALE(volume), GTK_POS_TOP); commented */

	tmpwid=gtk_table_new(2, 1, FALSE);
	gtk_box_pack_start(GTK_BOX(hbot),tmpwid,FALSE,FALSE,0);
	tmpwid1=gtk_vbox_new(FALSE, 0);
	object->vol_lab=gtk_label_new("VOL");
	gtk_box_pack_start(GTK_BOX(tmpwid1), object->vol_lab, FALSE, FALSE, 0);
	/*volume=gtk_tx_dial_new(GTK_ADJUSTMENT(adj1));
	set_tip(volume, "Volume");*/
	gtk_box_pack_start(GTK_BOX(tmpwid1), volume, TRUE, TRUE, 0);
	object->vol_lab=gtk_label_new("100");
	gtk_box_pack_start(GTK_BOX(tmpwid1), object->vol_lab, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmpwid), tmpwid1, 0, 1, 1, 2);

#ifdef WITH_SPEED
	/* speed widget */
	//	tmpwid1=gtk_vbox_new(FALSE, 0);
	object->speed=gtk_tx_dial_new(GTK_ADJUSTMENT(object->adjspeed));
	set_tip(object->speed, "Speed");
	gtk_box_pack_start(GTK_BOX(tmpwid1), object->speed, FALSE, FALSE, 0);
	object->speed_lab=gtk_label_new("100");
	gtk_box_pack_start(GTK_BOX(tmpwid1), object->speed_lab, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmpwid), tmpwid1, 0, 1, 0, 1);
#endif

	scrollwin=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_usize(GTK_WIDGET(scrollwin), 90, 140);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
			GTK_POLICY_AUTOMATIC,GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(hbot),scrollwin,TRUE,TRUE,0);

	lista=gtk_clist_new(1);
	gtk_clist_set_column_width(GTK_CLIST(lista), 0, 90);
	object->lista=lista;
	gtk_clist_set_shadow_type(GTK_CLIST(lista),GTK_SHADOW_IN);

	blockid[idx]=gtk_signal_connect(GTK_OBJECT(object->lista), 
			"select_row", (GtkSignalFunc)gcb_set_channel, object);
	gtk_signal_connect(GTK_OBJECT(object->lista), "button_press_event",
			(GtkSignalFunc)gcb_event_clist_popup, object);
	gtk_container_add(GTK_CONTAINER(scrollwin),lista);
	
	/* playmode menu */
	playmenu=gtk_menu_new();
	tmpwid=gtk_menu_item_new_with_label("single play");
	gtk_object_set_data(GTK_OBJECT(tmpwid), "label", (void *)"0");
	gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
			(GtkSignalFunc) gcb_set_playmode, object);
	gtk_menu_append(GTK_MENU(playmenu), tmpwid);
	
	tmpwid=gtk_menu_item_new_with_label("loop");
	gtk_object_set_data(GTK_OBJECT(tmpwid), "label", (void *)"1");
	gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
			(GtkSignalFunc) gcb_set_playmode, object);
	gtk_menu_append(GTK_MENU(playmenu), tmpwid);
	
	tmpwid=gtk_menu_item_new_with_label("continuous");
	gtk_object_set_data(GTK_OBJECT(tmpwid), "label", (void *)"2");
	gtk_signal_connect(GTK_OBJECT(tmpwid), "activate",
			(GtkSignalFunc) gcb_set_playmode, object);
	gtk_menu_append(GTK_MENU(playmenu), tmpwid);

	playmenuopt=gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(playmenuopt), playmenu);
	
	rmit=createpixmap(window, rmit, delch_xpm, 
			"Close Channel", FALSE);
	object->rmit=rmit;

	gtk_signal_connect(GTK_OBJECT(object->rmit),"clicked",
			(GtkSignalFunc)gcb_deletech,object);

	dock=createpixmap(window, dock, dock_xpm, 
			"Undock Channel", FALSE);
	object->dock=dock;
	gtk_signal_connect(GTK_OBJECT(object->dock), "clicked",
			(GtkSignalFunc)dockchan, object);

	piddi=gtk_hbox_new(FALSE, 0);
	object->hbox=piddi;
	gtk_box_pack_start(GTK_BOX(piddi), rmit, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(piddi), dock, FALSE, FALSE, 0);

	table=gtk_table_new(1, 4, TRUE);
	object->table=table;
	gtk_table_attach_defaults(GTK_TABLE(table), piddi, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), playmenuopt, 3, 5, 0, 1);

	/*fixed = gtk_fixed_new();
	gtk_fixed_put(GTK_FIXED(fixed), table, 0, 0);
	gtk_container_add(GTK_CONTAINER(vbox), fixed);*/
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);

	object->playmode=0;

	/* pack_chan_insert and pos update */
	pos = pack_chan_insert(object->frami);


	return TRUE;
}


void spawnfilew(GtkWidget *button, struct gchan *o)
{
	GtkWidget *filew;

	func("GTK_GUI::spawnfilew : filebrowser for chan[%u]", o->idx);

	filew=gtk_file_selection_new("Select a .mp3 file or a playlist (.pl)");
	if(pathfile)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filew), pathfile);
	gtk_object_set_data(GTK_OBJECT(GTK_FILE_SELECTION(filew)->ok_button),
			"chan", (void *) &o->idx);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filew)->ok_button),
			"clicked",(GtkSignalFunc) gcb_add_file, filew);
	
	gtk_signal_connect(GTK_OBJECT(GTK_WIDGET(filew)), "destroy",
			(GtkSignalFunc) gtk_widget_destroy, NULL);
	gtk_signal_connect_object(GTK_OBJECT(
				GTK_FILE_SELECTION(filew)->cancel_button),
			"clicked", (GtkSignalFunc)gtk_widget_destroy, GTK_OBJECT(filew));
	gtk_widget_show(filew);


}

/* FIXME: gchan pointer name */
void httpwin(GtkWidget *w, struct gchan *o)
{
	GtkWidget *table, *tmp;

	if(winh)
		return;

	winh=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	o->httpwind=winh;
	gtk_window_set_title(GTK_WINDOW(winh), "Listen to a network stream");
	gtk_signal_connect(GTK_OBJECT(winh), "destroy",
		(GtkSignalFunc) gtk_widget_destroyed, &winh);
	gtk_container_set_border_width(GTK_CONTAINER(winh), 7);
	
	table=gtk_table_new(3, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 3);
	gtk_container_add(GTK_CONTAINER(winh), table);
	
	tmp=gtk_label_new("Insert http source: ");
	gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, 0, 1);

	tmp=gtk_entry_new();
	o->httpentry=tmp;
	gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, 1, 2);
	
	tmp=gtk_button_new_with_label(" Ok ");
	gtk_signal_connect(GTK_OBJECT(tmp), "clicked",
			(GtkSignalFunc) httpinsert, o);
	gtk_table_attach_defaults(GTK_TABLE(table), tmp, 1, 2, 2, 3);

	gtk_widget_show_all(winh);
}

void httpinsert(GtkWidget *w, struct gchan *o)
{
	gchar *s=NULL;

	s=gtk_editable_get_chars(GTK_EDITABLE(o->httpentry), 0, -1);
	if(s[0] == 0) {
		g_free(s);
		gtk_widget_destroy(o->httpwind);
		winh = NULL;
		return;
	}
	act("httpinsert() url is %s", s);
	if(!(mixer->add_to_playlist(o->idx-1, s))) 
		error("GTK_GUI:: httpinsert problem adding url");
	g_free(s);
	gtk_widget_destroy(o->httpwind);
	winh = NULL;
}

void gcb_deletech(GtkWidget *w, struct gchan *c)
{
	mixer->delete_channel(c->idx-1);
	switch(c->pos) {
		case 2: gtk_widget_hide(pack1.hscale);
				break;
		case 4: gtk_widget_hide(pack2.hscale);
				break;
		case 6: gtk_widget_hide(pack3.hscale);
	}

	list_remove(&listachan, c->idx);
	pack_refresh(listachan, NULL, true);
	
}

/* FIXME: backward compatibility */
struct gchan *gcb_findch(unsigned int idx,unsigned int pos)
{
	struct listitem *tmp;
	
	if(pos == NOPOS)
		tmp = list_find_byidx(listachan, idx);
	else
		tmp = list_find_bypos(listachan, pos);

	if(!tmp)
		return NULL;
	else
		return (struct gchan *) tmp->data;
}

void gcb_set_channel(GtkWidget *w, gint row, gint column, 
		GdkEventButton *e, struct gchan *o)
{

	unsigned int res;
	
	res=mixer->set_channel(o->idx-1, row+1); 

	if(!res)
	  func("GTK_GUI::gcb_set_channel : Stream_mixer::set_channel(%d,%d) returned %u", o->idx-1, row+1, res);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->play), FALSE);

	switch(res) {
	case 0:
	  func("GTK_GUI::gcb_set_channel : can't open bitstream");
	  o->channel=0;
	  break;
	case 1: /* channel is seekable */
	  gtk_entry_set_text(GTK_ENTRY(o->ptime), "00:00");
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(o->adjprog), 0.0);
	  o->channel=1;
	  break;
	case 2: /* unseekable */
	  gtk_entry_set_text(GTK_ENTRY(o->ptime), "-----");
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(o->adjprog), 0.0);
	  o->channel=2;
	  break;
	}

}

void gcb_set_playmode(GtkWidget *w, struct gchan *o) 
{
	gchar *text;

	text= (gchar *) gtk_object_get_data(GTK_OBJECT(w), "label");
	int num = atoi(text);
	//	func("GTK_GUI::gcb_set_playmode sets to %i",num);
	mixer->set_playmode(o->idx-1, num);
	o->playmode=atoi(text);
}

void gcb_play_channel(GtkWidget *w, struct gchan *o)
{
  bool pro=false;
  
  if(GTK_TOGGLE_BUTTON(w)->active) {
    pro=mixer->play_channel(o->idx-1);
    if(!pro)// notice("ok for playing");
      //    else {
      {
	notice("KO for playing :(");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->play), FALSE);
      }
  }
  /*else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->pause), FALSE);
    mixer->stop_channel(o->idx-1);
    } */

}

void gcb_stop_channel(GtkWidget *w, struct gchan *o)
{
	mixer->stop_channel(o->idx-1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->pause), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->play), FALSE);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(o->adjprog), 0.0);
	switch(o->channel) {
		case 1:
		gtk_entry_set_text(GTK_ENTRY(o->ptime), "00:00");
		break;
		case 2:
		gtk_entry_set_text(GTK_ENTRY(o->ptime), "stream");
	}

}

void gcb_pause_channel(GtkWidget *w, struct gchan *o)
{
	mixer->pause_channel(o->idx-1); 
}

void gcb_begin_channel(GtkWidget *w, struct gchan *o)
{
	mixer->set_position(o->idx-1, 0.0);
}

void gcb_end_channel(GtkWidget *w, struct gchan *o)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->pause), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(o->play), FALSE);
	/*mixer->set_position(o->idx-1, 1.0); */

}

void gcb_set_volume(GtkAdjustment *adj, struct gchan *o)
{
	char tmp[4];

	mixer->set_volume(o->idx-1, (1.0-adj->value));
	snprintf(tmp, sizeof(tmp), "%d", (int) ((1.0-adj->value)*100));
	gtk_label_set_text(GTK_LABEL(o->vol_lab), (gchar *) tmp);
}

#ifdef WITH_SPEED
void gcb_set_speed(GtkAdjustment *adj, struct gchan *o)
{
	char tmp[4];

	/* mixer->set_speed(o->idx-1, adj->value); */
	snprintf(tmp, sizeof(tmp), "%d", (int) (adj->value*100));
	gtk_label_set_text(GTK_LABEL(o->speed_lab), (gchar *) tmp);
}
#endif


void gcb_rem_from_playlist(GtkWidget *w, struct gchan *o)
{
	
	func("gcb_rem_from_playlist");
	
	if(o->rem != -1) {
		mixer->rem_from_playlist(o->idx-1, o->rem+1);
		gtk_clist_remove(GTK_CLIST(o->lista), o->rem);
	}
}

void gcb_add_file(GtkWidget *w, GtkFileSelection *fw)
{
	unsigned int idx=0;
	gchar *cist[1]; /* e la cista ale' la la la */
	bool res=false;

	cist[0]=NULL;
	
	idx= *(unsigned int *) gtk_object_get_data(GTK_OBJECT(GTK_FILE_SELECTION(fw)->ok_button), "chan");
	func("GTK_GUI::gcb_add_file : idx %u", idx);
	cist[0]=gtk_file_selection_get_filename(GTK_FILE_SELECTION (fw));
	
	if(!cist[0]) return;

	res=mixer->add_to_playlist(idx-1, cist[0]);
	if(!res) func("gcb_add_file:: mixer->add_to_playlist(%u, %s) failed", idx-1, cist[0]);
	else {
	  /* saves last directory visited */
	  if(pathfile) g_free(pathfile);
	  pathfile=strdup(cist[0]);
	  int i=strlen(pathfile);
	  while(pathfile[i] != '/') i--;
	  pathfile[i+1]= '\0';
	}
	gtk_widget_destroy((GtkWidget *)fw);
	
}

