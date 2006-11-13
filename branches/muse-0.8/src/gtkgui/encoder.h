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

#ifndef ENCODER_H
#define ENCODER_H

struct encdata {
	GtkWidget *verbox, *tabbola, *tasti;
	GtkWidget *bitrate, *freq, *chan, *freqfil;	
	GtkWidget *lowps, *highps;
	GtkObject *adj1; /* basic setting */
	GtkWidget *qualityi, *adj_lab, *mode; /* (basic settings) */
	GtkWidget *expert, *lista, *profroot;
	/* 0.8 elements */
	OutChannel *outchan;
};

struct encdata *enc_new(char *); 
void enc_profmenu(struct encdata *);

void enc_fill(struct encdata *);
void enc_put(struct encdata *);
void enc_get(struct encdata *);

void gcb_set_pass(GtkWidget *, struct encdata *);
void gcb_set_enc(GtkWidget *, struct encdata *);

void gcb_enc_put(GtkWidget *, struct encdata *);
void gcb_enc_save(GtkWidget *, struct encdata *);

void gcb_enc_set_quality(GtkWidget *, struct encdata *);
void gcb_enc_set_mode(GtkMenuShell *, struct encdata *);

void expert_mode(GtkWidget *, GtkWidget *);
#endif
