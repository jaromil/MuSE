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
#include <xmlprofile.h>
#include <musegtk.h>

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
GList *iceprof, *lameprof, *vorbisprof;
int lameid, oggid;
float storevol[MAX_CHANNELS];
char *pathfile;
bool state;
bool vu_status = false;
int dndch=0;
gboolean dndlock=FALSE;

bool gtkgui_init(int argc, char *argv[], Stream_mixer *mix)
{

	GtkWidget *bbox = NULL;
	bool isx = false;
	/* FIXME: bisogan mettere l'enable_nls*/
	/* i18n */
	setlocale(LC_ALL, "");
	bindtextdomain("muse", LOCALEDIR);
	bind_textdomain_codeset("muse", "UTF-8");
	textdomain("muse");
	
	/* initialization */
	state = true;
	mixer = mix;
	
	list_init(&listachan);
	list_init(&lamelist);
	list_init(&ogglist);
	iceprof = lameprof = vorbisprof = NULL;
	
	if(!profile_init())
		error(_("profile initialization error"));
	profile_ice_load();
	/* profile_lame_load and profile_vorbis_load are into encoder.cpp */

	pathfile = NULL;
	
	/* signal to glib we're going to use threads */
	g_thread_init(NULL);

	isx = gtk_init_check(&argc,&argv);
	if(!isx) 
		return false;	

	isx = mixer->set_lineout(true);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("MuSE-cvs Gtk+2"));
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 12);
	g_signal_connect(G_OBJECT(window), "delete_event",
					G_CALLBACK(gcb_exit), NULL);

	/* FIXME: gtk2 remove ? */
	gtk_widget_realize(window);
	
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	

	fix = gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(vbox), fix, FALSE, FALSE, 0);
	
	bbox = createbbox(bbox);
	gtk_fixed_put(GTK_FIXED(fix), bbox, 0, 0);
	if(isx)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(speakout), TRUE);
	
	
	pack_new();		
	
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
	if(c) {
	  gtk_entry_set_text(GTK_ENTRY(c->ptime), (gchar *) testo);
	  return true;
	}
	return false;
}

bool gtkgui_set_pos(unsigned int chan, float position)
{
	struct gchan *c;

	c=(struct gchan *)list_get_data(listachan, chan+1, 0);
	
	if(c) {
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjprog), position);
	  return true;
	}
	return false;
}

bool gtkgui_add_into_pl(unsigned int chan, char *file)
{
	struct gchan *c;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *fileutf8;

	c = (struct gchan *) list_get_data(listachan, chan+1, 0); 

	if(!c) return false;
	
	fileutf8 = g_filename_to_utf8(file, -1, NULL, NULL, NULL);
	func(_("I'm adding %s"), fileutf8);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->tree));
	
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			TITLE, fileutf8,
			-1);
	
	return true;
}

void gtkgui_set_maintitle(char *testo) 
{
	gtk_window_set_title(GTK_WINDOW(window), (gchar *) testo);
}

void gtkgui_set_statustext(char *testo) 
{
	contextid=gtk_statusbar_push(GTK_STATUSBAR(statusbar), 
			       contextid, (gchar *) testo);
}

bool gtkgui_sel_playlist(unsigned int chan, int row)
{
	struct gchan *c = NULL;
	GtkTreeModel *model = NULL;
	GtkTreeSelection *selection = NULL;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;

	c = (struct gchan *) list_get_data(listachan, chan+1, 0); 

	if(!c) return false;

	if(!c->playmode)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->play), FALSE);

	gtk_entry_set_text(GTK_ENTRY(c->ptime), "00:00:00");
	gtk_adjustment_set_value(GTK_ADJUSTMENT(c->adjprog), 0.0);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->tree));
	if(!model) return false;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(c->tree));
	if(!selection) return false;
	gtk_tree_selection_get_selected(selection, NULL, &iter);
	path = gtk_tree_model_get_path(model, &iter);
	if(!path) return false;
	
	gtk_tree_selection_select_path(selection, path);
	
	gtk_tree_path_free(path);
	

	return true;
}

void gtkgui_toggle_mic(bool i) 
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mic), i);
}

void gtkgui_toggle_spk(bool i)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(linein), i);
}

void gtkgui_toggle_lineout(bool i) 
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(speakout), i);
}

void gtkgui_set_vuvolume(int value) 
{
	gtk_adjustment_set_value(GTK_ADJUSTMENT(vu->vu_adjvol), (float) value);
}

void gtkgui_set_vuband(int value)
{
	char tmp[32];

	gtk_adjustment_set_value(GTK_ADJUSTMENT(vu->vu_adjband), (float) value);
	snprintf(tmp, 32, _("%d byte/s"), value/ 8);
	gtk_label_set_text(GTK_LABEL(vu->vu_labband), tmp);

}

void gtkgui_exit(void) 
{
	gtk_widget_destroy(window);
	g_list_free(listachan);
	g_list_free(lamelist);
	g_list_free(ogglist);
}
