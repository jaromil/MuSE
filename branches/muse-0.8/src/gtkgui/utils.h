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

#ifndef UTILS_GUI_H
#define UTILS_GUI_H

GtkWidget *createbbox(GtkWidget *);
void createtab(GtkWidget **);

void pack_new(void);
void cfade_set(GtkWidget *, struct pack *);

void putstatusbar(void);
void status_window(GtkWidget *, GdkEventButton *);
GtkWidget* create_pixmap(GtkWidget *widget, const gchar *filename);
GtkWidget *createpixmap(GtkWidget *, GtkWidget *, gchar **, gchar *, bool);
void win_profilesave(GtkWidget *, void *);
void set_tip(GtkWidget *, gchar *);
void gcb_set_speakout(GtkWidget *);
void gcb_set_linein(GtkWidget *);
void gcb_set_talk(GtkWidget *);
void gcb_exit(GtkWidget *, GdkEvent *);

#endif
