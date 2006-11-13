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

#ifndef GCHAN_H
#define GCHAN_H

struct gchan { 
	GtkWidget *frami, *windock;
	GtkWidget *play, *pause;
	GtkWidget *progress, *ptime; /* sound progress */
	GtkObject *adjprog, *adjvol;
	GtkWidget *vol_lab; /* volume label */
  //  GtkWidget *vol_text; /* VOL text over the knob */
#ifdef WITH_SPEED
	GtkWidget *speed; /* speed widget */
	GtkWidget *speed_lab; /* speed label */
	GtkObject *adjspeed; /* speed adjustment */
#endif
	GtkWidget *volume; /* maybe volume */
	guint volid;
	GtkWidget *tree;
	GtkWidget *cfade; /* crossfade widget */
	GtkObject *adjcfade; /* crossfade adjustment */
	guint cfadeid; /* to connect and disconnect callback */
	GtkWidget *table, *rmit, *dock, *hbox; /* button */
	GtkWidget *httpentry; /* mah */
	int rem;
	unsigned int playmode; /* 0: PLAY 1: LOOP 2: CONT */
	unsigned int channel; /* 1: seekable 2: stream */
	unsigned int idx;
	unsigned int pos; 
};

struct pack {
	GtkWidget *hbox, *hscale;
	GtkObject *adj;
	unsigned int id;
};


bool createch(void);

void gcb_deletech(GtkWidget *, struct gchan *);
struct gchan *gcb_findch(unsigned int, unsigned int);
void spawnfilew(GtkWidget *,struct gchan *); /* open file sel win */

void httpwin(GtkWidget *, struct gchan *);
void httpinsert(GtkWidget *, struct gchan *);

void gcb_set_playmode(GtkWidget *, struct gchan *);
void gcb_set_channel(GtkWidget *, struct gchan *);
void gcb_play_channel(GtkWidget *, struct gchan *);
void gcb_stop_channel(GtkWidget *, struct gchan *);
void gcb_pause_channel(GtkWidget *, struct gchan *);
void gcb_begin_channel(GtkWidget *, struct gchan *);
void gcb_end_channel(GtkWidget *, struct gchan *);
void gcb_set_volume(GtkAdjustment *, struct gchan *);
#ifdef WITH_SPEED
void gcb_set_speed(GtkAdjustment *, struct gchan *);
#endif
void gcb_rem_from_playlist(GtkWidget *, struct gchan *);
void gcb_add_file(GtkWidget *, GtkFileSelection *); 

#endif
