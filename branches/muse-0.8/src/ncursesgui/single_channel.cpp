/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2002 Luca 'rubik' Profico <rubik@olografix.org>
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
 *
 * $Id$
 *
 */

#include "single_channel.h"
#include "muse_console.h"

SChannel::SChannel ()
{
	time_slider.lowValue = 0;
	time_slider.highValue = 100;
	time_slider.currentValue = 10;
	time_slider.box = FALSE;

	volume_slider.lowValue = 0;
	volume_slider.highValue = 100;
	volume_slider.currentValue = 100;
	volume_slider.box = FALSE;

	playmode=0;
}

SChannel::~SChannel ()
{
	warning ("schannel destructor");
}

void SChannel::init( CDKSCREEN *screen, int xrel, int chan)
{
	char temp[20];
	int ybase = 2;

	func ("SChannel::init()");
	chanID = chan;

	parentWidth = getmaxx(screen->window) -1;
	parentHeight = getmaxy(screen->window) -1;

	time_slider.setscreen(screen);
	time_slider.setparm ("", "[T:]", parentWidth /2 - 8, xrel, ybase +1);

	volume_slider.setscreen(screen);
	volume_slider.setparm("", "[V:]", parentWidth /2 -8, xrel, ybase +2);

	infobox.setscreen (screen);
	infobox.setparm (xrel, ybase + 4, parentWidth/2-2);
	
	
	snprintf (temp, 19, "[chan %d] playlist:", chanID + 1);
	playlist_scroll.setscreen (screen);
	playlist_scroll.setparm(temp, xrel, ybase +7, parentHeight - 13, parentWidth/2-2);

	func ("SChannel::init() END");
}

void SChannel::defparam (void)
{
	infobox.setsongNR (" - ");
	infobox.setelapsedNR ("00:00");
	infobox.setmodeNR ("Play");
	playmode = 0;

	playlist_scroll.ulhook (ACS_LTEE);
	playlist_scroll.urhook (ACS_RTEE);
}


void SChannel::selnextitem (void)
{
	playlist_scroll.selnext();
}

void SChannel::selprevitem (void)
{
	playlist_scroll.selprev();
}

void SChannel::add_to_playlist (char *txt)
{
	warning ("scroll currentItem: %d", playlist_scroll.curritem());
	warning ("scroll currentHigh: %d", playlist_scroll.currhigh());
	warning ("scroll listSize: %d", playlist_scroll.listsize());
	warning ("scroll viewSize: %d", playlist_scroll.viewsize());
	playlist_scroll.additem (txt);
	warning ("scroll currentItem: %d", playlist_scroll.curritem());
	warning ("scroll currentHigh: %d", playlist_scroll.currhigh());
	warning ("scroll listSize: %d", playlist_scroll.listsize());
	warning ("scroll viewSize: %d", playlist_scroll.viewsize());
}

void SChannel::rem_from_playlist (int pos)
{
	playlist_scroll.delitem (pos);
}

float SChannel::incrvol (void)
{
	volume_slider.incr();
	return volume_slider.fpos();
}

float SChannel::decrvol (void)
{
	volume_slider.decr();
	return volume_slider.fpos();
}

void SChannel::zerovol (void)
{
	volume_slider.set_pos(0);
}

float SChannel::getvol (void)
{
	return volume_slider.fpos();
}

void SChannel::show(void)
{
	func ("SChannel::show");
	// volume slider
	volume_slider.draw();
	time_slider.draw();
	playlist_scroll.draw();
	infobox.draw();
	/*
	current_file.draw();
	elapsed_time.draw();
	play_mode.draw();
	*/
}

void SChannel::hide(void)
{
	// volume slider
	volume_slider.hide();
	time_slider.hide();
	playlist_scroll.hide();
	infobox.hide();
	/*
	current_file.draw();
	elapsed_time.draw();
	play_mode.draw();
	*/
}

void SChannel::reg (void)
{
	volume_slider.reg();
	time_slider.reg();
	playlist_scroll.reg();
	infobox.reg();
}

void SChannel::unreg (void)
{
	func ("SChannel::unreg START");
	volume_slider.unreg();
	time_slider.unreg();
	playlist_scroll.unreg();
	infobox.unreg();
	func ("SChannel::unreg END");
}

void SChannel::activate_volume()
{
	volume_slider.activate();
}

void SChannel::setctrl()
{
	func ("SChannel::setctrl()");
	infobox.setctrl();
	playlist_scroll.mark();
	func ("end SChannel::setctrl()");
}

void SChannel::unsetctrl()
{
	infobox.unsetctrl();
	playlist_scroll.unmark();
}

int SChannel::cplitem (void)
{
	return playlist_scroll.curritem();
}

void SChannel::set_time (char *lcd)
{
	infobox.setelapsed (lcd);
}

void SChannel::set_pos (float pos)
{
	time_slider.set_pos (pos);
	time_slider.draw();
}

int SChannel::cycmode (void)
{
	func ("SChannel::cycmode()");
	switch (playmode) {
		case 0:
			playmode = 1;
			infobox.setmode("Loop");
			break;
		case 1:
			playmode = 2;
			infobox.setmode("Cont");
			break;
		case 2:
			playmode = 0;
			infobox.setmode("Play");
			break;
	}
	return playmode;
}

int SChannel::setmode (int newmode)
{
	func ("SChannel::setmode()");
	playmode = newmode;
	switch (playmode) {
		case 0:
			infobox.setmode("Play");
			break;
		case 1:
			infobox.setmode("Loop");
			break;
		case 2:
			infobox.setmode("Cont");
			break;
	}
	return playmode;
}
		       
			
	
