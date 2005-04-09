/*
 *  carbon_message.cpp
 *  muse-osx
 *
 *  Created by xant on 07/04/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
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
	run(CFSTR("NotifyrWindow"),CM_NOTIFY_ID,msg);
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