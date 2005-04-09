/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2002-2004 jaromil <jaromil@dyne.org>
 *
 * This sourcCARBONe code is free software; you can redistribute it and/or
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

#ifndef __CARBON_COMMON_H__
#define __CARBON_COMMON_H__

#include <Carbon/Carbon.h>


/* CONTROLS */
#define CARBON_GUI_REMOVE_CHANNEL 'rmCh'
#define CARBON_GUI_APP_SIGNATURE 'MuSE'

/* MAIN WINDOW */
#define MAIN_CONTROLS_NUM 6

#define STREAM_BUT 0
#define STREAM_BUT_ID 100
#define NEWCH_BUT 1
#define NEWCH_BUT_ID 101
#define SNDOUT_BUT 2
#define SNDOUT_BUT_ID 102
#define SNDIN_BUT 3
#define SNDIN_BUT_ID 103
#define VOL_BUT 4
#define VOL_BUT_ID 104
#define ABOUT_BUT 5
#define ABOUT_BUT_ID 105

/* CHANNEL WINDOW */
#define PLAYLIST_BOX_ID 307

/* MSG WINDOW */


/* EVENTS */

#define CARBON_GUI_EVENT_CLASS	'MusE'
#define CG_RMCH_EVENT	'rmCh'
#define CG_RMCH_EVENT_PARAM	'cidx'
#endif
