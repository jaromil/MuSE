/* A simple radio scheduler - Multiple Streaming Engine
 * Copyright (C) 2004 nightolo <night@autistici.org>
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

#include <gtk/gtk.h>

#include <jutils.h>
#include <jmixer.h>

#include <radiosched.h>

#include <gen.h>
#include <config.h>

enum {
	SOURCE,
	COMMENT,
	START_TIME,
	END_TIME,
	COLUMN_EDITABLE,
	NUM_COLUMNS
};

void rsched_new(GtkWidget *w)
{
	GtkWidget *winsched, *scroll, *tree;
	GtkWidget *tmpvbox, *tmphbox, *button;
	GtkWidget *tmpwid, *tmpwid1, *tmptable;
	GtkWidget *align;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;
	GtkTreeSelection *select;

	winsched = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(winsched), _("MuSE - Radio Scheduler") );
	gtk_container_set_border_width(GTK_CONTAINER(winsched), 6);
	g_signal_connect(G_OBJECT(winsched), "delete_event",
			G_CALLBACK(gtk_widget_destroy), NULL);
	
	tmpvbox = gtk_vbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(tmpvbox), 6);
	gtk_container_add(GTK_CONTAINER(winsched), tmpvbox);
	
	store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, 
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
			SOURCE, "http://muse.dyne.org:8000/lasurchi.mp3",
			COMMENT, "We got it!",
			START_TIME, "08:00",
			END_TIME, "10:00",
			COLUMN_EDITABLE, TRUE,
			-1);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
			SOURCE, "http://papuasia.org:8000/rcyb.ogg",
			COMMENT, "Radio Cybernet",
			START_TIME, "10:00",
			END_TIME, "12:00",
			COLUMN_EDITABLE, TRUE,
			-1);
	
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(tmpvbox), scroll, TRUE, TRUE, 6);
	gtk_container_add(GTK_CONTAINER(scroll), tree);
	
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	/* Add Column */
	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			G_CALLBACK(cell_edited), (gpointer) store);
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(SOURCE));
	column = gtk_tree_view_column_new_with_attributes(_("Source"), renderer, 
			"text", SOURCE, "editable", COLUMN_EDITABLE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			G_CALLBACK(cell_edited), (gpointer) store);
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COMMENT));
	column = gtk_tree_view_column_new_with_attributes(_("Comment"), renderer, 
			"text", COMMENT, "editable", COLUMN_EDITABLE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	
	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			G_CALLBACK(cell_edited), (gpointer) store);
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(START_TIME));
	column = gtk_tree_view_column_new_with_attributes(_("Start Time"), renderer,
			"text", START_TIME, "editable", COLUMN_EDITABLE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			G_CALLBACK(cell_edited), (gpointer) store);
	g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(END_TIME));
	column = gtk_tree_view_column_new_with_attributes(_("End Time"), renderer, 
			"text", END_TIME, "editable", COLUMN_EDITABLE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(tree), TRUE);
	gtk_widget_set_size_request(scroll, 530, 200);
	/* A separator */
	tmpwid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(tmpvbox), tmpwid, FALSE, FALSE, 6);
	
	/* Add Button */
	//tmphbox = gtk_hbox_new(FALSE, 6);
	//gtk_box_pack_start(GTK_BOX(tmpvbox), tmphbox, FALSE, FALSE, 6);
	tmptable = gtk_table_new(1, 6, TRUE);
	gtk_box_pack_start(GTK_BOX(tmpvbox), tmptable, FALSE, FALSE, 6);
	gtk_table_set_col_spacings(GTK_TABLE(tmptable), 6);
	
	tmpwid = gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON);
	tmpwid1 = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);
	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(button), tmpwid1);
	tmpwid = gtk_label_new("Add");
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(rsched_add), (gpointer) store);
	g_object_unref(store);
	//gtk_box_pack_start(GTK_BOX(tmphbox), button, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmptable), button, 2, 3, 0, 1);
	
	tmpwid = gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON);
	tmpwid1 = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);
	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(button), tmpwid1);
	tmpwid = gtk_label_new("Remove");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(rsched_remove), (gpointer) tree);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);
	
	//gtk_box_pack_start(GTK_BOX(tmphbox), button, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmptable), button, 3, 4, 0, 1);
	
	tmpwid = gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON);
	tmpwid1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);
	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(button), tmpwid1);
	tmpwid = gtk_label_new("Reload");
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);

	//gtk_box_pack_start(GTK_BOX(tmphbox), button, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmptable), button, 4, 5, 0, 1);

	tmpwid = gtk_image_new_from_stock(GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON);
	tmpwid1 = gtk_hbox_new(FALSE, 0);
	button = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(button), tmpwid1);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);
	tmpwid = gtk_label_new("Cancel");
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
			G_CALLBACK(gtk_widget_destroy), winsched);
	gtk_box_pack_start(GTK_BOX(tmpwid1), tmpwid, FALSE, FALSE, 0);

	//gtk_box_pack_start(GTK_BOX(tmphbox), button, FALSE, FALSE, 0);
	gtk_table_attach_defaults(GTK_TABLE(tmptable), button, 5, 6, 0, 1);

	
	gtk_widget_show_all(winsched);
}

void rsched_add(GtkWidget *w, gpointer data)
{
	GtkTreeModel *model = (GtkTreeModel *) data;
	GtkTreeIter iter;

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			SOURCE, "Insert Source",
			COMMENT, "Insert Comment",
			START_TIME, "XX:XX",
			END_TIME, "XX:XX",
			COLUMN_EDITABLE, TRUE,
			-1);
}

void rsched_remove(GtkWidget *w, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeView *tree = (GtkTreeView *) data;
	GtkTreeSelection *select;
	GtkTreeModel *model;
	
	select = gtk_tree_view_get_selection(tree);

	if( gtk_tree_selection_get_selected(select, &model, &iter) ) 
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

}

void cell_edited(GtkCellRendererText *cell, const gchar *path_string,
		const gchar *new_text, gpointer data)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	GtkTreeModel *model = (GtkTreeModel *) data;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	gchar *old_text;
	
	if(!new_text[0])
		return;
	gtk_tree_model_get_iter(model, &iter, path);

	gtk_tree_model_get(model, &iter, column, &old_text, -1);
	g_free(old_text);
			
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, new_text, -1);

	gtk_tree_path_free(path);

}

