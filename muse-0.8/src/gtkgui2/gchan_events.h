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

#ifndef GCHAN_EVENTS_H
#define GCHAN_EVENTS_H

/* event stuff: position and popup */
/* void gcb_event_pause_channel(GtkWidget *, GdkEventButton *, struct gchan *); */
gboolean gcb_event_set_position(GtkWidget *, GdkEventButton *, struct gchan *);

void DND_begin(GtkWidget *, GdkDragContext *, struct gchan *);
void DND_end(GtkWidget *, GdkDragContext *, struct gchan *);
gboolean DND_data_motion(GtkWidget *, GdkDragContext *, gint , gint , guint , struct gchan *);
gboolean DND_data_get(GtkWidget *, GdkDragContext *, GtkSelectionData *, guint , struct gchan *);
void DND_data_received(GtkWidget *, GdkDragContext *, gint, gint, GtkSelectionData *, guint, guint, struct gchan *);
void DND_data_delete(GtkWidget *, GdkDragContext *, struct gchan *);
gboolean gcb_event_view_popup(GtkWidget *, GdkEventButton *, struct gchan *);

#endif
