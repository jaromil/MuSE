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


/*
 * O muse, o alto ingegno, or m'aiutate;
 * o mente che scrivesti ciò ch'io vidi,
 * qui si parrà la tua nobilitate.
 *                       (Dante, Inferno, II vv. 7-9)
 */

#include <stdlib.h>
#include <gtk/gtk.h>

#include <gen.h>
#include <gchan.h>
#include <utils.h>
#include <listpack.h>
#include <musegtk.h>
#include <tX_knobloader.h>

#include <jutils.h>
#include <jmixer.h>

#include <config.h>

GtkWidget *window, *statusbar, *mic, *linein, *speakout;
struct pack pack1, pack2, pack3;
guint contextid;
GtkWidget *vbox, *fix; // *maintab;
GtkWidget *lametab, *oggtab;
GtkWidget *winil, *profentry;
Stream_mixer *mixer;
GList *listachan, *lamelist, *ogglist;
int lameid, oggid;
float storevol[MAX_CHANNELS];
char *pathfile;
/* id of "select_row" handler */
guint blockid[7];
bool state;
bool vu_status=false;

bool gtkgui_init(int argc,char *argv[], Stream_mixer *mix)
{

	GtkWidget *bbox=NULL;
	bool isx=false;
	int i;
	
	/* initialization */
	state=true;
	mixer=mix;
	for(i=0; i<7; i++) {
/*		channum[i]=0;*/
		blockid[i]=0;
	}
	
	/*maxpos=0;*/
	list_init(&listachan);
	list_init(&lamelist);
	list_init(&ogglist);
	pathfile=NULL;
	
	/* signal to glib we're going to use threads */
	g_thread_init(NULL);

	isx=gtk_init_check(&argc,&argv);
	if(!isx) return false;	

	isx=mixer->set_lineout(true);

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"MuSE - Multiple Streaming Engine");
	gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(window),7);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",
					(GtkSignalFunc)gcb_exit,NULL);

	gtk_widget_realize(window);
//	load_knob_pixs(window);
	
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	

	fix=gtk_fixed_new();
	//gtk_container_add(GTK_CONTAINER(vbox), fix);
	gtk_box_pack_start(GTK_BOX(vbox), fix, FALSE, FALSE, 0);
	
	bbox=createbbox(bbox);
	gtk_fixed_put(GTK_FIXED(fix), bbox, 0, 0);
	/*gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);*/ 
	if(isx)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(speakout), TRUE);
	
	
	pack_new();		
	
	/*firstchanfree(channum);*/
	createch();
	putstatusbar();
	
	/*let's show window */
	gtk_widget_show_all(window);
	gtk_widget_hide(pack1.hscale);
	gtk_widget_hide(pack2.hscale);
	gtk_widget_hide(pack3.hscale);

	return true;	
}

bool
gtkgui_refresh(void)
{
  //  func("REFRESH GUI");
  gdk_threads_enter();

  /* Rest in gtk_main and wait for fun to begin ;)  */
  while(gtk_events_pending())
    gtk_main_iteration();
  
  gdk_threads_leave();
  //  func("GUI REFRESHED");
  return state;
}

bool gtkgui_get_state(void)
{
	return state;
}

bool gtkgui_set_lcd(unsigned int chan, char *testo)
{
	struct gchan *c;
	
	c=(struct gchan *)list_get_data(listachan, chan+1, 0); 
	/* c=gcb_findch(chan+1, NOPOS);*/
	if(c) {
	  //	  gdk_threads_enter();
	  gtk_entry_set_text(GTK_ENTRY(c->ptime), (gchar *) testo);
	  //	  gdk_threads_leave();
	  return true;
	}
	return false;
}

bool gtkgui_set_pos(unsigned int chan, float position)
{
	struct gchan *c;

	c=(struct gchan *)list_get_data(listachan, chan+1, 0);
	/* c=gcb_findch(chan+1, NOPOS); */
	
	if(c) {
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjprog), position );
	  return true;
	}
	return false;
}

bool gtkgui_add_into_pl(unsigned int chan, char *file)
{
	struct gchan *c;

	char *cist[1];
	c=(struct gchan *)list_get_data(listachan, chan+1, 0); 
	/* o=gcb_findch(chan+1, NOPOS); */

	if(!c) return false;
	cist[0] = file;

	int riga = gtk_clist_append(GTK_CLIST(c->lista), cist);
	if(!riga) gtk_clist_select_row(GTK_CLIST(c->lista), 0, 0);

	return true;
}

void gtkgui_set_maintitle(char *testo) 
{
  //  gdk_threads_enter();

  gtk_window_set_title(GTK_WINDOW(window), (gchar *) testo);

  //  gdk_threads_leave();
}

void gtkgui_set_statustext(char *testo) 
{
  //  gdk_threads_enter();

  contextid=gtk_statusbar_push(GTK_STATUSBAR(statusbar), 
			       contextid, (gchar *) testo);
  
  //  gdk_threads_leave();
}

bool gtkgui_sel_playlist(unsigned int chan, int row)
{
	struct gchan *c;

	c=(struct gchan *)list_get_data(listachan, chan+1, 0); 
	/* o=gcb_findch(chan+1, NOPOS); */

	if(!c) return false;

	//	gdk_threads_enter();

	gtk_signal_handler_block(GTK_OBJECT(c->lista), blockid[chan+1]);
	gtk_clist_select_row(GTK_CLIST(c->lista), row-1, 0);
	gtk_signal_handler_unblock(GTK_OBJECT(c->lista), blockid[chan+1]);
	gtk_entry_set_text(GTK_ENTRY(c->ptime), "00:00");
	gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjprog), 0.0);

	if(!c->playmode)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->play), FALSE);
	//	gdk_threads_leave();
	return true;
}

void gtkgui_toggle_mic(bool i) 
{
  //  gdk_threads_enter();

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mic), i);

  //  gdk_threads_leave();
}

void gtkgui_toggle_spk(bool i)
{
  //  gdk_threads_enter();

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(linein), i);
  
  //  gdk_threads_leave();
}

void gtkgui_toggle_lineout(bool i) 
{
  //  gdk_threads_enter();

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(speakout), i);

  //  gdk_threads_leave();
}

void gtkgui_set_vuvolume(int value) 
{
	gtk_adjustment_set_value(GTK_ADJUSTMENT(vu->vu_adjvol), (float) value);
}

void gtkgui_set_vuband(int value)
{
	char tmp[32];

	gtk_adjustment_set_value(GTK_ADJUSTMENT(vu->vu_adjband), (float) value);
	snprintf(tmp, 32, "%d byte/s", value/ 8);
	gtk_label_set_text(GTK_LABEL(vu->vu_labband), tmp);

}

void gtkgui_exit(void) 
{
  //  gdk_threads_enter();
  gtk_widget_destroy(window);
  g_list_free(listachan);
  
  //  gdk_threads_leave();

  
}
