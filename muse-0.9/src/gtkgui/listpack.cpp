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

/* list and packing utilities, a step into 0.8 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <generic.h>
#include <gen.h>
#include <gchan.h>
#include <listpack.h>

#include <jutils.h>
#include <config.h>

/* list utilities */

void list_init(GList **list)
{
	*list=NULL;
}

bool list_add(GList **list, void *data, unsigned int pos, 
		unsigned int idx, GtkWidget *container)
{
	struct listitem *item;	

	item = (struct listitem *) g_malloc(sizeof(struct listitem));
	if(!item)
		return false;

	item->idx = idx;
	item->pos = pos;
	item->container = container;
	item->data = data;

	func("list_add:: item %p item->pos = %u	item->container= %p item->data=%p", item, item->pos,
			item->container, item->data);
	/**list = g_list_append(*list, (void *) item);*/
	*list = g_list_insert(*list, (void *) item, pos-1);
	return true;
}

bool list_remove(GList **list, unsigned int idx)
{
	GList *listrunner = g_list_first(*list);
	struct listitem *tmp = NULL;
	bool exist=false;
	
	while(listrunner) {
		tmp = (struct listitem *) listrunner->data;
		if(tmp->idx == idx) {
			exist=true;
			func("GTK_GUI:: list_remove : object %u found", idx);
			break;
		}
		listrunner = g_list_next(listrunner);
	}
	
	if(!exist) {
	  warning("GTK_GUI:: list_remove : object %u not found", idx);
	  return false;
	}
	
	gtk_widget_destroy(tmp->container);

	*list = g_list_remove_link(*list, listrunner);
	*list = g_list_remove(*list, listrunner->data);
	return true;

}

/* list private functions */
struct listitem *list_find_bypos(GList *list, unsigned int pos)
{
	GList *listrunner = g_list_first(list);
	struct listitem *tmp;

	//func("GTK_GUI:: list_find_bypos list=%p pos=%u", list, pos);
	while(listrunner) {
		tmp= (struct listitem *) listrunner->data;
		if(tmp->pos == pos) 
			return tmp;
		else
			listrunner=g_list_next(listrunner);
	}
	
	//func("GTK_GUI:: list_find_bypos returns NULL for pos %u", pos);
	return NULL;
}

struct listitem *list_find_byidx(GList *list, unsigned int idx)
{
	GList *listrunner = g_list_first(list);
	struct listitem *tmp;
	
	while(listrunner) {
		tmp = (struct listitem *) listrunner->data;
		if(tmp->idx == idx) 
			return tmp;
		else
			listrunner = g_list_next(listrunner);
	}
	return NULL;
}

/* list utilities */
bool list_set_pos(GList *list, GtkWidget *container, unsigned int pos)
{
	GList *listrunner = g_list_first(list);
	struct listitem *tmp;

	if(!list)
		warning("GTK_GUI:: list_set_pos: list is NULL");
	/* find listitem through container */
	while(listrunner) {
		tmp = (struct listitem *) listrunner->data;
		if(tmp->container == container) {
			tmp->pos=pos;
			return true;
		} else
			listrunner = g_list_next(listrunner);
	}

	warning("I CANNOT SET POSITION %d FOR container= %p", pos, container);
	return false;	
	
}

void *list_get_data(GList *list, unsigned int idx, unsigned int pos)
{
	struct listitem *tmp;
	
	if(pos == 0) {
		if(!(tmp = list_find_byidx(list, idx)))
			return tmp;
	} else {
		if(!(tmp = list_find_bypos(list, pos)))
			return tmp;
	}
	
	return tmp->data;
}

void list_debug(GList *list)
{
	GList *listrunner;
	struct listitem *tmp;

	if(!(listrunner=g_list_first(list)))
		func("GTK_GUI:: list_debug: g_list_first is NULL, sorry");

	func("DEBUGGING THOUGH list %p", list);
	while(listrunner) {
		tmp= (struct listitem *) listrunner->data;
		if(!tmp)
			func("GTK_GUI:: cannot debug list %p", list);
		else 
			func("-----DEBUG-----\n*)listitem %p\n*)pos %u\n*)container %p\n ----END DEBUG-----", tmp, tmp->pos, tmp->container);
		listrunner=g_list_next(listrunner);
	}
		
}

/* for docking and undocking use */
void list_swap(GList *list, GtkWidget *oldcontainer, GtkWidget *newcontainer)
{
	GList *listrunner = g_list_first(list);
	struct listitem *tmp;

	func("list_swap called");
	while(listrunner) {
		tmp = (struct listitem *) listrunner->data;
		if(tmp->container == oldcontainer) {
			tmp->container = newcontainer;
			func("list_swap :: new container assigned = %p out to %p", 
					newcontainer, tmp->container);
		} else
			listrunner = g_list_next(listrunner);
	}
}

/* packing utilites */
/* pack container in first position free in table */
unsigned int pack_chan_insert(GtkWidget *container) {
	unsigned int pos;
	struct listitem *tmp;
	struct gchan *chan;
	
	for(pos = 1; (tmp = list_find_bypos(listachan, pos)); pos++)
		;
	
	func("GTK_GUI:: pack_chan_insert: packing %p into %u", container, pos);
	
	if(pos > MAX_CHANNELS) {
		error("GTK_GUI:: pack_chan_insert: Sorry MAX_CHANNEL reached");
		return 0;
	}
	
	if(!(list_set_pos(listachan, container, pos)))
		warning("GTK_GUI:: pack_insert : error in setting position %d", pos);
		
	tmp = list_find_bypos(listachan, pos);
	chan = (struct gchan *) tmp->data;
	chan->pos = pos;

	switch(pos) {
		case 1: gtk_box_pack_start(GTK_BOX(pack1.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_hide(pack1.hscale);
				gtk_widget_show_all(pack1.hbox);
				break;
		case 2: gtk_box_pack_start(GTK_BOX(pack1.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_show(pack1.hscale);
				gtk_widget_show_all(pack1.hbox);
				break;
		case 3: gtk_box_pack_start(GTK_BOX(pack2.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_hide(pack2.hscale);
				gtk_widget_show_all(pack2.hbox);
				break;
		case 4: gtk_box_pack_start(GTK_BOX(pack2.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_show(pack2.hscale);
				gtk_widget_show_all(pack2.hbox);
				break;
		case 5: gtk_box_pack_start(GTK_BOX(pack3.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_hide(pack3.hscale);
				gtk_widget_show_all(pack3.hbox);
				break;
		case 6: gtk_box_pack_start(GTK_BOX(pack3.hbox), container, 
						TRUE, TRUE, 0);
				gtk_widget_show(pack3.hscale);
				gtk_widget_show_all(pack3.hbox);
	}


	return pos;
}

unsigned int pack_insert(GList *list, GtkWidget *table, GtkWidget *container)
{
	unsigned int pos;
	struct listitem *tmp;

	/* FIXME: try g_list function */
	for(pos=1; (tmp=list_find_bypos(list, pos)); pos++)
		;

	func("GTK_GUI:: pack_insert: packing %p in pos %u", container, pos);

	if(pos > MAX_CHANNELS) {
		error("GTK_GUI::packchan : Sorry I'm not able to pack chan %u \n",pos);
		return 0;
	}

	switch(pos) {
		case 1: gtk_table_attach_defaults(GTK_TABLE(table), container,
						0, 1, 0, 1);
				break;
		case 2: gtk_table_attach_defaults(GTK_TABLE(table), container, 
						1, 2, 0, 1);
				break;
		case 3: gtk_table_attach_defaults(GTK_TABLE(table), container,
						0, 1, 1, 2);
				break;
		case 4: gtk_table_attach_defaults(GTK_TABLE(table), container,
						1, 2, 1, 2);
				break;
		case 5: gtk_table_attach_defaults(GTK_TABLE(table), container,
						0, 1, 2, 3);
				break;
		case 6: gtk_table_attach_defaults(GTK_TABLE(table), container,
						1, 2, 2, 3);
				break;
		default: g_print("nothing to do");
	}

	/* pos update */
	if(!(list_set_pos(list, container, pos)))
		warning("GTK_GUI:: pack_insert : error in setting position %d", pos);
	
	gtk_widget_show_all(container);
	return pos;
}

void pack_refresh(GList *list, GtkWidget *table, bool chanlist)
{
	unsigned int pos;
	struct listitem *tmp;

	func("pack_refresh(list=%p, table=%p)", list, table);
	/* get first free position */
	for(pos=1; (tmp=list_find_bypos(list, pos)); pos++)
			;

	func("pack_refresh:: first free position is %d", pos);

	if((tmp=list_find_bypos(list, pos+1))) {
		gtk_object_ref(GTK_OBJECT(tmp->container));
		
		gtk_container_remove(GTK_CONTAINER(GTK_WIDGET(tmp->container)->parent),
				tmp->container);
		if(!chanlist)
			pack_insert(list, table, tmp->container);
		else 
			pack_chan_insert(tmp->container);


		gtk_object_unref(GTK_OBJECT(tmp->container));
		pack_refresh(list, table, chanlist);
	}

}	



