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

#ifndef GENERIC_GUI_H
#define GENERIC_GUI_H

#include <stdio.h>
#include <string.h>

#include <libintl.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <jmixer.h>
#include <encoder.h>
#include <gchan.h>
#include <ice.h>
#include <vumeters.h>
#include <jutils.h>
#include <../generic.h>


#define GTK_DISABLE_DEPRECATED

/* FIXME: support more than 6 chan */
#define NOIDX 12
#define NOPOS 13
#define NOTOGGLE 12
#define TOGGLE 13

/* ripped from wolfpack.twu.net/docs/gtkdnd */
#define DRAG_TAR_NAME_0		"text/plain"
#define DRAG_TAR_INFO_0		0

#define DRAG_TAR_NAME_1		"text/uri-list"		/* not url-list */
#define DRAG_TAR_INFO_1		1

#define DRAG_TAR_NAME_2		"STRING"
#define DRAG_TAR_INFO_2		2
/* end */

#define _(x)	gettext(x)
#define N_(x)	x

/* experimental */
//#define WITH_SPEED

/* FIXME: cleaning up */

class Stream_mixer;

extern GtkWidget *window, *statusbar, *mic, *linein, *speakout;
extern struct pack pack1, pack2, pack3;
extern guint contextid;
extern GtkWidget *vbox, *fix;
extern GtkWidget *lametab, *oggtab;

extern GtkWidget *winil, *profentry;
extern GtkWidget *winvu;

extern Stream_mixer *mixer;
extern GList *listachan, *lamelist, *ogglist;
extern GList *iceprof, *vorbisprof, *lameprof;
extern int lameid, oggid;
extern float storevol[MAX_CHANNELS];
extern char *pathfile;
extern struct vumeters *vu;
extern int dndch;
extern gboolean dndlock;
extern bool state; 
extern bool vu_status;

enum {
TITLE,
N_COLUMNS };

#endif
