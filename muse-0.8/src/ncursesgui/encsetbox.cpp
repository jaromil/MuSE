/* A NCURSES/CDK TUI (Text User Interface) for MuSE
 * Copyright (C) 2003 Luca 'rubik' Profico <rubik@olografix.org>
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

#include "encsetbox.h"
#include "muse_console.h"

EncSetBox::EncSetBox ()
{
	bitratelist[0] = strdup ("8");
	bitratelist[1] = strdup ("16");
	bitratelist[2] = strdup ("24");
	bitratelist[3] = strdup ("32");
	bitratelist[4] = strdup ("56");
	bitratelist[5] = strdup ("64");
	bitratelist[6] = strdup ("96");
	bitratelist[7] = strdup ("128");

	freqlist[0] = strdup ("auto");
	freqlist[1] = strdup ("11000");
	freqlist[2] = strdup ("16000");
	freqlist[3] = strdup ("22050");
	freqlist[4] = strdup ("44100");

	chanlist[0] = strdup ("0 - stereo");
	chanlist[1] = strdup ("1 - jstereo");
	chanlist[2] = strdup ("2 - dual chan");
	chanlist[3] = strdup ("3 - mono");

	qualitylist[0] = strdup ("1 (best)");
	qualitylist[1] = strdup ("2");
	qualitylist[2] = strdup ("3");
	qualitylist[3] = strdup ("4");
	qualitylist[4] = strdup ("5");
	qualitylist[5] = strdup ("6");
	qualitylist[6] = strdup ("7");
	qualitylist[7] = strdup ("8");
	qualitylist[8] = strdup ("9 (worst)");
	
	filteringlist[0] = strdup ("auto");
	filteringlist[1] = strdup ("none");
	filteringlist[2] = strdup ("manual");
}

void EncSetBox::setscreen (CDKSCREEN *screen)
{
	cdkscreen = screen;
}

void EncSetBox::setmixer (Stream_mixer *m, encdata *e)
{
	mixer = m;
	enc = e;
}

void EncSetBox::setparm (int x, int y)
{
	warning ("EncSetBox::setparm() cdkscreen = %p [START]", cdkscreen);
	xbase=x; ybase=y;

	bitrate.setscreen (cdkscreen); 
	bitrate.box=FALSE;
	filtering.setscreen (cdkscreen); 
	filtering.box=FALSE;
	frequency.setscreen (cdkscreen); 
	frequency.box=FALSE;
	lowpasshz.setscreen (cdkscreen); 
	lowpasshz.box=FALSE;
	channels.setscreen (cdkscreen); 
	channels.box=FALSE;
	highpasshz.setscreen (cdkscreen); 
	highpasshz.box=FALSE;
	quality.setscreen (cdkscreen); 
	quality.box=FALSE;
	label1.setscreen (cdkscreen);

	bitrate.setparm (NULL, "</48>b<!48></32>itrate:<!32>", xbase +1, ybase +8, bitratelist, 8, 2, false);
	filtering.setparm  (NULL, "</32>freq. fil<!32></48>t<!48></32>ering:<!32>", xbase +15, ybase +8, filteringlist, 3, 0, false);
	frequency.setparm (NULL, "</48>f<!48></32>requency:<!32>", xbase+44, ybase +8, freqlist, 5, 2, false);
	lowpasshz.setparm (NULL, "</48>l<!48></32>owpassHZ:<!32>", 5, xbase +2, ybase+10, false, '_');
	highpasshz.setparm (NULL, "</32>h<!32></48>i<!48></32>ghpassHZ:<!32>", 5, xbase +20, ybase+10, false, '_');
	channels.setparm (NULL, "</48>c<!48></32>hannels:<!32>", xbase +37, ybase +10, chanlist, 4, 0, false);
	quality.setparm (NULL, "</48>q<!48></32>uality:<!32>", xbase+1, ybase +12, qualitylist, 9, 3, false);
	label1.setparm (xbase, ybase,30, 1);
	label1.setvalue ("Encoder settings:");
	
	warning ("EncSetBox::setparm() objcnt = %d ", cdkscreen->objectCount);

	label1.unreg ();
	bitrate.unreg (); 
	filtering.unreg ();
	frequency.unreg ();
	lowpasshz.unreg ();
	channels.unreg ();
	highpasshz.unreg ();
	quality.unreg (); 

	// showcurrval();

	func ("EncSetBox::setparm() cdkscreen = %p objcnt = %d [END]", cdkscreen,cdkscreen->objectCount);

}

void EncSetBox::reg (void)
{
	func ("EncSetBox::draw() cdkscreen =%p objcnt = %d [START]", cdkscreen, cdkscreen->objectCount);
	label1.reg(); label1.draw();
	bitrate.reg(); bitrate.draw();
	filtering.reg(); filtering.draw();
	frequency.reg(); frequency.draw();
	lowpasshz.reg(); lowpasshz.draw();
	highpasshz.reg(); highpasshz.draw();
	channels.reg(); channels.draw();
	quality.reg(); quality.draw();
	func ("EncSetBox::draw() cdkscreen =%p objcnt = %d [END]", cdkscreen, cdkscreen->objectCount);
}

void EncSetBox::draw (void)
{
	func ("EncSetBox::draw() cdkscreen =%p objcnt = %d [START]", cdkscreen, cdkscreen->objectCount);
	label1.draw();
	bitrate.draw();
	filtering.draw();
	frequency.draw();
	lowpasshz.draw();
	highpasshz.draw();
	channels.draw();
	quality.draw();
	func ("EncSetBox::draw() cdkscreen =%p objcnt = %d [END]", cdkscreen, cdkscreen->objectCount);
}

void EncSetBox::unreg (void)
{

	func ("EncSetBox::unreg() bitrate.cdkscreen=%p", bitrate.getscreen());
	label1.unreg(); label1.erase();
	bitrate.unreg(); bitrate.erase();
	filtering.unreg(); filtering.erase();
	frequency.unreg(); frequency.erase();
	lowpasshz.unreg(); lowpasshz.erase();
	highpasshz.unreg(); highpasshz.erase();
	channels.unreg(); channels.erase();
	quality.unreg(); quality.erase();
	func ("EncSetBox::unreg() bitrate.cdkscreen=%p label1.cdkscreen=%p", bitrate.getscreen(), label1.getscreen());
}

void EncSetBox::erase (void)
{
	filtering.destroy();
	bitrate.destroy();
	frequency.destroy();
	lowpasshz.destroy();
	highpasshz.destroy();
	channels.destroy();
	quality.destroy();
	label1.destroy();
}

int EncSetBox::tabentry (int k)
{

	int retval = 0;
	int loop=0;

	while (!loop) {
		loop=1;

		switch (k) {
			case 'b': //bitrate
				bitrate.activate();
				if (bitrate.lastkey() != 9) break;
			case 't': //frequency filtering
				filtering.activate();
				if (filtering.lastkey() != 9) break;
			case 'f': //frequency
				frequency.activate();
				if (frequency.lastkey() != 9) break;
			case 'l': //lowpass hz
				lowpasshz.activate();
				if (lowpasshz.lastkey() != 9) break;
			case 'i': //highpass hz
				highpasshz.activate();
				if (highpasshz.lastkey() != 9) break;
			case 'c': //channels
				channels.activate();
				if (channels.lastkey() != 9) break;
			case 'q': // quality
				quality.activate();
				if (quality.lastkey() != 9) break;
			case NULL:
				k='b'; // start from first case
				loop = 0;
				break;
			default:
				retval = 0;
				break;
		}
	}
	return retval;
}

void EncSetBox::setval (int id)
{

  enc->outchan = mixer->get_enc (id);
  
  enc->outchan->bps(bitrate.getcurritem());
  //filtering; ??
  enc->outchan->freq(frequency.getcurritem());;
  enc->outchan->lowpass(atoi(lowpasshz.getvalue()));
  enc->outchan->highpass(atoi(highpasshz.getvalue()));
  enc->outchan->channels(channels.getcurritem());
  enc->outchan->quality(quality.getcurritem()+1);
}

void EncSetBox::showcurrval (int id)
{
	enc->outchan = mixer->get_enc(id);

	char *temp=NULL;
	temp = (char *) malloc(32);

	bitrate.setitem(enc->outchan->bps());
	filtering.setitem(0);
	frequency.setitem(enc->outchan->freq());
	snprintf (temp, 32, "%d", enc->outchan->lowpass());
	lowpasshz.setvalue(temp);
	snprintf (temp, 32, "%d", enc->outchan->highpass());
	highpasshz.setvalue(temp);
	channels.setitem(enc->outchan->channels());
	quality.setitem((int)(enc->outchan->quality())-1);

	/*
	//lame settings
	switch (mixer->lame.bps)
	{
		case 8: bitrate.setitem (0); break;
		case 16: bitrate.setitem (1); break;
		case 24: bitrate.setitem (2); break;
		case 32: bitrate.setitem (3); break;
		case 56: bitrate.setitem (4); break;
		case 64: bitrate.setitem (5); break;
		case 96: bitrate.setitem (6); break;
		case 128: bitrate.setitem (7); break;
	}

	//filtering.setitem ();
	switch (mixer->lame.freq)
	{
		//case auto: frequency.setitem (0); break;
		case 11000: frequency.setitem (1); break;
		case 16000: frequency.setitem (2); break;
		case 22050: frequency.setitem (3); break;
		case 44100: frequency.setitem (4); break;
	}

	snprintf (temp,32,"%d",mixer->lame.lowpass);
	lowpasshz.setvalue (temp);
	snprintf (temp,32,"%d",mixer->lame.highpass);
	highpasshz.setvalue (temp);
	channels.setitem (mixer->lame.mode);
	quality.setitem (mixer->lame.quality - 1);
	*/

	free (temp);
}

void EncSetBox::cleanup()
{

	func ("EncSetBox::cleanup()");
	highpasshz.reg();
	highpasshz.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	filtering.reg();
	filtering.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	bitrate.reg();
	bitrate.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	frequency.reg();
	frequency.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	channels.reg();
	channels.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	quality.reg();
	quality.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	label1.reg();
	label1.destroy();
	warning ("objcnt = %d",cdkscreen->objectCount);
	lowpasshz.reg();
	lowpasshz.destroy();
	func ("end EncSetBox::cleanup()");
}


EncSetBox::~EncSetBox ()
{
	int i;
	func ("EncSetBox::~EncSetBox screen = %p objcnt = %d()", cdkscreen, cdkscreen->objectCount);

	for (i=0;i<8;i++) { free(bitratelist[i]); bitratelist[i] = NULL; }
	for (i=0;i<5;i++) { free(freqlist[i]); freqlist[i] = NULL; }
	for (i=0;i<4;i++) { free(chanlist[i]); chanlist[i] = NULL; }
	for (i=0;i<9;i++) { free(qualitylist[i]); qualitylist[i] = NULL; }
	for (i=0;i<3;i++) { free(filteringlist[i]); filteringlist[i] = NULL; }

	/*
	bitrate.destroy();
	filtering.destroy();
	frequency.destroy();
	lowpasshz.destroy();
	highpasshz.destroy();
	channels.destroy();
	quality.destroy();
	label1.destroy();
	*/
	func ("end EncSetBox::~EncSetBox ()");
}
