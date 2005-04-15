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

#define SEEK_TIME_CONTROL 300
#define SEEK_CONTROL 312
#define PLAY_BUT 301
#define PAUSE_BUT 302
#define STOP_BUT 303
#define PREV_BUT 304
#define NEXT_BUT 305

#define VOLUME_CONTROL 306
#define PLAYLIST_BOX 307

#define REPEAT_BUT 308
#define LOOP_BUT 309
#define SHUFFLE_BUT 310
#define SELECTED_SONG_CONTROL 311

#define PLAY_CMD 'Play'
#define STOP_CMD 'Stop'
#define PAUSE_CMD 'Paus'
#define PREV_CMD 'Prev'
#define NEXT_CMD 'Next'
#define VOL_CMD 'sVol'

#define MENU_PLAY_CMD 'mPla'
#define MENU_REMOVE_CMD 'sRem'

/* STREAM WINDOW */
#define ADD_STREAM_BUT 800
#define DEL_STREAM_BUT 801
#define STREAM_TAB_CONTROL 803
#define QUALITY_CONTROL 804
#define MODE_CONTROL 805
#define STREAM_DESCR_CONTROL 806
#define BITRATE_CONTROL 807
#define FREQUENCY_CONTROL 808
#define FREQUENCY_FILTER_CONTROL 809
#define LOWPASS_CONTROL 810
#define HIGHPASS_CONTROL 811
#define RECORD_STREAM_FILENAME 812
#define RECORD_STREAM_BROWSE 813
#define RECORD_STREAM_BUT 814
#define ADD_SERVER_BUT 815
#define DEL_SERVER_BUT 816
#define SERVER_TAB_CONTROL 817
#define HOST_CONTROL 818
#define PORT_CONTROL 819
#define MNT_CONTROL 820
#define NAME_CONTROL 821
#define URL_CONTROL 822
#define DECRIPTION_CONTROL 823
#define LOGINT_TYPE_CONTROL 824
#define USERNAME_CONTROL 825
#define PASSWORD_CONTROL 826
#define SERVER_STATUS_CONTROL 827
#define CONNECT_BUTTON 828

/* MSG WINDOW */


/* EVENTS */

#define CARBON_GUI_EVENT_CLASS	'MusE'
#define CG_RMCH_EVENT	'rmCh'
#define CG_RMCH_EVENT_PARAM	'cidx'
#endif
