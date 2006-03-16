/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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

#include "credits.h"

#define DEVELOPERS \
"Development team : \n\n\
Denis \"jaromil\" Rojo - main coder and mantainer.\n\
Andrea \"xant\" Guzzo - OSX and Carbon user interface.\n\
Antonino \"nightolo\" Radici - GTK user interface.\n\
Luca \"rubik\" Profico - NCurses user interface.\n\
Angelo \"pallotron\" Failla - more input channels.\n\
Eugen Melinte - radio scheduler, http stream parser.\n"


#define CREDITS \
"MuSE is an application for the mixing, encoding, and\n\
streaming of sound: is an engine that can simultaniously\n\
mix up to 6 separate MP3 or OggVorbis audio files from\n\
the hard drive or the network, where each channel of\n\
audio can be dynamicly adjusted for speed and volume\n\
plus a soundcard line-in channel. The resulting stream\n\
can be played locally on the sound card and/or\n\
encoded as an mp3 network stream to an icecast or\n\
shoutcast server (ready to be mixed and played\n\
again by other muses... ;)\n\
\n\
MuSE as it is now would have never existed without\n\
the contributions of:\n\
= SERVUS.AT trusting this project since its beginning\n\
= Alex, Rasty and Martinez ( maria libera! ) good vibes\n\
= August Black ( aug.ment.org ) original interface design\n\
= Asbesto Molesto ( freaknet.org ) testing and docu\n\
= Eric de Castro Lopo ( mega-nerd.com ) resample & libsnd\n\
= Filippo \"Godog\" ( esaurito.net ) organization and deb\n\
= Lobo for herbivore and the karma to code well\n\
= Markus Seidl ( funda.ment.org ) vision of such a tool\n\
= Matteo Nastasi aka Mop ( alternativeoutput.it ) dsp exp\n\
= PUBLIC VOICE Lab ( pvl.at ) support and horizons\n\
\n\
Also waves and thanks to:\n\
voyager, void, blicero, sandolo, eni,\n\
the Freaknet Medialab, the ASCII squat,\n\
henk, the imc-audio collective, jeff,\n\
the LOA hacklab, the TPO, bundes & didi,\n\
indymedia italy, neural.it, autistici.org,\n\
Mag-One, radio Ondarossa, bomboclat, newmark\n\
c1cc10, vanguardist, janki, kysucix, Adam\n\
and all the others i'm forgetting here!\n\
\n\
\nMuSE-OSX redistributes statically, the following libraries:\n\
= libmpeg by Woo-jae Jung and Mikael Hedin\n\
= libshout by Jack Moffit and others\n\
= libcdk by Mike Glover\n\
= libportaudio by Ross Bencina and Phil Burk \n\
= libvorbis  by Jack Moffitt \n\
= libmp3lame by The LAME Project \n\
= libsndfile by Erik de Castro Lopo \n\
\n\n\
MuSE sourcecode also got inspirations and code\n\
snippets from the stream mixer sourcecode by\n\
Scott Manley, the buffered FIFO pipe source\n\
by Charles Samuels, icons by Jakub Steiner\n\n\
MuSE Streamer is copyleft (c)\n\
2000-2004 by Denis \"jaromil\" Rojo\n\
\n\
MuSE-OSX (Carbon GUI) is copyleft (c)\n\
2005 by Andrea \"xant\" Guzzo\n\
\n\
MuSE's GTK+ GUI is copyleft (c)\n\
2001-2004 by Antonino \"nightolo\" Radici\n\
\n\
MuSE's NCURSES GUI is copyleft (c)\n\
2002-2004 by Luca \"rubik\" Profico\n\
\n\
DSP resampling routines are copyleft (c)\n\
2002 by Matteo \"MoP\" Nastasi\n\
\n\
MuSE's first GUI scheme is copyleft (c)\n\
2000-2001 by August Black\n\
\n\
part of the included code is copyright by the\n\
respective authors, please refer to the supplied\n\
sourcecode for further informations.\n\
\n---------------------------------------------------\n\n\
LICENCE: \n\n\
This source code is free software; you can redistribute \n\
it and/or modify it under the terms of the GNU Public \n\
License as published by the Free Software Foundation; \n\
either version 2 of the License, or (at your option) any \n\
later version. \n\
\n\
This source code is distributed in the hope that it will\n\
be useful, but WITHOUT ANY WARRANTY; without \n\
even the implied warranty of MERCHANTABILITY or \n\
FITNESS FOR A PARTICULAR PURPOSE. \n\
Please refer to the GNU Public License for more details. \n\
\n\
You should have received a copy of the GNU Public \n\
License along with this source code; if not, write to: \n\
Free Software Foundation, Inc., 675 Mass Ave, \n\
Cambridge, MA 02139, USA"


static OSStatus AboutEventHandler (
	EventHandlerCallRef nextHandler, EventRef event, void *userData);


/****************************************************************************/
/* credits globals */
/****************************************************************************/
const ControlID msgBoxID = { CARBON_GUI_APP_SIGNATURE, ABOUT_MESSAGE };
const ControlID devBoxID = { CARBON_GUI_APP_SIGNATURE, ABOUT_DEVELS };

const EventTypeSpec windowEvents[] = {
	{  kEventClassWindow, kEventWindowClose }
};

AboutWindow::AboutWindow(WindowRef mainWin,IBNibRef nib) 
{
	parent=mainWin;
	nibRef=nib;
	HIViewRef msgTextView,devTextView;
	OSStatus err;
	
	msg = new CarbonMessage(nibRef);
	
	err = CreateWindowFromNib(nibRef,CFSTR("AboutWindow"),&window);
	if(err != noErr) 
		msg->error("Can't create the about window (%d)!!",err);
	
	err = GetControlByID(window,&msgBoxID,&msgBox);
	if(err != noErr) 
		msg->error("Can't get HIViewRef for rasta image (%d)!!",err);

	/* Create a window group and put the stream window inside it ...
	 * this is done just to let Quartz handle window layering correctly */
	err=CreateWindowGroup(kWindowGroupAttrMoveTogether|kWindowGroupAttrLayerTogether|
		kWindowGroupAttrSharedActivation|kWindowGroupAttrHideOnCollapse,&aboutGroup);
	err=SetWindowGroup(window,aboutGroup);
	
	/* get TXNObject for the developers box */
	err= HIViewFindByID(HIViewGetRoot(window), msgBoxID, &msgTextView);
	if(err!=noErr) msg->error("Can't get msgBox view (%d)!!");
	msgText = HITextViewGetTXNObject(msgTextView);
	if(!msgText) {
		msg->error("Can't get msgText object from status window!!");
	}
	/* and do the same for the messages box */
	err= HIViewFindByID(HIViewGetRoot(window), devBoxID, &devTextView);
	if(err!=noErr) msg->error("Can't get devBox view (%d)!!");
	devText = HITextViewGetTXNObject(devTextView);
	if(!devText) {
		msg->error("Can't get devText object from status window!!");
	}

	/* block user input in both develes and msgText boxes */
	
	TXNSetData(msgText,kTXNTextData,CREDITS,strlen(CREDITS),kTXNEndOffset,kTXNEndOffset);
	TXNSetData(devText,kTXNTextData,DEVELOPERS,strlen(DEVELOPERS),kTXNEndOffset,kTXNEndOffset);

	err = InstallEventHandler(GetWindowEventTarget(window),AboutEventHandler,1,windowEvents,this,NULL);
	if(err != noErr)
		msg->error("Can't install event handler for AboutWindow (%d)!!",err);
	
}

AboutWindow::~AboutWindow()
{
	DisposeWindow(window);
}

void AboutWindow::show()
{
	RepositionWindow(window,parent,kWindowCenterOnMainScreen);
	ShowWindow(window);
	BringToFront(window);
}


void AboutWindow::hide() {
	ActivateWindow(window,false);
	HideWindow(window);
	ActivateWindow(parent,true);
}

/* END OF AboutWindow */


/************************************************************
 * EVENT HANDLER 
 ************************************************************/
 
 /* when the user closes the AboutWindow, we will catch the event and
  * prevent window destruction so we can reuse it later if needed ...
  * CarbonGui will request window destruction whene application is quitting
  */ 
 static OSStatus AboutEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    AboutWindow *me = (AboutWindow *)userData;
	switch (GetEventKind (event))
    {
        case kEventWindowClose: 
			me->hide();
			return noErr;
            break;
		default:
            break;
    }
    return CallNextEventHandler(nextHandler,event);
}

