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

#ifndef MUSEGTK_H
#define MUSEGTK_H

bool gtkgui_init(int argc, char **argv, Stream_mixer *mix);
bool gtkgui_refresh(void);
void gtkgui_exit(void);

bool gtkgui_get_state(void);

/* @chan: [0-5]  @position: [0.0-1.0] */
bool gtkgui_set_pos(unsigned int chan, float increment);
/* @chan: [0-5]  @lcd: time text*/
bool gtkgui_set_lcd(unsigned int chan, char *lcd);
/* @chan: [0-5]  @position: file which should be added*/
bool gtkgui_add_into_pl(unsigned int chan, char *file);
/* @chan: [0-5] @row: [1-...] row to be to select in list */
bool gtkgui_sel_playlist(unsigned int chan, int row);
/* @state: FALSE == released */
void gtkgui_toggle_mic(bool state);
void gtkgui_toggle_spk(bool state);
void gtkgui_toggle_lineout(bool state);

void gtkgui_set_maintitle(char *);
void gtkgui_set_statustext(char *);
/* vumeters */
void gtkgui_set_vuvolume(int);
void gtkgui_set_vuband(int);

#endif
