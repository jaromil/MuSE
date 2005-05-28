/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2005 xant <xant@dyne.org>
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

#include "carbon_message.h"

static OSStatus MessageCommandHandler (EventHandlerCallRef nextHandler, EventRef event, void *userData);

CarbonMessage::CarbonMessage(IBNibRef nib) {
	nibRef = nib;
}

CarbonMessage::~CarbonMessage() {
}


void CarbonMessage::run(CFStringRef windowName,SInt32 textId,const char *msg) {
	OSStatus err;
	WindowRef window;
	const ControlID tc = { 'MuSE',textId };
	err = CreateWindowFromNib(nibRef, windowName, &window);
	err=CreateWindowGroup(kWindowGroupAttrMoveTogether|kWindowGroupAttrLayerTogether|
			kWindowGroupAttrSharedActivation|kWindowGroupAttrHideOnCollapse,&msgGroup);
		err=SetWindowGroup(window,msgGroup);
	
	err = GetControlByID(window,&tc,&textControl);
	if(err != noErr) {
		/* TODO - Error messages */
	}
	if(msg) {
		setText(msg);
	}
		
	const EventTypeSpec commands[] = {
        { kEventClassCommand, kEventCommandProcess }
    };

  //  require_noerr (err, CantInstallHandler); 
    err = InstallWindowEventHandler (window, 
            NewEventHandlerUPP (MessageCommandHandler), 
            GetEventTypeCount(commands), commands, 
            window, NULL);

	ShowWindow(window);
	BringToFront(window);
	ActivateWindow(window,true);
}

void CarbonMessage::notify(const char *format, ... ) {
	va_list arg;
	va_start(arg, format);
	char msg[255];
	vsnprintf(msg, 254, format, arg);
	run(CFSTR("NotifyWindow"),CM_NOTIFY_ID,msg);
	va_end(arg);
}
void CarbonMessage::warning(const char *format, ... ) {
	va_list arg;
	va_start(arg, format);
	char msg[255];
	vsnprintf(msg, 254, format, arg);
	run(CFSTR("WarnWindow"),CM_WARNING_ID,msg);
	va_end(arg);
}
void CarbonMessage::error(const char *format, ... ) {
	va_list arg;
	va_start(arg, format);
	char msg[255];
	vsnprintf(msg, 254, format, arg);
	run(CFSTR("ErrorWindow"),CM_ERROR_ID,msg);
	va_end(arg);
}

void CarbonMessage::setText(const char *msg ) {
	OSStatus err;
	if(msg) {
		SetControlData(textControl, kControlEntireControl,
		kControlEditTextTextTag, strlen(msg), msg);
	}
}

static OSStatus MessageCommandHandler (
    EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    HICommand command; 
    OSStatus err = noErr;
	SInt16 val;
    WindowRef window = (WindowRef)userData;
	err = GetEventParameter (event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), NULL, &command);
    if(err != noErr) {
		/* TODO - Error messages */
	}
    switch (command.commandID)
    {
        case 'clwa':
		case 'clno':
			DisposeWindow(window);
			//delete me;
			break;
		case 'cler':
			DisposeWindow(window);
			//delete me;
			QuitApplicationEventLoop();
			break;
		default:
            err = eventNotHandledErr;
            break;
    }
}
