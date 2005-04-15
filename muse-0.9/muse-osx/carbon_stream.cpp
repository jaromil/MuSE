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

#include "carbon_stream.h"

#define STREAM_EVENTS 1
const EventTypeSpec windowEvents[] = {
	{ kEventClassWindow, kEventWindowClose }
};

static OSStatus StreamEventHandler (
	EventHandlerCallRef nextHandler, EventRef event, void *userData);


CarbonStream::CarbonStream(Stream_mixer *mix,WindowRef mainWin,IBNibRef nib) {
		parent=mainWin;
		jmix=mix;
		nibRef=nib;
		OSStatus err;
		msg = new CarbonMessage(nibRef);
		err = CreateWindowFromNib(nibRef,CFSTR("StreamWindow"),&window);
		if(err != noErr) { 
			msg->error("Can't create the stream configuration window (%d)!!",err);
		}
		err = InstallEventHandler(GetWindowEventTarget(window),StreamEventHandler,STREAM_EVENTS,windowEvents,this,NULL);
		if(err != noErr) { 
			msg->error("Can't install event handler for Channel control (%d)!!",err);
		}
}

CarbonStream::~CarbonStream() {
}

void CarbonStream::show() {
	RepositionWindow(window,parent,kWindowCenterOnMainScreen);
	ShowWindow(window);
	BringToFront(window);
	ActivateWindow(window,true);
}

void CarbonStream::hide() {
	ActivateWindow(window,false);
	HideWindow(window);
	ActivateWindow(parent,true);
}

/****************************************************************************/
/* EVENT HANDLERS */
/****************************************************************************/

static OSStatus StreamEventHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus err = noErr;
    CarbonStream *me = (CarbonStream *)userData;
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
