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
 
#include "carbon_channel.h"

OSStatus close (EventHandlerCallRef nextHandler,EventRef inEvent, void *userData);

CARBON_CHANNEL::CARBON_CHANNEL(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib,unsigned int chan) {
	parent = mainWin;
	jmix = mix;
	nibRef = nib;
	chIndex = chan;
	OSStatus err;
	evtClose.eventClass = kEventClassWindow;
	evtClose.eventKind = kEventWindowClose;
	
	err = CreateWindowFromNib(nibRef, CFSTR("Channel"), &window);
	
	err = InstallEventHandler(GetWindowEventTarget(window),close,1,&evtClose,this,NULL);
	if(err != noErr) { 
		/* TODO - Warning messages */
	}
	
	CFStringRef format = CFStringCreateWithCString(NULL,"Channel %d",0);
	CFStringRef wName = CFStringCreateWithFormat(NULL,NULL,format,chan);
	SetWindowTitleWithCFString (window,wName);
	ShowWindow(window);
	BringToFront(window);
}

CARBON_CHANNEL::~CARBON_CHANNEL() {
}

OSStatus close (EventHandlerCallRef nextHandler,EventRef inEvent, void *userData) {
	CARBON_CHANNEL *me = (CARBON_CHANNEL *)userData;
	return CallNextEventHandler (nextHandler, inEvent);
}